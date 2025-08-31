#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"

namespace vk {

    class SBTBuilder;

    class SBT : public utils::Move, public ptr::ToShared<SBT> {
        friend class SBTBuilder;
    private:
        Buffer _buffer{};

    public:
        SBT() = default;
    };

    class SBTBuilder {
    public:
        using Ref = SBTBuilder&;

    private:
        // TODO parameters
    public:
        SBTBuilder() = default;

        SBT build() const;
    };
}