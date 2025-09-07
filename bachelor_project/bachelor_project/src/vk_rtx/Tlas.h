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
        Buffer instances_staging_buffer{};
        Buffer instances_buffer{};

        Buffer build_scratch_buffer{};
        Buffer update_scratch_buffer{};
        Buffer buffer{};
        Handle<VkAccelerationStructureKHR> tlas{};

        bool _dynamic{};
        bool _fast_build{};
        std::vector<TlasInstance> _instances{};

    public:
        Tlas() = default;

        inline VkAccelerationStructureKHR handle() const { return *tlas; }

        inline bool is_dynamic() const { return _dynamic; }
        inline bool was_fast_build() const { return _fast_build; }
        inline const std::vector<TlasInstance>& instances() const { return _instances; }
        inline std::vector<TlasInstance>& instances() { return _instances; }

        VkDeviceAddress device_address() const;

        void refit(const std::vector<TlasInstance>& instances = {});
        void rebuild(const std::vector<TlasInstance>& instances = {});
    private:
        void build(ASBuildMode mode, const std::vector<TlasInstance>& instances = {});
    };

    class TlasBuilder
    {
    public:
        using Ref = TlasBuilder&;

    private:
        std::vector<TlasInstance> _instances{};
        bool _dynamic{ false };
        bool _fast_build{ false };
    public:
        Ref instances(const std::vector<TlasInstance>& instances) { _instances = instances; return *this; }
        Ref add_instance(const TlasInstance& instance) { _instances.push_back(instance); return *this; }

        inline Ref dynamic(bool dynamic) { _dynamic = dynamic; return *this; }
        inline Ref fast_build(bool fast_build) { _fast_build = fast_build; return *this; }

        Tlas build() const;
    };
}