#include "Animation.h"

void TranslationChannel::apply_for(float time) {
	if (!node->transform.raw) return;

	const auto& value = sampler.sample_at(time);
	node->transform.transform.raw.translation = value;
}

void RotationChannel::apply_for(float time) {
	if (!node->transform.raw) return;

	const auto& value = sampler.sample_at(time);
	node->transform.transform.raw.rotation = value;
}

void ScaleChannel::apply_for(float time) {
	if (!node->transform.raw) return;

	const auto& value = sampler.sample_at(time);
	node->transform.transform.raw.scale = value;
}

void Animation::apply_for(float time) {
	for (const auto& channel : channels) {
		channel->apply_for(time);
	}
}