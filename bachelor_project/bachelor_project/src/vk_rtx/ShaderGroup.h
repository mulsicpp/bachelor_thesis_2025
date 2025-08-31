#pragma once

#include "utils/move.h"
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
    };

    struct ShaderGroupHit {
        using Ref = ShaderGroupHit&;

        ptr::Shared<Shader> closest_hit{};
        ptr::Shared<Shader> any_hit{};
    };

    using ShaderGroupId = uint64_t;
    
    class ShaderGroup {
    private:
        static ShaderGroupId next_id;

        ShaderGroupId id{ 0 };
        std::variant<
            ShaderGroupGeneral,
            ShaderGroupHit
        > group{};

    public:
        ShaderGroup() = default;

        static ShaderGroup create_general(ptr::Shared<Shader> general);

        static ShaderGroup create_hit(ptr::Shared<Shader> closest_hit, ptr::Shared<Shader> any_hit);
        static ShaderGroup create_hit_closest(ptr::Shared<Shader> closest_hit);
        static ShaderGroup create_hit_any(ptr::Shared<Shader> any_hit);

        inline ShaderGroupType type() const { return group.index() == 0 ? ShaderGroupType::General : ShaderGroupType::Hit; }

        inline const ShaderGroupGeneral& as_general() const { return std::get<0>(group); }
        inline const ShaderGroupHit& as_hit() const { return std::get<1>(group); }
    };
}