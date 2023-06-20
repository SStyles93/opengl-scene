#pragma once
#include "SDL_scancode.h"

namespace gpr5300
{
	constexpr float SCREEN_WIDTH = 1290.0f;
	constexpr float SCREEN_HEIGHT = 720.0f;

	//Camera values
	constexpr float MAX_PITCH = 89.0f;
	constexpr float MIN_PITCH = -89.0f;

	constexpr float MAX_FOV = 45.0f;
	constexpr float MIN_FOV = 1.0f;

	constexpr float YAW = -90.0f;
	constexpr float PITCH = 0.0f;
	constexpr float ZOOM = 45.0f;

	constexpr float SPEED = 5.0f;
	constexpr float CAM_INCREASE_KEY = SDL_SCANCODE_2;
	constexpr  float MAX_CAM_SPEED = 100;
	constexpr float CAM_DECREASE_KEY = SDL_SCANCODE_1;
	constexpr  float MIN_CAM_SPEED = 0.1;
	constexpr float SENSITIVITY = 0.1f;

}
