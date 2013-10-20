/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <et/core/tools.h>
#include <et/opengl/openglcaps.h>

using namespace et;

void OpenGLCapabilites::checkCaps()
{
	const char* glv = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	_versionShortString = std::string();
	_versionString = std::string(glv ? glv : "<Unknown OpenGL version>");
	if (glv != nullptr)
	{
		do
		{
			if ((_versionShortString.size() > 0) && (*glv == ET_SPACE)) break;
			
			if ((*glv >= '0') && (*glv <= '9'))
				_versionShortString.push_back(*glv);
		}
		while (*glv++);
	}
	while (_versionShortString.size() < 3)
		_versionShortString.push_back('0');
	checkOpenGLError("OpenGLCapabilites::checkCaps");
	
	const char* glslv = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	_glslVersionShortString = std::string();
	_glslVersionString = std::string(glslv ? glslv : "<Unknown GLSL version>");
	if (glslv != nullptr)
	{
		do
		{
			if ((_glslVersionShortString.size() > 0) && (*glslv == ET_SPACE)) break;

			if ((*glslv >= '0') && (*glslv <= '9'))
				_glslVersionShortString.push_back(*glslv);
		}
		while (*glslv++);
	}

	while (_glslVersionShortString.size() < 3)
		_glslVersionShortString.push_back('0');

	std::string lowercaseVersion = _versionString;
	lowercase(lowercaseVersion);
	
	_version = (strToInt(_glslVersionShortString) < 130) || (lowercaseVersion.find("es") != std::string::npos) ?
		OpenGLVersion_Old : OpenGLVersion_New;
	
	int maxSize = 0;
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxSize);
	_maxCubemapTextureSize = static_cast<size_t>(maxSize);
	
	int maxUnits = 0;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
	
	if (maxUnits > 0)
		setFlag(OpenGLFeature_VertexTextureFetch);

#if defined(GL_ARB_draw_elements_base_vertex)
	setFlag(OpenGLFeature_DrawElementsBaseVertex);
#endif
	
	if (glGenerateMipmap != nullptr)
		setFlag(OpenGLFeature_MipMapGeneration);
	
#if (ET_SUPPORT_VERTEX_ARRAY_OBJECTS)
	bool stillSupport = (glGenVertexArrays != nullptr) && (glDeleteVertexArrays != nullptr)
		&& (glBindVertexArray != nullptr) && (glIsVertexArray != nullptr);
	
	if (stillSupport)
	{
		uint32_t testArray = 0;
		glGenVertexArrays(1, &testArray);
		if ((glGetError() == GL_NO_ERROR) && (testArray != 0))
		{
			glDeleteVertexArrays(1, &testArray);
			setFlag(OpenGLFeature_VertexArrayObjects);
		}
	}
#endif
	
	log::info("[OpenGLCapabilites] Version: %s (%s), GLSL version: %s (%s)", _versionString.c_str(),
		_versionShortString.c_str(), _glslVersionString.c_str(), _glslVersionShortString.c_str());
};