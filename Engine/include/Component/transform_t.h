#pragma once

#include <Utilities\Math.h>

#include <UI\EditStruct.h>

namespace pn {

struct transform_t {
	pn::vec3f		position	= { 0, 0, 0 };
	pn::quaternion	rotation	= { 0, 0, 0, 1 };
	pn::vec3f		scale		= { 1, 1, 1 };
	transform_t*	parent		= nullptr;
};

pn::mat4f LocalToWorldSRT(const transform_t& transform);
pn::mat4f TransformToSRT(const transform_t& transform);

namespace gui {

template<>
void EditStruct(pn::transform_t& transform);

} // namespace gui

} // namespace pn

