#include "Camera.h"

#include <iostream>

Camera::Camera(glm::vec3 const &position, glm::vec3 const &front, glm::vec3 const &up, float viewportAspectRatio) 
    : position_(position), front_(glm::normalize(front)), up_(glm::normalize(up))
{
    updateViewMatrix();
    updateProjectionMatrix(viewportAspectRatio);
}

void Camera::updateViewMatrix() 
{
    viewMatrix_ = glm::lookAt(position_, position_ + front_, up_);
}

void Camera::updateProjectionMatrix(float viewportAspectRatio)
{
    // perspective(float fov, float aspect, float zNear, float zFar) -- parameter list of perspective
    projectionMatrix_ = glm::perspective(glm::radians(fieldOfView_), viewportAspectRatio, 0.01f, 150.f);
}

/**
 *  Nothing fancy here just to move the view of camera.
 *  Should include: yaw, pitch. TODO: roll
 */ 
void Camera::moveView(float deltaX, float deltaY) 
{
    yaw_ += deltaX * sensitivityX_;
    pitch_ += deltaY * sensitivityY_;

    // Constrain pitch so there's no weird effect on the view when 
    //  look all the way up or all the way down.
    if (pitch_ > 89.0f) {
        pitch_ = 89.0f;
    }
    if (pitch_ < -89.0f) {
        pitch_ = -89.0f;
    }

    // Calculate the front vector, a.k.a where the camera is looking at
    glm::vec3 frontVector;
    frontVector.x = cosf(glm::radians(pitch_)) * cosf(glm::radians(yaw_));
    frontVector.y = sinf(glm::radians(pitch_));
    frontVector.z = cosf(glm::radians(pitch_)) * sinf(glm::radians(yaw_));

    // This one updates the front vector but not in the way that I want
    front_ = normalize(frontVector);
    updateViewMatrix();
}

void Camera::movePosition(MOVEMENT_SET const &movementDirection, float deltaTime) 
{ 
    switch(movementDirection) {
    case LEFT:
        position_ -= glm::normalize(glm::cross(front_, up_)) * movementSpeed_ * deltaTime;
        break;
    case RIGHT:
        position_ += glm::normalize(glm::cross(front_, up_)) * movementSpeed_ * deltaTime;
        break;
    case FORWARD:
        position_ += front_ * movementSpeed_ * deltaTime;
        break;
    case BACKWARD:
        position_ -= front_ * movementSpeed_ * deltaTime;
        break;    
    }
    
    updateViewMatrix();
}

void Camera::zoomFieldOfView(double deltaY, float viewportAspectRatio) 
{
    if (fieldOfView_ >= 1.f && fieldOfView_ <= 45.f) {
        fieldOfView_ -= static_cast<float>(deltaY);
    }
    if (fieldOfView_ <= 1.f) {
        fieldOfView_ = 1.f;
    }
    if (fieldOfView_ >= 45.f) {
        fieldOfView_ = 45.f;
    }

    updateProjectionMatrix(viewportAspectRatio);
}