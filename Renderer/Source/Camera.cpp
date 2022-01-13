#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters //DONE
        
        this->cameraFrontDirection.x = cameraTarget.x;
        this->cameraFrontDirection.y = cameraTarget.y;
        this->cameraFrontDirection.z = cameraTarget.z - 1.0f;

        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO //DONE
        if (direction == MOVE_FORWARD) this->cameraPosition += speed * this->cameraFrontDirection;
        if (direction == MOVE_BACKWARD) this->cameraPosition -= speed * this->cameraFrontDirection;
        if (direction == MOVE_RIGHT) this->cameraPosition += speed * this->cameraRightDirection;
        if (direction == MOVE_LEFT) this->cameraPosition -= speed * this->cameraRightDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO //DONE
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        
        this->cameraRotateDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraRotateDirection.y = sin(glm::radians(pitch));
        this->cameraRotateDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraTarget = glm::normalize(this->cameraRotateDirection);
    }
}