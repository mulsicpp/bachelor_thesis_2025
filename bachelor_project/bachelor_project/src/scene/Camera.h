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
