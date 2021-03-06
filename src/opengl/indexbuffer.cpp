/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/opengl/opengl.h>
#include <et/rendering/rendercontext.h>

using namespace et;

IndexBuffer::IndexBuffer(RenderContext* rc, IndexArray::Pointer i, BufferDrawType drawType,
	const std::string& aName) : APIObject(aName), _rc(rc), _size(i->actualSize()), _sourceObjectName(i->name()),
	_drawType(drawType)
{
	build(i);
}

IndexBuffer::~IndexBuffer()
{
	uint32_t buffer = static_cast<uint32_t>(apiHandle());
	if (buffer != 0)
	{
		_rc->renderState().indexBufferDeleted(buffer);
		glDeleteBuffers(1, &buffer);
	}
}

void IndexBuffer::setProperties(const IndexArray::Pointer& i)
{
	_size = i->actualSize();
	_primitiveType = i->primitiveType();
	_format = i->format();

	switch (_format)
	{
		case IndexArrayFormat::Format_8bit:
		{
			_dataType = DataType::UnsignedChar;
			break;
		}
			
		case IndexArrayFormat::Format_16bit:
		{
			_dataType = DataType::UnsignedShort;
			break;
		}
			
		case IndexArrayFormat::Format_32bit:
		{
			_dataType = DataType::UnsignedInt;
			break;
		}
			
		default:
			ET_FAIL_FMT("Invalid IndexArrayFormat value: %u", static_cast<uint32_t>(_format));
	}
}

void IndexBuffer::build(const IndexArray::Pointer& i)
{
	ET_ASSERT(i.valid());

	if (apiHandleInvalid())
	{
		uint32_t buffer = 0;
		
		glGenBuffers(1, &buffer);
		checkOpenGLError("glGenBuffers(1, &_indexBuffer)");
		
		setAPIHandle(buffer);
	}

	setProperties(i);
	internal_setData(i->data(), static_cast<size_t>(i->format()) * _size);
}

void IndexBuffer::internal_setData(const unsigned char* data, size_t size)
{
	if (size > 0)
	{
		_rc->renderState().bindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<uint32_t>(apiHandle()));
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, drawTypeValue(_drawType));
		checkOpenGLError("glBufferData(GL_ELEMENT_ARRAY_BUFFER, %u, 0x%08X, ..,)", size, data);
	}
}

void* IndexBuffer::indexOffset(size_t offset) const
{
	return reinterpret_cast<void*>(static_cast<size_t>(_format) * offset);
}

void IndexBuffer::setData(const IndexArray::Pointer& i)
{
	build(i);
}

void IndexBuffer::clear()
{
	_size = 0;
	internal_setData(nullptr, 0);
}

void IndexBuffer::overridePrimitiveType(PrimitiveType pt)
{
	_primitiveType = pt;
}
