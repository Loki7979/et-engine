#include <et/models/objloader.h>
#include <et/camera/camera.h>
#include <et/rendering/rendercontext.h>
#include <et/json/json.h>
#include <et/core/conversion.h>
#include "maincontroller.hpp"

using namespace et;
using namespace demo;

void MainController::setApplicationParameters(et::ApplicationParameters& p)
{
	p.windowStyle |= WindowStyle_Sizable;
	p.windowSize = WindowSize::Predefined;
}

void MainController::setRenderContextParameters(et::RenderContextParameters& p)
{
	p.multisamplingQuality = MultisamplingQuality_None;
	p.contextBaseSize = vec2i(1024, 640);
	p.contextSize = p.contextBaseSize;
    p.enableHighResolutionContext = true;
}

void MainController::applicationDidLoad(et::RenderContext* rc)
{
	_rc = rc;
	srand(static_cast<unsigned int>(time(nullptr)));
	
	ObjectsCache localCache;
	
#if (ET_PLATFORM_WIN)
	application().pushSearchPath("Q:\\SDK\\Models\\");
#endif
	
	auto configName = application().resolveFileName("config/config.json");
	
	ValueClass vc = ValueClass_Invalid;
	_options = json::deserialize(loadTextFile(configName), vc);
	ET_ASSERT(vc == ValueClass_Dictionary);
	
	auto modelName = application().resolveFileName("media/" + _options.stringForKey("model-name")->content);
	
	_scene = s3d::Scene::Pointer::create();
	OBJLoader loader(modelName, OBJLoader::Option_CalculateTangents);
	auto model = loader.load(rc, _scene->storage(), localCache);
	model->setParent(_scene.ptr());
	
	Raytrace::Options rtOptions;
	rtOptions.maxRecursionDepth = _options.integerForKey("max-recursion-depth", 8)->content;
	rtOptions.raysPerPixel = _options.integerForKey("rays-per-pixel", 32)->content;
	rtOptions.maxKDTreeDepth = _options.integerForKey("kd-tree-max-depth", 4)->content;
	rtOptions.renderRegionSize = _options.integerForKey("render-region-size", 32)->content;
	rtOptions.debugRendering = _options.integerForKey("debug-rendering", 0ll)->content != 0;
	rtOptions.renderKDTree = _options.integerForKey("render-kd-tree", 0ll)->content != 0;
	rtOptions.kdTreeSplits = static_cast<int>(_options.integerForKey("kd-tree-splits", 4)->content);
	_rt.setOptions(rtOptions);
	
	_rt.setOutputMethod([this](const vec2i& pixel, const vec4& color)
	{
		if ((pixel.x >= 0) && (pixel.y >= 0) && (pixel.x < _texture->size().x) &&  (pixel.y < _texture->size().y))
		{
			DataStorage<vec4> vec4data(reinterpret_cast<vec4*>(_textureData.binary()), _textureData.dataSize());
			int pos = pixel.x + pixel.y * _texture->size().x;
			vec4data[pos] = mix(vec4data[pos], color, color.w);
		}
	});
	
	Input::instance().keyPressed.connect([this](size_t key)
	{
		if (key == ET_KEY_SPACE)
			start();
	});
	
	_gestures.click.connect([this](const PointerInputInfo& p)
	{
		vec2i pixel(int(p.pos.x), int(_texture->size().y - p.pos.y));
		_rt.output(pixel, _rt.performAtPoint(_scene, _camera, _texture->size(), pixel));
	});
	
	Input::instance().keyPressed.invokeInMainRunLoop(ET_KEY_SPACE);
}

void MainController::start()
{
	_rt.stop();
	
	vec2i textureSize = _rc->sizei();
	
	_textureData.resize(textureSize.square() * sizeof(vec4));
	_textureData.fill(0);
	
	_texture = _rc->textureFactory().genTexture(TextureTarget::Texture_2D, TextureFormat::RGBA32F, textureSize,
		TextureFormat::RGBA, DataType::Float, _textureData, "output-texture");
	
	const vec3 lookPoint = arrayToVec3(_options.arrayForKey("camera-view-point"));
	const vec3 offset = arrayToVec3(_options.arrayForKey("camera-offset"));
	float cameraFOV = _options.floatForKey("camera-fov", 60.0f)->content * TO_RADIANS;
	float cameraPhi = _options.floatForKey("camera-phi", 0.0f)->content * TO_RADIANS;
	float cameraTheta = _options.floatForKey("camera-theta", 0.0f)->content * TO_RADIANS;
	float cameraDistance = _options.floatForKey("camera-distance", 3.0f)->content;
	_camera.perspectiveProjection(cameraFOV, vector2ToFloat(textureSize).aspect(), 0.1f, 2048.0f);
	_camera.lookAt(cameraDistance * fromSpherical(cameraTheta, cameraPhi) + offset, lookPoint);
	
	_rt.perform(_scene, _camera, _texture->size());
}

void MainController::applicationWillTerminate()
{
	_rt.stop();
}

void MainController::render(et::RenderContext* rc)
{
	rc->renderer()->clear(true, true);
	
	if (_texture.valid())
	{
		_texture->updateDataDirectly(rc, _texture->size(),
			_textureData.binary(), _textureData.dataSize());
		rc->renderer()->renderFullscreenTexture(_texture);
	}
}

et::IApplicationDelegate* et::Application::initApplicationDelegate()
	{ return sharedObjectFactory().createObject<MainController>(); }

et::ApplicationIdentifier MainController::applicationIdentifier() const
	{ return et::ApplicationIdentifier(applicationIdentifierForCurrentProject(), "Cheetek", "RT demo"); }
