/**
 * Implementation of RigidBody class and its derived classes
 * @author: Quan Bui
 * @version: 02/26/2020
 */ 
#include "RigidBody.h"

void DynamicBody::move() 
{
    position_ += linearVelocity_;
    linearVelocity_ += linearAcceleration_;

    // Due to air drag
    //linearVelocity_ -= glm::vec3{0.0001f};
}

void KinematicBody::move()
{
    
}