#pragma once

#include "external/glm.h"

#include <vector>
#include <queue>

#include "utils/ptr.h"

#include "Mesh.h"

struct RawTransform {
	glm::vec3 translation{ 0.0f };
	glm::quat rotation{};
	glm::vec3 scale{ 1.0f };

	inline glm::mat4 to_mat() {
		//return glm::mat4{ 1.0 };
		return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}
};

struct NodeTransform {
	bool raw{ false };
	union {
		glm::mat4 matrix;
		RawTransform raw;
	} transform;

	inline NodeTransform() : NodeTransform{ glm::mat4{ 1.0 } } {}

	inline NodeTransform(glm::mat4 matrix) : raw{ false }, transform{} {
		transform.matrix = matrix;
	}

	inline NodeTransform(RawTransform raw) : raw{ true }, transform{} {
		transform.raw = raw;
	}
};

struct Node {
	NodeTransform transform{};
	glm::mat4 global_transform{ 1.0 };
	std::vector<ptr::Shared<Node>> children{};
	ptr::Shared<Mesh> mesh{};
	ptr::Shared<Skin> skin{};
	std::vector<vk::SubBuffer> dynamic_positions{};

	void update_global_transfrom(glm::mat4 parent_transform = { 1.0f });
};

class NodeIterator {
private:
	std::queue<ptr::Shared<Node>> nodes{};

	inline NodeIterator(const ptr::Shared<Node>& node) {
		nodes.push(node);
	}

	inline NodeIterator(const std::vector<ptr::Shared<Node>>& nodes) {
		for (const auto& node : nodes) {
			this->nodes.push(node);
		}
	}

public:
	inline bool has_next() const { return nodes.size() > 0; }

	inline ptr::Shared<Node> next() {
		auto node = nodes.front();
		nodes.pop();
		for (const auto& child : node->children) {
			nodes.push(child);
		}
		return node;
	}

	inline static NodeIterator from(const ptr::Shared<Node>& node) { return { node }; }
	inline static NodeIterator from(const std::vector<ptr::Shared<Node>>& nodes) { return { nodes }; }
};