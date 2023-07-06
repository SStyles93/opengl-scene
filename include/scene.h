#pragma once
#include <iostream>
#include <vector>

#include <SDL.h>
#include <GL/glew.h>

#include <fmt/format.h>

#include "stb_image.h"
#include "file_utility.h"
#include "camera.h"
#include "pipeline.h"

namespace gpr5300
{

	class Scene
	{
	public:
		virtual ~Scene() = default;
		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Update(float dt) = 0;
		virtual void DrawImGui() {}
		virtual  void OnEvent(const SDL_Event& event) {}

		Camera* camera = nullptr;

		//action given by the engine
		bool action1 = false;
		bool action2 = false;

		///<summary>
		///Initializes a texture and returns it's ID
		///</summary>
		///<param name="filePath">The path at which the texture is located</param>
		static unsigned int LoadTexture(std::string_view filePath, bool gammaCorrection = false);
	};

} // namespace gpr5300
