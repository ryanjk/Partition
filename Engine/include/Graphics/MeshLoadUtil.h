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

pn::vector<Mesh>	LoadMesh(const std::string& filename, const MeshLoadData& mesh_load_data);
pn::vector<Mesh>	LoadMesh(const std::string& filename);

} // namespace pn