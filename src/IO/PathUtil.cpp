#include <IO\PathUtil.h>

namespace pn {

// --------- VARIABLES -----------------

static string working_dir;
static string resource_dir;

static string texture_dir = "texture\\";
static string mesh_dir    = "mesh\\";
static string cubemap_dir = "cubemap\\";
static string shader_dir  = "shader\\";

// --------- FUNCTIONS -----------------

void InitPathUtil() {
	working_dir		= "";
	resource_dir	= "";
}

string GetWorkingDirectory() {
	return working_dir;
}

void SetWorkingDirectory(const string& directory) {
	working_dir = directory + "/";
}

string GetResourceDirectory() {
	return resource_dir;
}

void SetResourceDirectoryName(const string& name) {
	resource_dir = working_dir + name + "/";
}

string ResourceTypeToDirectory(const string& resource_name) {
	const auto split_file = Split(resource_name, '.');
	if (Size(split_file) != 2) {
		LogError("Couldn't parse file extension of {}", resource_name);
		return "";
	}

	const string file_extension = split_file[1];
	if (file_extension == "jpg" || file_extension == "png") {
		return texture_dir;
	}
	if (file_extension == "fbx") {
		return mesh_dir;
	}
	if (file_extension == "hlsl" || file_extension == "hlsli") {
		return shader_dir;
	}
	if (file_extension == "dds") {
		return cubemap_dir;
	}

	LogError("Unsupported file extension: {}", resource_name);
	return "";
}

string GetResourcePath(const string& resource_name) {
	string type_dir = ResourceTypeToDirectory(resource_name);
	return resource_dir + type_dir + resource_name;
}

} // namespace pn