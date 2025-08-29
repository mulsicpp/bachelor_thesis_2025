#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"

#include "external/volk.h"
#include "external/glm.h"

#include "Blas.h"

namespace vk {

    struct TlasInstance {
        using Ref = TlasInstance&;

        ptr::Shared<Blas> blas{};

        glm::mat4 transform{ 1.0f };

        VkAccelerationStructureInstanceKHR as_vk_struct() const;
    };

    class TlasBuilder;

    class Tlas : public utils::Move, public ptr::ToShared<Tlas> {
        friend class TlasBuilder;
    private:
        Buffer update_scratch_buffer{};
        Buffer buffer{};
        Handle<VkAccelerationStructureKHR> tlas{};

        std::vector<TlasInstance> _instances{};

    public:
        Tlas() = default;

        inline VkAccelerationStructureKHR handle() const { return *tlas; }

        VkDeviceAddress device_address() const;
    };

    class TlasBuilder
    {
    public:
        using Ref = TlasBuilder&;

    private:
        std::vector<TlasInstance> _instances{};
    public:
        Ref instances(const std::vector<TlasInstance>& instances) { _instances = instances; return *this; }
        Ref add_instance(const TlasInstance& instance) { _instances.push_back(instance); return *this; }

        Tlas build() const;
    };
}