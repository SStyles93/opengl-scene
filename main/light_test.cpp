#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "fmt/format.h"

#include "stb_image.h"

#include "engine.h"
#include "scene.h"
#include "pipeline.h"
#include "settings.h"

namespace gpr5300
{
	class light_test final : public Scene
	{
	public:
		void Begin() override;
		void End() override;
		void Update(float dt) override;


		/// <summary>
		/// Sets a directionnal light
		/// </summary>
		/// <param name="pipeline">Sets the pipeline on which you want to add a light</param>
		/// <param name="lightPosition">The position of the light</param>
		/// <param name="lightAmbient">The ambient parameter</param>
		/// <param name="lightDiffuse">The diffuse parameter</param>
		/// <param name="lightSpecular">The specular parameter</param>
		void SetLight(
			Pipeline pipeline, glm::vec3 lightPosition,
			glm::vec3 lightAmbient, glm::vec3 lightdiffuse, glm::vec3 lightSpecular);
		/// <summary>
		/// Set spot light
		/// </summary>
		/// <param name="pipeline">Sets the pipeline on which you want to add a light</param>
		/// <param name="lightPosition">The position of the light</param>
		/// <param name="lightAmbient">The ambient parameter</param>
		/// <param name="lightDiffuse">The diffuse parameter</param>
		/// <param name="lightSpecular">The specular parameter</param>
		/// <param name="lightConstant">The light constant value</param>
		/// <param name="lightLinear">The light linear value</param>
		/// <param name="lightQuadratic">The light quadratic value</param>
		void SetLight(
			Pipeline pipeline, glm::vec3 lightPosition,
			glm::vec3 lightAmbient, glm::vec3 lightDiffuse, glm::vec3 lightSpecular,
			float lightConstant, float lightLinear, float lightQuadratic);
		/// <summary>
		/// Set spot light
		/// </summary>
		/// <param name="pipeline">Sets the pipeline on which you want to add a light</param>
		/// <param name="lightPosition">The position of the light</param>
		/// <param name="lightDirection">The direction of the light</param>
		/// <param name="lightAmbient">The ambient parameter</param>
		/// <param name="lightDiffuse">The diffuse parameter</param>
		/// <param name="lightSpecular">The specular parameter</param>
		/// <param name="lightConstant">The light constant value</param>
		/// <param name="lightLinear">The light linear value</param>
		/// <param name="lightQuadratic">The light quadratic value</param>
		/// <param name="cutOff">The light cut off</param>
		/// <param name="outerCutOff">the light outer cut off</param>
		void SetLight(Pipeline pipeline, glm::vec3 lightPosition, glm::vec3 lightDirection,
			glm::vec3 lightAmbient, glm::vec3 lightDiffuse, glm::vec3 lightSpecular,
			float lightConstant, float lightLinear, float lightQuadratic,
			float cutOff, float outerCutOff);

		float time_ = 0;

		SDL_Event event;

		// world space positions of our cubes
		glm::vec3 cubePositions[10] = {
			glm::vec3(0.0f,  0.0f,  0.0f),
			glm::vec3(2.0f,  5.0f, -15.0f),
			glm::vec3(-1.5f, -2.2f, -2.5f),
			glm::vec3(-3.8f, -2.0f, -12.3f),
			glm::vec3(2.4f, -0.4f, -3.5f),
			glm::vec3(-1.7f,  3.0f, -7.5f),
			glm::vec3(1.3f, -2.0f, -2.5f),
			glm::vec3(1.5f,  2.0f, -2.5f),
			glm::vec3(1.5f,  0.2f, -1.5f),
			glm::vec3(-1.3f,  1.0f, -1.5f)
		};

		glm::vec3 lightPos{ 1.2f, 1.0f, 2.0f };

		int lightCount = 5;
		glm::vec3 pointLightPositions[128]{};

		Pipeline lightingShader;
		Pipeline lightCubeShader;

		unsigned int VBO, cubeVAO;
		unsigned int lightCubeVAO;

		unsigned int diffuseMap;
		unsigned int specularMap;

		bool blinn = false;
	};

	unsigned int LoadTexture(std::string_view filePath)
	{
		unsigned int textureID;

		//Generate texture ID
		glGenTextures(1, &textureID);
		//Load Texture
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filePath.data(), &width, &height, &nrChannels, 0);
		//Load texture
		if (data)
		{
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		//Free stb data
		stbi_image_free(data);

		return textureID;
	}

	void light_test::SetLight(
		Pipeline pipeline, glm::vec3 lightPosition,
		glm::vec3 lightAmbient, glm::vec3 lightdiffuse, glm::vec3 lightSpecular)
	{
		pipeline.setVec3("dirLight.direction", lightPosition);
		pipeline.setVec3("dirLight.ambient", lightAmbient);
		pipeline.setVec3("dirLight.diffuse", lightdiffuse);
		pipeline.setVec3("dirLight.specular", lightSpecular);
	}
	void light_test::SetLight(
		Pipeline pipeline, glm::vec3 lightPosition,
		glm::vec3 lightAmbient, glm::vec3 lightDiffuse, glm::vec3 lightSpecular,
		float lightConstant, float lightLinear, float lightQuadratic)
	{
		//Set the light's values
		pipeline.setVec3(fmt::format("pointLights{}.position", lightCount), lightPosition);
		pipeline.setVec3(fmt::format("pointLights{}.ambient", lightCount), lightAmbient);
		pipeline.setVec3(fmt::format("pointLights{}.diffuse", lightCount), lightDiffuse);
		pipeline.setVec3(fmt::format("pointLights{}.specular", lightCount), lightSpecular);
		pipeline.setFloat(fmt::format("pointLights{}.constant", lightCount), lightConstant);
		pipeline.setFloat(fmt::format("pointLights{}.linear", lightCount), lightLinear);
		pipeline.setFloat(fmt::format("pointLights{}.quadratic", lightCount), lightQuadratic);
	}
	void light_test::SetLight(Pipeline pipeline, glm::vec3 lightPosition, glm::vec3 lightDirection,
		glm::vec3 lightAmbient, glm::vec3 lightDiffuse, glm::vec3 lightSpecular,
		float lightConstant, float lightLinear, float lightQuadratic,
		float cutOff, float outerCutOff)
	{
		lightingShader.setVec3("spotLight.position", lightPosition);
		lightingShader.setVec3("spotLight.direction", lightDirection);
		lightingShader.setVec3("spotLight.ambient", lightAmbient);
		lightingShader.setVec3("spotLight.diffuse", lightDiffuse);
		lightingShader.setVec3("spotLight.specular", lightSpecular);
		lightingShader.setFloat("spotLight.constant", lightConstant);
		lightingShader.setFloat("spotLight.linear", lightLinear);
		lightingShader.setFloat("spotLight.quadratic", lightQuadratic);
		lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(cutOff)));
		lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(outerCutOff)));
	}

	void light_test::Begin()
	{
		lightingShader = Pipeline("data/shaders/hello_cube/multiple_lights.vert", "data/shaders/hello_cube/multiple_lights.frag");
		lightCubeShader = Pipeline("data/shaders/hello_cube/light_cube.vert", "data/shaders/hello_cube/light_cube.frag");

		//Enable depth testing
		glEnable(GL_DEPTH_TEST);

		float vertices[] = {
			// positions          // normals           // texture coords
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
		};

		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(cubeVAO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)

		glGenVertexArrays(1, &lightCubeVAO);
		glBindVertexArray(lightCubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// note that we update the lamp's position attribute's stride to reflect the updated buffer data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// load textures (we now use a utility function to keep the code more organized)
		// -----------------------------------------------------------------------------
		diffuseMap = LoadTexture("data/textures/container2.png");
		specularMap = LoadTexture("data/textures/container2_specular.png");

		// shader configuration
		// --------------------
		lightingShader.use();
		lightingShader.setInt("material.diffuse", 0);
		lightingShader.setInt("material.specular", 1);


		pointLightPositions[0] = glm::vec3(0.0f, -3.0f, -3.0f);
		pointLightPositions[1] = glm::vec3(0.0f, 3.0f, -3.0f);
		pointLightPositions[2] = glm::vec3(-3.0f, 0.0f, -3.0f);
		pointLightPositions[3] = glm::vec3(3.0f, 0.0f, -3.0f);
		pointLightPositions[4] = glm::vec3(0.0f, 0.0f, -3.0f);

	}

	void light_test::End()
	{
		glDeleteVertexArrays(1, &cubeVAO);
		glDeleteVertexArrays(1, &lightCubeVAO);
		glDeleteBuffers(1, &VBO);
		//glDeleteBuffers(1, &ebo_);
	}

	void light_test::Update(float dt)
	{
		//The action here is given to the blinn flag
		blinn = action;

		// be sure to activate shader when setting uniforms/drawing objects
		lightingShader.use();
		lightingShader.setVec3("viewPos", camera->Position);
		lightingShader.setFloat("material.shininess", 32.0f);
		//The blinn flag is given to the shader to swap between blinn-/phong lighting
		lightingShader.setInt("blinn", blinn);

		// directional light
		glm::vec3 dirLightPos{ -0.2f, -1.0f, -0.3f };
		glm::vec3 dirLightAmbient{ 0.05f, 0.05f, 0.05f };
		glm::vec3 dirLightDiffuse{ 0.4f, 0.4f, 0.4f };
		glm::vec3 dirLightSpecular{ 0.5f, 0.5f, 0.5f };

		SetLight(lightingShader, dirLightPos, dirLightAmbient, dirLightDiffuse, dirLightSpecular);

		glm::vec3 pointLightAmbient{ 0.05f, 0.05f, 0.05f };
		glm::vec3 pointLightDiffuse{ 0.8f, 0.8f, 0.8f };
		glm::vec3 pointLightSpecular{ 1.0f, 1.0f, 1.0f };
		float pointLightConstant = 2.0f;
		float pointLightLinear = 0.18f;
		float pointLightQuadratic = 0.064f;

		lightingShader.setInt("lightCount", lightCount);
		
		for (size_t i = 0; i < lightCount; i++)
		{
			SetLight(
				lightingShader, pointLightPositions[i],
				pointLightAmbient, pointLightDiffuse, pointLightSpecular,
				pointLightConstant, pointLightLinear, pointLightQuadratic);
		}

		//SpotLight
		SetLight(
			lightingShader, camera->Position, camera->Front,
			pointLightAmbient, pointLightDiffuse, pointLightSpecular,
			pointLightConstant, pointLightLinear, pointLightQuadratic,
			12.5f, 15.0f);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), 1280.0f / 720.0f, 0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		// bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		// render containers
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			lightingShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// also draw the lamp object(s)
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		// we now draw as many light bulbs as we have point lights.
		glBindVertexArray(lightCubeVAO);
		for (unsigned int i = 0; i < lightCount; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			lightCubeShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}
}
int main(int argc, char** argv)
{
	gpr5300::Camera camera;
	gpr5300::light_test scene;
	scene.camera = &camera;
	gpr5300::Engine engine(&scene);
	engine.Run();

	return EXIT_SUCCESS;
}