#include <Component\transform_t.h>

#include <UI\UIUtil.h>

namespace pn {

pn::mat4f LocalToWorldSRT(const transform_t& transform) {
	auto cur_srt = TransformToSRT(transform);
	auto* cur_parent = transform.parent;
	while (cur_parent != nullptr) {
		cur_srt *= TransformToSRT(*cur_parent);
		cur_parent = cur_parent->parent;
	}
	return cur_srt;
}

pn::mat4f TransformToSRT(const transform_t& transform) {
	return SRTMatrix(transform.scale, transform.rotation, transform.position);
}

namespace gui {

template<>
void EditStruct(pn::transform_t& transform) {
	DragFloat3("position", &transform.position.x, -INFINITY, INFINITY);
	DragFloat3("rotation", &transform.rotation.x, -INFINITY, INFINITY, 0.1f);
	DragFloat3("scale", &transform.scale.x, -INFINITY, INFINITY);
}

} // namespace gui

} // namespace pn
