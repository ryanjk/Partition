#pragma once

#include <Utilities\Math.h>

namespace pn {

struct transform_t {
	pn::vec3f		position;
	pn::vec3f		rotation;
	pn::vec3f		scale;
	transform_t*	parent = nullptr;
};

pn::mat4f LocalToWorldSRT(const transform_t& transform);
pn::mat4f TransformToSRT(const transform_t& transform);

} // namespace pn

