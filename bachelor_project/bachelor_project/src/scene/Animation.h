#pragma once

#include "external/glm.h"

#include "Node.h"

#include <vector>

template<class T>
struct Sample {
	float time;
	T value;
};

enum class Interpolation {
	Step,
	Linear,
	CubicSpline
};

template<class T>
struct Sampler {
	std::vector<Sample<T>> samples;
	Interpolation interpolation;

	const T& sample_at(float time) {
		uint32_t index = 0;

		for (index = 0; index < samples.size() && samples[index].time <= time; index++);

		uint32_t prev_index = index - 1;

		if (prev_index < 0) {
			return samples[index].value;
		}

		if (index >= samples.size()) {
			return samples[index - 1].value;
		}

		if (interpolation == Interpolation::Step) {
			return samples[prev_index].value;
		}

		float factor = (time - samples[prev_index].time) / (samples[index].time - samples[prev_index].time);

		return samples[index].value * factor + samples[prev_index].value * (1.0f - factor);
	}
};



class AnimationChannel {
protected:
	ptr::Shared<Node> node{};

public:
	virtual void apply_for(float time) = 0;
};



class TranslationChannel : public AnimationChannel {
private:
	Sampler<glm::vec3> sampler;

public:
	void apply_for(float time) override;
};

class RotationChannel : public AnimationChannel {
private:
	Sampler<glm::quat> sampler;

public:
	void apply_for(float time) override;
};

class ScaleChannel : public AnimationChannel {
private:
	Sampler<glm::vec3> sampler;

public:
	void apply_for(float time) override;
};



struct Animation {
	std::vector<ptr::Owned<AnimationChannel>> channels;

	void apply_for(float time);
};