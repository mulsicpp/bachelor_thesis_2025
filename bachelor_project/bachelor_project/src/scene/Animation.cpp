#include "Animation.h"

#include "utils/dbg_log.h"

void TranslationChannel::apply_for(float time) {
	if (!node->transform.raw) return;

	auto value = sampler.sample_at(time);
	node->transform.transform.raw.translation = value;
}

void RotationChannel::apply_for(float time) {
	if (!node->transform.raw) return;

	auto value = sampler.sample_at(time);
	node->transform.transform.raw.rotation = glm::normalize(value);
}

void ScaleChannel::apply_for(float time) {
	if (!node->transform.raw) return;

	auto value = sampler.sample_at(time);
	node->transform.transform.raw.scale = value;
}

void Animation::apply_for(float time) {
	for (const auto& channel : channels) {
		channel->apply_for(time);
	}
}