#include <Application\ResourceDatabase.h>

namespace pn::rdb {

// ------- VARIABLES ------------

resource_id_t						asset_count = 0;

resource_map_t<shader_program_t>	shaders{};
resource_map_t<mesh_buffer_t>		meshes{};

// -------- FUNCTIONS ------------

resource_id_t GetNextAssetId() {
	return asset_count++;
}

} // namespace pn::rdb