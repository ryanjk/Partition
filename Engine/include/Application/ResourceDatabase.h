#pragma once

#include <Utilities\UtilityTypes.h>

#include <Graphics\DirectX.h>

namespace pn::rdb {

// -------- TYPEDEFS ----------

using resource_key		= string;

template<typename V>
using resource_map_t	= map<resource_key, V>;

// ------- VARIABLES ------------

resource_map_t<shader_program_t*>	shaders;
resource_map_t<mesh_buffer_t*>		meshes;

// -------- FUNCTIONS ------------

template<typename V>
void	AddResource(resource_map_t<V>& resource_map, const resource_key& key, const V& value) {
	Insert(resource_map, key, value);
}

template<typename V>
void	RemoveResource(resource_map_t<V>& resource_map, const resource_key& key) {
	Remove(resource_map, key);
}

template<typename V>
auto	GetResource(const resource_map_t<V>& resource_map, const resource_key& key) {
	return Get(resource_map, key);
}

} // namespace pn::rdb