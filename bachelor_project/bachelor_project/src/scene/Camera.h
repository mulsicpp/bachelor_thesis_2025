#pragma once

#include "external/glm.h"

struct CameraUBO {
	glm::mat4 view;
	glm::mat4 proj;
};

class Camera {
public:
	virtual CameraUBO as_camera_ubo() const = 0;
};

struct AppCamera : Camera {
	glm::vec3 center{ 0.0 };

	float distance{ 6.0f };
	float theta{ 0.0f };
	float phi{ 0.0f };

	float aspect{ 1.0f };
	float near{ 0.05f };
	float far{ 10.0f };

	CameraUBO as_camera_ubo() const override;
};
