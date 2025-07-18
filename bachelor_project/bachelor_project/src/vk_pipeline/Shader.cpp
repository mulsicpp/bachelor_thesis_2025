#include "Shader.h"

#include "utils/load_file.h"

#include "vk_core/Context.h"

#include <filesystem>

namespace vk {

	VkPipelineShaderStageCreateInfo Shader::get_create_info() const {
		VkPipelineShaderStageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage = stage;
		info.module = *shader_module;
		info.pName = "main";

		return info;
	}

	ShaderBuilder::ShaderBuilder()
		: _code{ nullptr }
		, _size{ 0 }
		, _file_data{}
		, _stage{ VK_SHADER_STAGE_VERTEX_BIT }
	{}

	ShaderBuilder::Ref ShaderBuilder::load_spirv(const std::string& file_path) {
		_file_data = utils::load_file(file_path);
		_code = (uint32_t*)_file_data.data();
		_size = _file_data.size();

		return *this;
	}



	Shader ShaderBuilder::build() {
		if (_code == nullptr) {
			throw std::runtime_error("Shader creation failed! Code was 'nullptr'");
		}

		if (_size == 0) {
			throw std::runtime_error("Shader creation failed! Code size was '0'");
		}

		Shader shader;

		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pCode = _code;
		create_info.codeSize = _size;

		if (vkCreateShaderModule(Context::get()->get_device(), &create_info, nullptr, &*shader.shader_module) != VK_SUCCESS) {
			throw std::runtime_error("Shader creation failed!");
		}
		shader.stage = _stage;

		return shader;
	}
}