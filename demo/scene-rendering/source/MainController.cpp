#include <et/input/input.h>
#include <et/rendering/rendercontext.h>
#include "MainController.h"

using namespace et;
using namespace demo;

void MainController::setApplicationParameters(et::ApplicationParameters&)
{
	
}

void MainController::setRenderContextParameters(et::RenderContextParameters& params)
{
	params.contextSize = vec2i(1024, 768);
	params.contextBaseSize = params.contextSize;
	params.multisamplingQuality = MultisamplingQuality_Best;
}

void MainController::applicationDidLoad(et::RenderContext* rc)
{
#if (ET_PLATFORM_WIN)
	application().pushRelativeSearchPath("..");
	application().pushRelativeSearchPath("..\\..");
	application().pushRelativeSearchPath("..\\..\\..");
	application().pushSearchPath("Q:\\SDK\\Models\\");
	application().pushSearchPath("Q:\\SDK\\");
#elif (ET_PLATFORM_MAC)
	application().pushSearchPath("/Volumes/Development/SDK/");
	application().pushSearchPath("/Volumes/Development/SDK/Models/");
#endif

	rc->renderState().setClearColor(vec4(0.25f, 0.0f));
	
	rc->renderingInfoUpdated.connect([this](const et::RenderingInfo& info)
	{
		log::info("Rendering stats: %lu fps, %lu polys, %lu draw calls", info.averageFramePerSecond,
			info.averagePolygonsPerSecond, info.averageDIPPerSecond);
	});
	
	_gestures.drag.connect([this](et::vec2 d, size_t)
	{
		_cameraController.handlePointerDrag(d);
	});
	
	_loader.init(rc);
	_renderer.init(rc);
	_cameraController.init(rc);
	
	auto loadedScene = _loader.loadFromFile(application().resolveFileName("bunny.obj"));
	_renderer.setScene(loadedScene);

	connectInputEvents();
}

void MainController::connectInputEvents()
{
	input().keyPressed.connect([this](size_t key)
	{
		_cameraController.handlePressedKey(key);
		_renderer.handlePressedKey(key);
	});
	
	input().keyReleased.connect([this](size_t key)
	{
		_cameraController.handleReleasedKey(key);
	});
}

 void MainController::applicationWillResizeContext(const et::vec2i& sz)
{
	_cameraController.adjustCameraToNextContextSize(vector2ToFloat(sz));
}

void MainController::render(et::RenderContext* rc)
{
	_renderer.render(_cameraController.camera(), _cameraController.observerCamera(), false);
}

et::IApplicationDelegate* et::Application::initApplicationDelegate()
	{ return sharedObjectFactory().createObject<MainController>(); }

et::ApplicationIdentifier MainController::applicationIdentifier() const
	{ return ApplicationIdentifier("com.cheetek.scenerendering", "Cheetek", "Scene Rendering Demo"); }
