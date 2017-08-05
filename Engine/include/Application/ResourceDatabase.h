#pragma once

#include <Utilities\UtilityTypes.h>

#include <Graphics\DirectX.h>

namespace pn::rdb {

// -------- TYPEDEFS ----------

using resource_id_t		= unsigned long long;

template<typename V>
using resource_map_t	= map<resource_id_t, V>;

// ------- VARIABLES ------------

extern resource_map_t<shader_program_t>		shaders;
extern resource_map_t<mesh_buffer_t>		meshes;

// -------- FUNCTIONS ------------

resource_id_t	GetNextAssetId();

template<typename V>
resource_id_t	AddResource(resource_map_t<V>& resource_map, V&& value) {
	auto key = GetNextAssetId();
	pn::Insert(resource_map, std::forward<resource_id_t>(key), std::forward<V>(value));
	return key;
}

template<typename V>
void	RemoveResource(resource_map_t<V>& resource_map, const resource_id_t& key) {
	pn::Remove(resource_map, key);
}

template<typename V>
auto	GetResource(const resource_map_t<V>& resource_map, const resource_id_t& key) {
	return pn::Get(resource_map, key);
}

} // namespace pn::rdb