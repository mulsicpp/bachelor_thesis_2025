#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"

#include "external/volk.h"

namespace vk
{
    class Blas : utils::Move, ptr::ToShared<Blas>
    {
    private:
        Buffer buffer{};
        Handle<VkAccelerationStructureKHR> blas{};

    public:
        Blas() = default;

		inline VkAccelerationStructureKHR handle() const { return *blas; }
    };

    class BlasBuilder
    {
        using Ref = BlasBuilder &;
    private:
        // TODO: build parameters
    public:
        Blas build() const;
    };
}