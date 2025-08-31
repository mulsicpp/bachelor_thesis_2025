#include "ShaderGroup.h"

namespace vk {
    ShaderGroupId ShaderGroup::next_id{ 0 };


    ShaderGroup ShaderGroup::create_general(ptr::Shared<Shader> general) {
        ShaderGroupGeneral general_group{};
        general_group.general = general;

        ShaderGroup group{};
        group.group = general_group;
        group.id = ++next_id;

        return group;
    }


    ShaderGroup ShaderGroup::create_hit(ptr::Shared<Shader> closest_hit, ptr::Shared<Shader> any_hit) {
        ShaderGroupHit hit_group{};
        hit_group.closest_hit = closest_hit;
        hit_group.any_hit = any_hit;

        ShaderGroup group{};
        group.group = hit_group;
        group.id = ++next_id;

        return group;
    }

    ShaderGroup ShaderGroup::create_hit_closest(ptr::Shared<Shader> closest_hit) {
        return create_hit(closest_hit, {});
    }

    ShaderGroup ShaderGroup::create_hit_any(ptr::Shared<Shader> any_hit) {
        return create_hit({}, any_hit);
    }
}