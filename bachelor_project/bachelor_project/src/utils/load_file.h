#pragma once

#include <string>
#include <vector>

namespace utils {
	std::vector<uint8_t> load_file(const std::string& file_path);
}