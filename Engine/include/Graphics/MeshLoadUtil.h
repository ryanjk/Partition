#pragma once

#include <string>
#include <vector>

#include <Graphics\DirectX.h>
#include <Utilities\UtilityTypes.h>

namespace pn {

// -------- CLASS DEFINITIONS ------------

struct MeshLoadData {
	bool triangulate;
	bool convert_left;
};

// ---------- FUNCTIONS --------------------

void InitMeshLoadUtil(dx_device device);

pn::vector<mesh_t>	LoadMesh(const std::string& filename, const MeshLoadData& mesh_load_data);
pn::vector<mesh_t>	LoadMesh(const std::string& filename);

} // namespace pn