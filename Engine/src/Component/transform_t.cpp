#include <Component\transform_t.h>

#include <UI\UIUtil.h>
#include <Utilities\Logging.h>

#include <imgui\imgui_internal.h>

namespace pn {

pn::mat4f LocalToWorldMatrix(const transform_t& transform) {
	auto cur_srt = TransformToMatrix(transform);
	auto* cur_parent = transform.parent;
	while (cur_parent != nullptr) {
		cur_srt *= TransformToMatrix(*cur_parent);
		cur_parent = cur_parent->parent;
	}
	return cur_srt;
}

pn::mat4f WorldToLocalMatrix(const transform_t& transform) {
	return Inverse(LocalToWorldMatrix(transform));
}

pn::vec3f TransformPoint(const transform_t& transform, const pn::vec3f& point) {
	return TransformPoint(transform, vec4f(point, 1.0f)).xyz();
}
pn::vec3f TransformDirection(const transform_t& transform, const pn::vec3f& direction) {
	return TransformDirection(transform, vec4f(direction, 0.0f)).xyz();
}
pn::vec3f TransformVector(const transform_t& transform, const pn::vec3f& vector) {
	return TransformVector(transform, vec4f(vector, 0.0f)).xyz();
}

pn::vec4f TransformPoint(const transform_t& transform, const pn::vec4f& point) {
	assert(point.w == 1.0f);
	return point * LocalToWorldMatrix(transform);
}
pn::vec4f TransformDirection(const transform_t& transform, const pn::vec4f& direction) {
	assert(direction.w == 0.0f);
	return Normalize(direction * LocalToWorldMatrix(transform));
}
pn::vec4f TransformVector(const transform_t& transform, const pn::vec4f& vector) {
	assert(vector.w == 0.0f);
	return vector * LocalToWorldMatrix(transform);
}

pn::vec3f InverseTransformPoint(const transform_t& transform, const pn::vec3f& point) {
	return InverseTransformPoint(transform, vec4f(point, 1.0f)).xyz();
}
pn::vec3f InverseTransformDirection(const transform_t& transform, const pn::vec3f& direction) {
	return InverseTransformDirection(transform, vec4f(direction, 0.0f)).xyz();
}
pn::vec3f InverseTransformVector(const transform_t& transform, const pn::vec3f& vector) {
	return InverseTransformVector(transform, vec4f(vector, 0.0f)).xyz();
}

pn::vec4f InverseTransformPoint(const transform_t& transform, const pn::vec4f& point) {
	assert(point.w == 1.0f);
	return point * WorldToLocalMatrix(transform);
}
pn::vec4f InverseTransformDirection(const transform_t& transform, const pn::vec4f& direction) {
	assert(direction.w == 0.0f);
	return Normalize(direction * WorldToLocalMatrix(transform));
}
pn::vec4f InverseTransformVector(const transform_t& transform, const pn::vec4f& vector) {
	assert(vector.w == 0.0f);
	return vector * WorldToLocalMatrix(transform);
}

pn::mat4f TransformToMatrix(const transform_t& transform) {
	return SRTMatrix(transform.scale, transform.rotation, transform.position);
}

namespace gui {

template<>
void EditStruct(pn::transform_t& transform) {
	DragFloat3("position", &transform.position.x, -INFINITY, INFINITY);
	
	static float ROT_SCALE = 0.01f;

	auto euler = QuaternionToEuler(transform.rotation);
	
	// this is a mess, lots of inlining imgui functions
	using namespace ImGui;
	const char* label = "rotation";
	ImGuiContext& g = *GImGui;
	float return_delta = 0.0f;
	BeginGroup();
	PushID(label);
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	const ImGuiStyle& style = GImGui->Style;
	auto w_full = ImGui::CalcItemWidth();
	int components = 3;
	const float w_item_one = ImMax(1.0f, (float)(int)((w_full - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
	const float w_item_last = ImMax(1.0f, (float)(int)(w_full - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));
	window->DC.ItemWidthStack.push_back(w_item_last);
	for (int i = 0; i < components - 1; i++)
		window->DC.ItemWidthStack.push_back(w_item_one);
	window->DC.ItemWidth = window->DC.ItemWidthStack.back();

	{
		PushID(0);
		float xr = ImGui::DeltaDragFloat("##v", &euler.x);
		if (xr != 0.0f)
			transform.rotation *= AxisAngleToQuaternion(TransformDirection(transform, vec3f::UnitX), xr*ROT_SCALE);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}

	{
		PushID(1);
		float xy = ImGui::DeltaDragFloat("##v", &euler.y);
		if (xy != 0.0f)
			transform.rotation *= AxisAngleToQuaternion(TransformDirection(transform, vec3f::UnitY), xy*ROT_SCALE);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}

	{
		PushID(2);
		float xz = ImGui::DeltaDragFloat("##v", &euler.z);
		if (xz != 0.0f)
			transform.rotation *= AxisAngleToQuaternion(TransformDirection(transform, vec3f::UnitZ), xz*ROT_SCALE);
		SameLine(0, g.Style.ItemInnerSpacing.x);
		PopID();
		PopItemWidth();
	}

	PopID();

	TextUnformatted(label, FindRenderedTextEnd(label));
	EndGroup();
	
	pn::gui::DragFloat3("scale", &transform.scale.x, -INFINITY, INFINITY);
}

} // namespace gui

} // namespace pn
