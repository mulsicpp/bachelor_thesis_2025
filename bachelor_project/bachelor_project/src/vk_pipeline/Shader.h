#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include <string>
#include <vector>

namespace vk {

	class ShaderBuidler;

	class Shader : public utils::Move, public ptr::ToShared<Shader> {
		friend class ShaderBuilder;
	private:
		Handle<VkShaderModule> shader_module{};
		VkShaderStageFlagBits stage{ VK_SHADER_STAGE_VERTEX_BIT };

	public:
		Shader() = default;

		inline VkShaderModule handle() const { return *shader_module; }

		VkPipelineShaderStageCreateInfo get_create_info() const;
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