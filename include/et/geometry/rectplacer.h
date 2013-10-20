/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#pragma once

#include <et/core/et.h>
#include <et/geometry/geometry.h>

namespace et
{
	class RectPlacer
	{
	public:
		typedef std::vector<rect> RectList;

	public:
		RectPlacer(const vec2i& contextSize, bool addSpace);
		bool place(const vec2i& size, rect& placedPosition);

		const RectList& placedItems() const 
			{ return _placedItems; }

	private:
		vec2i _contextSize;
		RectList _placedItems;
		bool _addSpace;
	};
}