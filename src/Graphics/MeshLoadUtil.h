#pragma once

#include <string>
#include <vector>

#include <Application\ResourceDatabaseTypes.h>
#include <Utilities\UtilityTypes.h>
#include <Utilities\Math.h>

namespace pn {

// -------- CLASS DEFINITIONS ------------

struct mesh_t {
	pn::vector<pn::vec3f>		vertices;
	pn::vector<pn::vec4f>		colors;
	pn::vector<pn::vec3f>		normals;
	pn::vector<pn::vec3f>		tangents;
	pn::vector<pn::vec3f>		bitangents;
	pn::vector<pn::vec2f>		uvs;
	pn::vector<pn::vec2f>		uv2s;

	pn::vector<unsigned int>	indices;

	pn::string					name;

};

struct MeshLoadData {
	bool triangulate;
	bool convert_left;
};

// ---------- FUNCTIONS --------------------

bool LoadMesh(const pn::string& filename, const MeshLoadData& mesh_load_data, mesh_t& mesh);
bool LoadMesh(const pn::string& filename, mesh_t& mesh);

} // namespace pn