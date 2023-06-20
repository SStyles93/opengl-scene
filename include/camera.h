#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <vector>

#include "settings.h"

namespace gpr5300
{

	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	struct Camera_Movement
	{
		bool FORWARD{};
		bool BACKWARD{};
		bool LEFT{};
		bool RIGHT{};
		bool UP{};
		bool DOWN{};

		[[nodiscard]] bool HasMovement() const;
	};

	// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
	class Camera
	{
	public:
		// camera Attributes
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;
		// euler Angles
		float Yaw;
		float Pitch;
		// camera options
		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;
		float Fov = 45.0f;

		/// <summary>
		/// Basic constructor
		/// </summary>
		Camera();
		// constructor with vectors
		Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
		// constructor with scalar values
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

		// returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4 GetViewMatrix();

		// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void ProcessKeyboard(Camera_Movement direction, float deltaTime);

		// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
		void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch);

		// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void ProcessMouseScroll(float yoffset);

	private:
		// calculates the front vector from the Camera's (updated) Euler Angles
		void UpdateCameraVectors();
	};
};


