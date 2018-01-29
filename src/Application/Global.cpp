
#include <fstream>

#include <json11\json11.hpp>

#include <Application\Global.h>
#include <IO\PathUtil.h>

namespace pn {

namespace app {

// -------- VARIABLES -----------

pn::application_window_desc window_desc;

double FPS;
double FIXED_DT;
double dt;
double time_since_application_start;

bool exit;

json11::Json& JsonConfiguration() {
	static json11::Json config;
	return config;
}

std::string ReadFile(const std::string& file_name){
	std::ifstream t(file_name);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

json11::Json LoadJsonConfiguration(const std::string& file_name) {
	using namespace json11;

	std::string config_str = ReadFile(file_name);
	std::string error;

	json11::Json config = Json::parse(config_str, error);

	if (error.size()) {
		LogError("Could not load Json file: {}", error);
	}
	
	return config;
}

void LoadEngineConfiguration() {
	using namespace json11;

	JsonConfiguration() = LoadJsonConfiguration(PARTION_CONFIGURATION_FILE_NAME);
	Json& config = JsonConfiguration();

#define GET_STRING(JSON, DEFAULT) (((JSON).is_string()) ? ((JSON).string_value()): (DEFAULT))
#define GET_INT(JSON, DEFAULT) (((JSON).is_number()) ? ((JSON).int_value()): (DEFAULT))
#define GET_BOOL(JSON, DEFAULT) (((JSON).is_bool()) ? ((JSON).bool_value()): (DEFAULT))

	if (config.is_object()){
		LogInfo("Loading custom configuration");

		const Json& screen = config["screen"];
		if (screen.is_object()) {
			LogDebug("Loading Screen Configuration");
			window_desc.width = GET_INT(screen["width"], 1366);
			window_desc.height = GET_INT(screen["height"], 768);
			window_desc.fullscreen = GET_BOOL(screen["full_screen"], false);
		}

		const Json& resources = config["resources"];
		if (resources.is_object()){
			LogDebug("Loading Resources Configuration");
			pn::SetWorkingDirectory(GET_STRING(resources["path"], "."));
			pn::SetResourceDirectoryName(GET_STRING(resources["name"], "resources"));
		}
	}

#undef GET_STRING
#undef GET_INT
#undef GET_BOOL
}

// ---------- FUNCTIONS ---------

void Exit() {
	exit = true;
}
bool ShouldExit() {
	return exit;
}

}

}