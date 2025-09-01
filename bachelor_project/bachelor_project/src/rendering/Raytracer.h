#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_rtx/RtxPipeline.h"

class RaytracerBuilder;

class Raytracer : public utils::Move, public ptr::ToShared<Raytracer> {
    friend class RaytracerBuilder;
private:
    ptr::Shared<vk::PipelineLayout> pipeline_layout{};
    vk::RtxPipeline pipeline{};
    vk::SBT sbt{};

public:
    Raytracer() = default;
};

class RaytracerBuilder {
public:
    using Ref = RaytracerBuilder&;

private:
    // TODO raytracer builder parameters

public:
    RaytracerBuilder() = default;

    Raytracer build() const;
};