#pragma once

#include "utils/move.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace vk {

	class ShaderBuidler;

	class Shader {
		friend class ShaderBuilder;
	private:
		VkShaderModule shader_module;
		VkShaderStageFlagBits stage;

	public:
		Shader();

		inline VkShaderModule handle() const { return shader_module; }

		VkPipelineShaderStageCreateInfo get_create_info();

	private:
		void destroy();

		MOVE_SEMANTICS_VK_DEFAULT(Shader, shader_module)
	};

	class ShaderBuilder {
	public:
		using Ref = ShaderBuilder&;
	private:
		uint32_t* _code;
		size_t _size;
		VkShaderStageFlagBits _stage;

		std::vector<uint8_t> _file_data;
	public:
		ShaderBuilder();

		Shader build();

		inline Ref code(uint32_t* code, size_t size) {
			_code = code;
			_size = size;
			return *this;
		}

		inline Ref stage(VkShaderStageFlagBits stage) { _stage = stage; return *this; }

		inline Ref vertex() { _stage = VK_SHADER_STAGE_VERTEX_BIT; return *this; }
		inline Ref fragment() { _stage = VK_SHADER_STAGE_FRAGMENT_BIT; return *this; }

		Ref load_spirv(const std::string& file_path);
	};
}