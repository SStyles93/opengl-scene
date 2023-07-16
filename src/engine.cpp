#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <cassert>
#include <chrono>

#include "engine.h"
#include "settings.h"

namespace gpr5300
{
	Engine::Engine(Scene* scene) : scene_(scene) {}
	//Engine::Engine(Scene* scene, Camera* camera) : scene_(scene) {}

	void Engine::Run()
	{
		Begin();
		bool isOpen = true;

		std::chrono::time_point<std::chrono::system_clock> clock = std::chrono::system_clock::now();
		while (isOpen)
		{
			const auto start = std::chrono::system_clock::now();
			using seconds = std::chrono::duration<float, std::ratio<1, 1>>;
			const auto dt = std::chrono::duration_cast<seconds>(start - clock);
			clock = start;

			float sensitivity = 0.1f; // change this value to your liking

			//Manage SDL event
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
				case SDL_QUIT:
					isOpen = false;
					break;
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event)
					{
					case SDL_WINDOWEVENT_CLOSE:
						isOpen = false;
						break;
					case SDL_WINDOWEVENT_RESIZED:
					{
						glm::uvec2 newWindowSize;
						newWindowSize.x = event.window.data1;
						newWindowSize.y = event.window.data2;
						//TODO do something with the new size
						break;
					}
					default:
						break;
					}
					break;
				}
				case SDL_MOUSEMOTION:
					if (event.button.button == SDL_PRESSED && scene_->camera != nullptr)
					{

						event.motion.xrel *= sensitivity;
						event.motion.yrel *= sensitivity;

						scene_->camera->Yaw -= event.motion.xrel;
						scene_->camera->Pitch += event.motion.yrel;

						// make sure that when pitch is out of bounds, screen doesn't get flipped
						if (scene_->camera->Pitch > MAX_PITCH)
							scene_->camera->Pitch = MAX_PITCH;
						if (scene_->camera->Pitch < MIN_PITCH)
							scene_->camera->Pitch = MIN_PITCH;

						glm::vec3 front;
						front.x = cos(glm::radians(scene_->camera->Yaw)) * cos(glm::radians(scene_->camera->Pitch));
						front.y = sin(glm::radians(scene_->camera->Pitch));
						front.z = sin(glm::radians(scene_->camera->Yaw)) * cos(glm::radians(scene_->camera->Pitch));
						scene_->camera->Front = glm::normalize(front);

						scene_->camera->Fov -= (float)event.wheel.y;
						if (scene_->camera->Fov < MIN_FOV)
							scene_->camera->Fov = MIN_FOV;
						if (scene_->camera->Fov > MAX_FOV)
							scene_->camera->Fov = MAX_FOV;
					}
					break;
				case SDL_KEYDOWN:
					if (scene_->camera != nullptr)
					{


						//Increase camera movement speed
						if (event.key.keysym.scancode == CAM_SPEED_INCREASE_KEY && scene_->camera->MovementSpeed < MAX_CAM_SPEED)
						{
							scene_->camera->MovementSpeed += 10;
							std::cout << scene_->camera->MovementSpeed;
						}
						else if (scene_->camera->MovementSpeed > MAX_CAM_SPEED)
						{
							scene_->camera->MovementSpeed = MAX_CAM_SPEED;
						}
						//Decrease camera movement speed 
						if (event.key.keysym.scancode == CAM_SPEED_DECREASE_KEY && scene_->camera->MovementSpeed > MIN_CAM_SPEED)
						{
							scene_->camera->MovementSpeed -= 10;
							std::cout << scene_->camera->MovementSpeed;
						}
						else if (scene_->camera->MovementSpeed < MIN_CAM_SPEED)
						{
							scene_->camera->MovementSpeed = MIN_CAM_SPEED;
						}
					}
					break;
				default:
					break;
				}
				scene_->OnEvent(event);
				ImGui_ImplSDL2_ProcessEvent(&event);

			}
			if (scene_->camera != nullptr)
			{

				const uint8_t* keyboardState = SDL_GetKeyboardState(nullptr);
				Camera_Movement cameraMovementState
				{
					.FORWARD = static_cast<bool>(keyboardState[SDL_SCANCODE_W]),
					.BACKWARD = static_cast<bool>(keyboardState[SDL_SCANCODE_S]),
					.LEFT = static_cast<bool>(keyboardState[SDL_SCANCODE_A]),
					.RIGHT = static_cast<bool>(keyboardState[SDL_SCANCODE_D]),
					.UP = static_cast<bool>(keyboardState[SDL_SCANCODE_E]),
					.DOWN = static_cast<bool>(keyboardState[SDL_SCANCODE_Q])
				};

				if (cameraMovementState.HasMovement())
				{
					scene_->camera->ProcessKeyboard(cameraMovementState, dt.count());
				}

				//Action given to the scene
				scene_->action1 = static_cast<bool>(keyboardState[ACTION_KEY1]);
				scene_->action2 = static_cast<bool>(keyboardState[ACTION_KEY2]);
			}
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			scene_->Update(dt.count());

			//Generate new ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(window_);
			ImGui::NewFrame();

			scene_->DrawImGui();
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			SDL_GL_SwapWindow(window_);
		}
		End();
	}

	void Engine::Begin()
	{

		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

		//SDL_SetRelativeMouseMode(SDL_TRUE);

		// Set our OpenGL version.
#if true
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
#else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		constexpr auto windowSize = glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT);
		window_ = SDL_CreateWindow(
			"GPR5300",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			windowSize.x,
			windowSize.y,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
		glRenderContext_ = SDL_GL_CreateContext(window_);
		//setting vsync
		SDL_GL_SetSwapInterval(1);

		if (GLEW_OK != glewInit())
		{
			assert(false && "Failed to initialize OpenGL context");
		}

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Keyboard Gamepad
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		ImGui::StyleColorsClassic();
		ImGui_ImplSDL2_InitForOpenGL(window_, glRenderContext_);
		ImGui_ImplOpenGL3_Init("#version 300 es");

		scene_->Begin();
	}

	void Engine::End()
	{
		scene_->End();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
		SDL_GL_DeleteContext(glRenderContext_);
		SDL_DestroyWindow(window_);
		SDL_Quit();

	}
} // namespace gpr5300
