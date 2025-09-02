#pragma once

#include "external/glm.h"

struct CameraUBO {
	glm::mat4 view;
	glm::mat4 proj;
};

class Camera {
public:
	virtual glm::mat4 get_view() const = 0;
	virtual glm::mat4 get_proj(bool rtx = false) const = 0;
	virtual CameraUBO as_camera_ubo(bool rtx = false) const;
};

struct AppCamera : public Camera {
	glm::vec3 center{ 0.0 };

	float distance{ 6.0f };
	float theta{ 0.0f };
	float phi{ 0.0f };

	float aspect{ 1.0f };
	float fovy{ glm::radians(45.0f) };
	float near{ 0.05f };
	float far{ 10.0f };

	glm::mat4 get_view() const override;
	glm::mat4 get_proj(bool rtx = false) const override;
};
