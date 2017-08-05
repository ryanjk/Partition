#include <Graphics\MeshLoadUtil.h>

#include <Component\transform_t.h>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <Application\ResourceDatabase.h>

#include <utility>

namespace pn {

// ----- VARIABLES ---------

dx_device device;

// ------ FUNCTIONS -----------

void			InitMeshLoadUtil(dx_device d) {
	device = d;
}

unsigned int	MeshLoadDataToAssimp(const MeshLoadData& mesh_load_data) {
	unsigned int assimp_post_process = 0;
	if (mesh_load_data.convert_left) assimp_post_process |= (aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);
	if (mesh_load_data.triangulate) assimp_post_process |= aiProcess_Triangulate;

	return assimp_post_process;
}

pn::mat4f AiMatrixToMat4f(const aiMatrix4x4& m) {
	return mat4f(
		m.a1, m.a2, m.a3, m.a4,
		m.b1, m.b2, m.b3, m.b4,
		m.c1, m.c2, m.c3, m.c4,
		m.d1, m.d2, m.d3, m.d4
	);
}

vec3f aiVector3DToVec3f(const aiVector3D& v) {
	return vec3f(v.x, v.y, v.z);
}

quaternion aiQuaternionToQuaternion(const aiQuaternion& q) {
	return quaternion(q.x, q.y, q.z, q.w);
}

pn::transform_t aiDataToTransform(const aiVector3D& scale, const aiQuaternion& quaternion, const aiVector3D& translation) {
	transform_t transform;
	transform.position	= aiVector3DToVec3f(translation);
	transform.rotation	= aiQuaternionToQuaternion(quaternion);
	transform.scale		= aiVector3DToVec3f(scale);
	return transform;
}

pn::transform_t aiMatrixToTransform(const aiMatrix4x4& aiMatrix) {
	aiVector3D scale, translation;
	aiQuaternion rotation;
	aiMatrix.Decompose(scale, rotation, translation);
	return aiDataToTransform(scale, rotation, translation);
}

pn::mesh_t ConvertAIMeshToMesh(aiMesh* mesh, const aiScene* scene) {
	LogDebug("Loading mesh {}", mesh->mName.C_Str());
	
	pn::mesh_t result_mesh;

	const unsigned int VERTEX_COUNT = mesh->mNumVertices;
	Resize(result_mesh.vertices, VERTEX_COUNT);
	std::memcpy(&(result_mesh.vertices[0]), mesh->mVertices, VERTEX_COUNT * sizeof(pn::vec3f));

	if (mesh->HasNormals()) {
		Resize(result_mesh.normals, VERTEX_COUNT);
		std::memcpy(&result_mesh.normals[0], mesh->mNormals, VERTEX_COUNT * sizeof(pn::vec3f));
	}

	if (mesh->HasTangentsAndBitangents()) {
		Resize(result_mesh.tangents, VERTEX_COUNT);
		Resize(result_mesh.bitangents, VERTEX_COUNT);
		std::memcpy(&result_mesh.tangents[0], mesh->mTangents, VERTEX_COUNT * sizeof(pn::vec3f));
		std::memcpy(&result_mesh.bitangents[0], mesh->mBitangents, VERTEX_COUNT * sizeof(pn::vec3f));
	}

	if (mesh->GetNumColorChannels() >= 1) {
		Resize(result_mesh.colors, VERTEX_COUNT);
		std::memcpy(&result_mesh.colors[0], mesh->mColors[0], VERTEX_COUNT * sizeof(pn::vec4f));
	}

	if (mesh->GetNumUVChannels() >= 1) {
		Resize(result_mesh.uvs, VERTEX_COUNT);
		if (mesh->GetNumUVChannels() >= 2) {
			Resize(result_mesh.uv2s, VERTEX_COUNT);
		}

		for (unsigned int i = 0; i < VERTEX_COUNT; ++i) {
			std::memcpy(&result_mesh.uvs[i], &(mesh->mTextureCoords[0][i]), sizeof(pn::vec2f));

			if (mesh->GetNumUVChannels() >= 2) {
				std::memcpy(&result_mesh.uv2s[i], &(mesh->mTextureCoords[1][i]), sizeof(pn::vec2f));
			}
		}
	}

	Reserve(result_mesh.indices, VERTEX_COUNT / 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			pn::PushBack(result_mesh.indices, face.mIndices[j]);
		}
	}
	result_mesh.topology = D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	LogDebug("Finished loading mesh {}", mesh->mName.C_Str());
	return std::move(result_mesh);
}

void ProcessAINode(aiNode* node, const aiScene* scene, pn::vector<mesh_t>& meshes) {
	auto transform = aiMatrixToTransform(node->mTransformation);

	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		auto* ai_mesh		= scene->mMeshes[node->mMeshes[i]];
		auto mesh			= std::move(ConvertAIMeshToMesh(ai_mesh, scene));
		auto mesh_buffer	= CreateMeshBuffer(device, mesh);
		auto mesh_id		= rdb::AddResource(rdb::meshes, std::move(mesh_buffer));

		// need to set up connection to parent and children somehow

		EmplaceBack(meshes, std::move(mesh));
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		ProcessAINode(node->mChildren[i], scene, meshes);
	}
}

auto ConvertAISceneToMeshes(const aiScene* ai_scene, pn::vector<mesh_t>& meshes) {
	ProcessAINode(ai_scene->mRootNode, ai_scene, meshes);
}

pn::vector<mesh_t> LoadMesh(const std::string& filename, const MeshLoadData& mesh_load_data) {
	Assimp::Importer importer;
	auto file_data = pn::ReadResource(filename);
	if (file_data.empty()) {
		LogError("MeshLoad: Couldn't load file {}", filename);
		return {};
	}
	const auto* ai_scene = importer.ReadFileFromMemory(
		file_data.data(), file_data.size(),
		MeshLoadDataToAssimp(mesh_load_data),
		nullptr
	);
	if (!ai_scene) {
		LogError("Assimp: Couldn't read file: {}", importer.GetErrorString());
		return {};
	}
	pn::vector<mesh_t> meshes;
	ConvertAISceneToMeshes(ai_scene, meshes);
	return meshes;
}

pn::vector<mesh_t> LoadMesh(const std::string& filename) {
	MeshLoadData default_load_data;
	default_load_data.convert_left = true;
	default_load_data.triangulate = true;
	return LoadMesh(filename, default_load_data);
}

} //namespace pn