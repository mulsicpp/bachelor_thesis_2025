#pragma once

#include "utils/ptr.h"

#include "vk_pipeline/Shader.h"

#include <variant>

namespace vk {

    enum class ShaderGroupType {
        General,
        Hit
    };

    struct ShaderGroupGeneral {
        using Ref = ShaderGroupGeneral&;

        ptr::Shared<Shader> general{};

        inline Ref set_general(const ptr::Shared<Shader>& general) { this->general = general; return *this; }
    };

    struct ShaderGroupHit {
        using Ref = ShaderGroupHit&;

        ptr::Shared<Shader> closest_hit{};
        ptr::Shared<Shader> any_hit{};

        inline Ref set_closest_hit(const ptr::Shared<Shader>& closest_hit) { this->closest_hit = closest_hit; return *this; }
        inline Ref set_any_hit(const ptr::Shared<Shader>& any_hit) { this->any_hit = any_hit; return *this; }
    };
    
    class ShaderGroup {
    private:
        std::variant<
            ShaderGroupGeneral,
            ShaderGroupHit
        > group{};

    public:
        ShaderGroup() = default;

        inline ShaderGroup(const ShaderGroupGeneral& general_group) :group{ general_group } {}
        inline ShaderGroup(const ShaderGroupHit& hit_group) :group{ hit_group } {}

        inline ShaderGroupType type() const { return group.index() == 0 ? ShaderGroupType::General : ShaderGroupType::Hit; }

        inline ShaderGroupGeneral& as_general() { return std::get<0>(group); }
        inline const ShaderGroupGeneral& as_general() const { return std::get<0>(group); }

        inline ShaderGroupHit& as_hit() { return std::get<1>(group); }
        inline const ShaderGroupHit& as_hit() const { return std::get<1>(group); }
    };
}