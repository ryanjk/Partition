#include <Application\ResourceDatabase.h>

namespace pn::rdb {

// ------- VARIABLES ------------

resource_id_t							asset_count = 1; // keep 0 reserved as NULL value

pn::map<resource_id_t, mesh_resource_t>	meshes{};
pn::map<resource_id_t, transform_t>		mesh_transforms{};
pn::map<resource_id_t, mesh_children_t>	mesh_children{};

// -------- FUNCTIONS ------------

resource_id_t GetNextAssetId() {
	return asset_count++;
}

resource_id_t		AddMeshResource(mesh_resource_t& mesh) {
	auto new_id = GetNextAssetId();
	mesh.id		= new_id;
	pn::Insert(meshes, new_id, mesh);
	return new_id;
}
void				RemoveMeshResource(const resource_id_t key) {
	pn::Remove(meshes, key);
}
mesh_resource_t		GetMeshResource(const resource_id_t key) {
	return pn::Get(meshes, key);
}

void		AddMeshTransform(const resource_id_t mesh_id, const mesh_transform_t& transform) {
	pn::Insert(mesh_transforms, mesh_id, transform);
}
void		RemoveMeshTransform(const resource_id_t mesh_id, const mesh_transform_t& transform) {
	pn::Remove(mesh_transforms, mesh_id);
}
mesh_transform_t GetMeshTransform(const resource_id_t mesh_id) {
	return pn::Get(mesh_transforms, mesh_id);
}

void		AddMeshChild(const resource_id_t mesh_id, const resource_id_t child_id) {
	if (!pn::Contains(mesh_children, mesh_id)) {
		pn::Insert(mesh_children, mesh_id, mesh_children_t{});
	}
	auto& children = pn::Get(mesh_children, mesh_id);
	pn::PushBack(children, child_id);
}
void		RemoveMeshChild(const resource_id_t mesh_id, const resource_id_t child_id) {
	mesh_children_t& children = pn::Get(mesh_children, mesh_id);
	pn::Erase(children, child_id);
}
mesh_children_t GetMeshChildren(const resource_id_t mesh_id) {
	return pn::Get(mesh_children, mesh_id);
}

} // namespace pn::rdb