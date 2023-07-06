#include "engine.h"
#include "scene.h"
#include "pipeline.h"
#include "model.h"
#include "file_utility.h"
#include "settings.h"

namespace gpr5300
{
	class deferred_shading_test final : public Scene
	{
	public:
		void Begin() override;
		void Update(float dt) override;
		void End() override;

		void renderCube();
		void renderQuad();

		unsigned int quadVAO = 0;
		unsigned int quadVBO;

		unsigned int cubeVAO = 0;
		unsigned int cubeVBO = 0;

		Pipeline shaderGeometryPass;
		Pipeline shaderLightingPass;
		Pipeline shaderLightBox;

		Model backpack;
		std::vector<glm::vec3> objectPositions;

		unsigned int gBuffer;
		unsigned int gPosition, gNormal, gAlbedoSpec;
		unsigned int rboDepth;

		const unsigned int NR_LIGHTS = 32;
		std::vector<glm::vec3> lightPositions;
		std::vector<glm::vec3> lightColors;

	};

	void deferred_shading_test::renderQuad()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
	void deferred_shading_test::renderCube()
	{
		// initialize (if necessary)
		if (cubeVAO == 0)
		{
			float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				// right face
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
				 // bottom face
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				 // top face
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
			glGenVertexArrays(1, &cubeVAO);
			glGenBuffers(1, &cubeVBO);
			// fill buffer
			glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			// link vertex attributes
			glBindVertexArray(cubeVAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	void deferred_shading_test::Begin()
	{

		// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
		stbi_set_flip_vertically_on_load(true);

		// configure global opengl state
		// -----------------------------
		glEnable(GL_DEPTH_TEST);

		// build and compile shaders
		// -------------------------
		shaderGeometryPass = Pipeline("data/shaders/deferred_shading/g_buffer.vert", "data/shaders/deferred_shading/g_buffer.frag");
		shaderLightingPass = Pipeline("data/shaders/deferred_shading/deferred_shading.vert", "data/shaders/deferred_shading/deferred_shading.frag");
		shaderLightBox = Pipeline("data/shaders/deferred_shading/deferred_light_box.vert", "data/shaders/deferred_shading/deferred_light_box.frag");

		// load models
		// -----------
		backpack = Model("data/objects/backpack.obj");
		objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
		objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
		objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
		objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
		objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
		objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
		objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
		objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
		objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));

		// configure g-buffer framebuffer
		// ------------------------------
		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		// position color buffer
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
		// normal color buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
		// color + specular color buffer
		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
		// create and attach depth buffer (renderbuffer)
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// lighting info
		// -------------
		srand(13);
		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{
			// calculate slightly random offsets
			float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
			float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
			float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
			lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
			// also calculate random color
			float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
			lightColors.push_back(glm::vec3(rColor, gColor, bColor));
		}

		// shader configuration
		// --------------------
		shaderLightingPass.use();
		shaderLightingPass.setInt("gPosition", 0);
		shaderLightingPass.setInt("gNormal", 1);
		shaderLightingPass.setInt("gAlbedoSpec", 2);
	}

	void deferred_shading_test::Update(float dt)
	{
		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. geometry pass: render scene's geometry/color data into gbuffer
		// -----------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);	
		shaderGeometryPass.use();
		shaderGeometryPass.setMat4("projection", projection);
		shaderGeometryPass.setMat4("view", view);
		for (unsigned int i = 0; i < objectPositions.size(); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, objectPositions[i]);
			model = glm::scale(model, glm::vec3(0.25f));
			shaderGeometryPass.setMat4("model", model);
			backpack.Draw(shaderGeometryPass);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
		// -----------------------------------------------------------------------------------------------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		// send light relevant uniforms
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
			// update attenuation parameters and calculate radius
			const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = 0.7f;
			const float quadratic = 1.8f;
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
			// then calculate radius of light volume/sphere
			const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
			float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Radius", radius);
		}
		shaderLightingPass.setVec3("viewPos", camera->Position);
		// finally render quad
		renderQuad();

		// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ----------------------------------------------------------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
		// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
		// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
		glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. render lights on top of scene
		// --------------------------------
		shaderLightBox.use();
		shaderLightBox.setMat4("projection", projection);
		shaderLightBox.setMat4("view", view);
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, lightPositions[i]);
			model = glm::scale(model, glm::vec3(0.125f));
			shaderLightBox.setMat4("model", model);
			shaderLightBox.setVec3("lightColor", lightColors[i]);
			renderCube();
		}
	}

	void deferred_shading_test::End()
	{
		glDeleteVertexArrays(1, &cubeVAO);
		glDeleteBuffers(1, &cubeVBO);
	}
}

int main(int argc, char** argv)
{
	gpr5300::Camera camera;
	gpr5300::deferred_shading_test scene;
	scene.camera = &camera;
	gpr5300::Engine engine(&scene);
	engine.Run();

	return EXIT_SUCCESS;
}
