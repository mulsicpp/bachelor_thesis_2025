#include "Camera.h"

CameraUBO Camera::as_camera_ubo(bool rtx) const {
    return CameraUBO{ get_view(), get_proj(rtx), get_focus_distance(), get_relative_lens_radius() };
}

glm::mat4 AppCamera::get_view() const {
    glm::mat4 view = glm::mat4{ 1.0f };
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -distance));
    view = glm::rotate(view, phi, glm::vec3{ 1.0f, 0.0f, 0.0f });
    view = glm::rotate(view, theta, glm::vec3{ 0.0f, 1.0f, 0.0f });
    view = glm::translate(view, -center);
    view = glm::scale(view, glm::vec3{ 1.0f, -1.0f, 1.0f });

    return view;
}

glm::mat4 AppCamera::get_proj(bool rtx) const {
    if(!rtx) {
        return glm::perspective(fovy, aspect, near * distance, far * distance);
    }

    auto tan_fovy_half = glm::tan(fovy * 0.5f);

    return glm::scale(glm::mat4{1.0f}, glm::vec3{tan_fovy_half * aspect, tan_fovy_half, 1.0f});
}

float AppCamera::get_focus_distance() const {
    return focus_distance > 0.0f ? focus_distance : distance;
}

float AppCamera::get_relative_lens_radius() const {
    return relative_lens_radius;
}