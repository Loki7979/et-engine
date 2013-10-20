/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#pragma once

#include <et/app/events.h>
#include <et/core/objectscache.h>
#include <et/scene3d/element.h>
#include <et/scene3d/storage.h>
#include <et/scene3d/mesh.h>
#include <et/scene3d/supportmesh.h>
#include <et/scene3d/cameraelement.h>
#include <et/scene3d/lightelement.h>

namespace et
{
	namespace s3d
	{
		class Scene : public ElementContainer, public ElementFactory
		{
		public:
			Scene(const std::string& name = "scene");

			/*
			 * Synchronous serializing
			 */
			void serialize(std::ostream& stream, StorageFormat fmt, const std::string& basePath);
			void serialize(const std::string& filename, StorageFormat fmt);

			/*
			 * Synchronous deserializing
			 */
			bool deserialize(std::istream& stream, RenderContext* rc, ObjectsCache& tc,
				ElementFactory* factory, const std::string& basePath);
			bool deserialize(const std::string& filename, RenderContext* rc, ObjectsCache& tc,
				ElementFactory* factory);

			/*
			 * Asynchronous deserializing
			 */
			void deserializeAsync(std::istream& stream, RenderContext* rc, ObjectsCache& tc,
				ElementFactory* factory, const std::string& basePath);
			void deserializeAsync(const std::string& filename, RenderContext* rc, ObjectsCache& tc,
				ElementFactory* factory);

		public:
			ET_DECLARE_EVENT1(deserializationFinished, size_t)

		private:
			bool performDeserialization(std::istream& stream, RenderContext* rc, ObjectsCache& tc,
				ElementFactory* factory, const std::string& basePath, bool async);

			void buildAPIObjects(Scene3dStorage::Pointer p, RenderContext* rc);

			Scene3dStorage::Pointer deserializeStorage(std::istream& stream, RenderContext* rc,
				ObjectsCache& tc, const std::string& basePath, StorageFormat fmt, bool async);
			
			Element::Pointer createElementOfType(size_t type, Element* parent);
			Material materialWithId(int id);

			VertexBuffer vertexBufferWithId(const std::string& id);
			IndexBuffer indexBufferWithId(const std::string& id);
			VertexArrayObject vaoWithIdentifiers(const std::string& vbid, const std::string& ibid);

		private:
			ElementFactory* _externalFactory;
			VertexBufferList _vertexBuffers;
			IndexBufferList _indexBuffers;
			VertexArrayObjectList _vaos;
		};
	}
}