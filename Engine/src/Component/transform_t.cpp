#include <Component\transform_t.h>

namespace pn {

pn::mat4f TransformToSRT(const transform_t& transform) {
	return pn::SRTMatrix(transform.scale, transform.rotation, transform.position);
}

} // namespace pn
