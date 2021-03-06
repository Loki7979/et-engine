/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/core/rawdataaccessor.h>
#include <et/vertexbuffer/vertexarray.h>
#include <et/vertexbuffer/vertexdeclaration.h>

namespace et
{
	template <VertexAttributeType T>
	class VertexDataAccessor { };
	
	class VertexStoragePrivate;
	class VertexStorage : public Object
	{
	public:
		ET_DECLARE_POINTER(VertexStorage)
		
		int tag = 0;
		
	public:
		VertexStorage(const VertexDeclaration&, size_t);
		VertexStorage(const VertexArray::Pointer&);
		~VertexStorage();
		
		template <VertexAttributeType T>
		VertexDataAccessor<T> accessData(VertexAttributeUsage usage, size_t offset)
		{
			ET_ASSERT(hasAttributeWithType(usage, T));
			
			return VertexDataAccessor<T>(data().binary(), data().dataSize(), stride(),
				offset * stride() + offsetOfAttribute(usage));
		}
		
		template <VertexAttributeType T>
		VertexDataAccessor<T> accessData(VertexAttributeUsage usage, size_t offset) const
		{
			ET_ASSERT(hasAttributeWithType(usage, T));
			
			return VertexDataAccessor<T>(data().binary(), data().dataSize(), stride(),
				offset * stride() + offsetOfAttribute(usage));
		}
		
		BinaryDataStorage& data();
		const BinaryDataStorage& data() const;
		
		void increaseSize(size_t);
		void resize(size_t);
		
		bool hasAttribute(VertexAttributeUsage) const;
		bool hasAttributeWithType(VertexAttributeUsage, VertexAttributeType) const;
		VertexAttributeType attributeType(VertexAttributeUsage) const;
		
		const VertexDeclaration& declaration() const;
		
		size_t stride() const;
		size_t capacity() const;
		
		size_t offsetOfAttribute(VertexAttributeUsage usage) const;
		size_t sizeOfAttribute(VertexAttributeUsage usage) const;
	private:
		ET_DECLARE_PIMPL(VertexStorage, 128)
	};
	
#define ET_DECLARE_ACCESSOR(vat, base) \
	template <> class VertexDataAccessor<VertexAttributeType::vat> : public RawDataAcessor<base> \
	{ \
	public: \
		VertexDataAccessor() { } \
		VertexDataAccessor(const char* data, size_t dataSize, size_t stride, size_t offset) : \
			RawDataAcessor<base>(data, dataSize, stride, offset) { } \
		VertexDataAccessor(char* data, size_t dataSize, size_t stride, size_t offset) : \
			RawDataAcessor<base>(data, dataSize, stride, offset) { } \
	};
	
	ET_DECLARE_ACCESSOR(Int, int32_t)
	ET_DECLARE_ACCESSOR(Float, float)
	ET_DECLARE_ACCESSOR(Vec2, vec2)
	ET_DECLARE_ACCESSOR(IntVec2, vec2i)
	ET_DECLARE_ACCESSOR(Vec3, vec3)
	ET_DECLARE_ACCESSOR(IntVec3, vec3i)
	ET_DECLARE_ACCESSOR(Vec4, vec4)
	ET_DECLARE_ACCESSOR(IntVec4, vec4i)
#undef ET_DECLARE_ACCESSOR
}
