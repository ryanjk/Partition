#include <IO\FileUtil.h>

#include <fstream>

#include <Utilities\Logging.h>

namespace pn {

bytes ReadFile(const std::string& filename) {
	std::ifstream input_file(filename, std::ios::binary | std::ios::ate);

	if (input_file.fail()) {
		LogError("Couldn't open file {}: {}", filename, strerror(errno));
		return {};
	}

	std::ifstream::pos_type size = input_file.tellg();
	if (size <= 0) {
		LogError("Couldn't read file {}: {}", filename, "File is empty");
		return {};
	}

	bytes file_bytes(size);

	input_file.seekg(0, std::ios::beg);
	input_file.read(&file_bytes[0], size);

	return file_bytes;
}

} // namespace pn