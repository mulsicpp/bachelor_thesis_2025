#include "load_file.h"

#include <fstream>

namespace utils {
	std::vector<uint8_t> load_file(const std::string& file_path) {
        std::ifstream file(file_path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("File read failed! Could not open '" + file_path + "'");
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<uint8_t> buffer(file_size);

        file.seekg(0);
        file.read((char*)buffer.data(), file_size);

        file.close();

        return buffer;
	}
}