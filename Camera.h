#pragma once

#ifndef CAMERA_H
#define CAMERA_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods */
enum class Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

/* Default camera values */
const auto YAW = -90.f;

const auto PITCH = 0.f;

const auto SPEED = 2.5f;

const auto SENSITIVITY = 0.1f;

const auto ZOOM = 45.f;

/* An abstract camera class that processes inputand calculates the corresponding Euler Angles, Vectorsand Matrices for use in OpenGL */
class Camera
{
public:
	/* Camera Attributes */
	glm::vec3 Position;

	glm::vec3 Front;

	glm::vec3 Up;

	glm::vec3 Right;

	glm::vec3 WorldUp;

	/* Euler Angles */
	float Yaw;

	float Pitch;

	/* Camera options */
	float MovementSpeed;

	float MouseSensitivity;

	float Zoom;

	/* Constructor with vectors */
	Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 0.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f), float yaw = YAW,
	       float pitch = PITCH);

	/* Constructor with scalar values */
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	/* Returns the view matrix calculated using Euler Angles and the LookAt Matrix */
	glm::mat4 GetViewMatrix() const;

	/* Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems) */
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	/* Processes input received from a mouse input system.Expects the offset value in both the xand y direction. */
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	/* Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis */
	void ProcessMouseScroll(float yoffset);

private:
	/* Calculates the front vector from the Camera's (updated) Euler Angles */
	void updateCameraVectors();
};
#endif
