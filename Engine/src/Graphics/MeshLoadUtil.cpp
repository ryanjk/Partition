#include <Graphics\MeshLoadUtil.h>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

namespace pn {

unsigned int MeshLoadDataToAssimp(const MeshLoadData& mesh_load_data) {
	unsigned int assimp_post_process;

	if (mesh_load_data.convert_left) assimp_post_process |= (aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);
	if (mesh_load_data.triangulate) assimp_post_process |= aiProcess_Triangulate;

	return assimp_post_process;
}

Mesh ConvertAIMeshToMesh(aiMesh* mesh, const aiScene* scene) {
	LogDebug("Loading mesh {}", mesh->mName.C_Str());
	Mesh result_mesh;

	const unsigned int VERTEX_COUNT = mesh->mNumVertices;
	result_mesh.positions.resize(VERTEX_COUNT);
	std::memcpy(&(result_mesh.positions[0]), mesh->mVertices, VERTEX_COUNT * sizeof(pn::vec3f));

	if (mesh->HasNormals()) {
		result_mesh.normals.resize(VERTEX_COUNT);
		std::memcpy(&result_mesh.normals[0], mesh->mNormals, VERTEX_COUNT * sizeof(pn::vec3f));
	}

	if (mesh->HasTangentsAndBitangents()) {
		result_mesh.tangents.resize(VERTEX_COUNT);
		result_mesh.bitangents.resize(VERTEX_COUNT);
		std::memcpy(&result_mesh.tangents[0], mesh->mTangents, VERTEX_COUNT * sizeof(pn::vec3f));
		std::memcpy(&result_mesh.bitangents[0], mesh->mBitangents, VERTEX_COUNT * sizeof(pn::vec3f));
	}

	if (mesh->GetNumColorChannels() >= 1) {
		result_mesh.colors.resize(VERTEX_COUNT);
		std::memcpy(&result_mesh.colors[0], mesh->mColors[0], VERTEX_COUNT * sizeof(pn::vec4f));
	}

	if (mesh->GetNumUVChannels() >= 1) {
		result_mesh.uvs.resize(VERTEX_COUNT);
		if (mesh->GetNumUVChannels() >= 2) {
			result_mesh.uv2s.resize(VERTEX_COUNT);
		}
	}

	for (unsigned int i = 0; i > VERTEX_COUNT; ++i) {
		std::memcpy(&result_mesh.uvs[i], &(mesh->mTextureCoords[0][i]), sizeof(pn::vec2f));

		if (mesh->GetNumUVChannels() >= 2) {
			std::memcpy(&result_mesh.uv2s[i], &(mesh->mTextureCoords[1][i]), sizeof(pn::vec2f));
		}
	}

	result_mesh.indices.reserve(VERTEX_COUNT / 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			result_mesh.indices.push_back(face.mIndices[j]);
		}
	}
	result_mesh.topology = D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	LogDebug("Finished loading mesh {}", mesh->mName.C_Str());
	return result_mesh;
}

void ProcessAINode(aiNode* node, const aiScene* scene, pn::vector<Mesh>& meshes) {
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		auto* ai_mesh = scene->mMeshes[node->mMeshes[i]];
		auto mesh = ConvertAIMeshToMesh(ai_mesh, scene);
		meshes.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		ProcessAINode(node->mChildren[i], scene, meshes);
	}
}

auto ConvertAISceneToMeshes(const aiScene* ai_scene, pn::vector<Mesh>& meshes) {
	ProcessAINode(ai_scene->mRootNode, ai_scene, meshes);
}

pn::vector<Mesh> LoadMesh(const std::string& filename, const MeshLoadData& mesh_load_data) {
	Assimp::Importer importer;
	auto file_data = pn::ReadFile(filename);
	if (file_data.empty()) {
		LogError("MeshLoad: Couldn't load file {}", filename);
		return {};
	}
	const auto* ai_scene = importer.ReadFileFromMemory(
		file_data.data(), file_data.size(),
		MeshLoadDataToAssimp(mesh_load_data),
		nullptr
	);
	pn::vector<Mesh> meshes;
	ConvertAISceneToMeshes(ai_scene, meshes);
	return meshes;
}

pn::vector<Mesh> LoadMesh(const std::string& filename) {
	MeshLoadData default_load_data;
	default_load_data.convert_left = true;
	default_load_data.triangulate = true;
	return LoadMesh(filename, default_load_data);
}

} //namespace pn