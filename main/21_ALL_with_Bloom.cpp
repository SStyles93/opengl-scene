#include <random>

#include "engine.h"
#include "scene.h"
#include "object.h"
#include "file_utility.h"
#include "settings.h"
#include "bloom.h"

#include "imgui.h"

namespace gpr5300
{
	struct Material
	{
		std::vector<unsigned int> textures;

		void setPipeline(Pipeline& pipeline)
		{
			pipeline.use();
			pipeline.setInt("texture_diffuse1", 0);
			pipeline.setInt("texture_normal1", 1);
			pipeline.setInt("texture_metallic1", 2);
			pipeline.setInt("texture_roughness1", 3);
			pipeline.setInt("texture_ao1", 4);

			for (int i = 0; i < textures.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, textures[i]);
			}
		}
	};

	class ALL_with_Bloom : public Scene
	{
	public:
		void Begin() override;
		void Update(float dt) override;
		void End() override;
		void DrawImGui() override;

		void renderImage();
		void renderEnvironmentCube();
		float ourLerp(float a, float b, float f);

		void renderCube();

		void SetUpPlane();

		void DrawScene(Pipeline& pipeline, std::vector<Model> models = {});

		std::vector<Pipeline> pipelines{};

		unsigned int gBuffer;
		unsigned int gPosition, gNormal, gBaseColor, gARM, gSSAO;
		unsigned int rboDepth;

		unsigned int quadVAO = 0;
		unsigned int quadVBO;
		unsigned int cubeVAO = 0;
		unsigned int cubeVBO = 0;

		//PointLights
		int NR_LIGHTS = 64;
		std::vector<glm::vec3> lightPositions;
		std::vector<glm::vec3> lightColors;
		float LightColorPower = 20.0f;
		float pointLightPos[3]{ 0.0f, 10.0f, 0.0f };
		float pointLightRadiusFromCenter = 10.0f;
		float pointLightColor[3]{ 1.0f , 1.0f, 1.0f };

		//LIGHT
		glm::vec3 DirectionalLightDirection{ -1.0f, -1.0f, -1.0f };
		float dirLightDir[3]{ -1.0f, -1.0f, -1.0f };
		glm::vec3 DirectionalLightColor{ 1.0f };
		float dirLightColor[3]{ 1.0f , 1.0f, 1.0f };
		float dirLightPower = 1.0f;

		Model backpack;
		std::vector<ModelMatrices> backpackMatrices;
		Model rock;
		std::vector<ModelMatrices> rockMatrices;
		Model poulpe;
		std::vector<ModelMatrices> poulpeMatrices;

		unsigned int rockAmount = 40;
		unsigned int backPackAmount = 10;

		float time_{};

		unsigned int ssaoFBO, ssaoBlurFBO;
		unsigned int noiseTexture;
		unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
		std::vector<glm::vec3> ssaoKernel;
		std::vector<glm::vec3> ssaoNoise;

		float planeVertices[48] = {
			// positions            // normals				// texcoords
			1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

			-1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
		};
		unsigned int planeVAO = 0, planeVBO = 0;

		//Floor
		Material wall;
		Material gold;

		ModelMatrices planeMatrice;

		//IBL
		unsigned int irradianceMap;
		unsigned int prefilterMap;
		unsigned int brdfLUTTexture;
		unsigned int envCubemap;
		unsigned int hdrTexture;
		unsigned int captureFBO;
		unsigned int captureRBO;

		//Shadows
		const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
		unsigned int depthMapFBO;
		unsigned int depthMap;
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 0.1f, far_plane = 200.0f;
		float shadowFrustrum = 10.0f;

		//BloomBlur
		unsigned int hdrBuffer;
		unsigned int hdrRBO;
		unsigned int colorBuffers[2];
		unsigned int pingpongFBO[2];
		unsigned int pingpongColorbuffers[2];

		BloomRenderer bloomRenderer;

		bool bloom = true;
		float exposure = 1.0f;
		float bloomFilterRadius = 0.005f;

		void RenderUpsamples(BloomRenderer& bloomRenderer, float filterRadius);
		void RenderDownsamples(BloomRenderer& bloomRenderer, unsigned int srcTexture);
		void RenderBloomTexture(BloomRenderer& bloomRenderer, unsigned int srcTexture, float filterRadius);

		//ImGui Params
		void SetPointLights();
		void RandomColour();
	};

	void ALL_with_Bloom::DrawImGui()
	{
		ImGui::Begin("Scene Settings");

		if (ImGui::CollapsingHeader("Directional Light Settings"))
		{
			ImGui::SliderFloat3("Directional Light Direction", dirLightDir, -1.0f, 1.0f);
			ImGui::ColorEdit3("Directional Light Color", dirLightColor);
			ImGui::SliderFloat("Directional Light Intensity", &dirLightPower, 0.0f, 100.0f);
		}

		if (ImGui::CollapsingHeader("Point Light Settings"))
		{
			ImGui::SliderFloat3("Point Lights Position", pointLightPos, 0, 50);
			ImGui::ColorEdit3("Point Light Color", pointLightColor);
			if (ImGui::Button("Random Colour"))
			{
				RandomColour();
			}
			ImGui::SliderFloat("Point Lights radius", &pointLightRadiusFromCenter, 0, 50);
			ImGui::SliderFloat("Point Light Intensity", &LightColorPower, 0.0f, 100.0f);
			ImGui::SliderInt("Number of Point Lights", &NR_LIGHTS, 0, 128);
			if (ImGui::Button("Set Point Lights"))
			{
				SetPointLights();
			}
		}

		ImGui::End();
	}

	void ALL_with_Bloom::SetPointLights()
	{
		lightPositions.resize(NR_LIGHTS);
		lightColors.resize(NR_LIGHTS);
		// lighting info
		//-------------
		srand(static_cast<unsigned int>(time_));
		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{

			float radiusFromCenter = pointLightRadiusFromCenter;
			float xPos = pointLightPos[0] + radiusFromCenter * std::sin(i);
			float yPos = pointLightPos[1];
			float zPos = pointLightPos[2] + radiusFromCenter * std::cos(i);
			lightPositions[i] = glm::vec3(xPos, yPos, zPos);

			// also calculate random color
			float rColor = pointLightColor[0];
			float gColor = pointLightColor[1];
			float bColor = pointLightColor[2];
			lightColors[i] = glm::vec3(rColor, gColor, bColor);
		}
	}
	void ALL_with_Bloom::RandomColour()
	{
		srand(static_cast<unsigned int>(time_));
		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{
			// also calculate random color
			float rColor = ((rand() % 10) / 10.0f); 
			float gColor = ((rand() % 10)/ 10.0f); 
			float bColor = ((rand() % 10) / 10.0f);
			lightColors[i] = glm::vec3(rColor, gColor, bColor);
		}
	}

	void ALL_with_Bloom::RenderDownsamples(BloomRenderer& bloomRenderer, unsigned int srcTexture)
	{
		const std::vector<bloomMip>& mipChain = bloomRenderer.mFBO.MipChain();

		bloomRenderer.mDownsampleShader->use();
		bloomRenderer.mDownsampleShader->setVec2("srcResolution", bloomRenderer.mSrcViewportSizeFloat);
		if (bloomRenderer.mKarisAverageOnDownsample)
		{
			bloomRenderer.mDownsampleShader->setInt("mipLevel", 0);
		}

		// Bind srcTexture (HDR color buffer) as initial texture input
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, srcTexture);

		// Progressively downsample through the mip chain
		for (int i = 0; i < (int)mipChain.size(); i++)
		{
			const bloomMip& mip = mipChain[i];
			glViewport(0, 0, mip.size.x, mip.size.y);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, mip.texture, 0);

			// Render screen-filled quad of resolution of current mip
			renderImage();

			// Set current mip resolution as srcResolution for next iteration
			bloomRenderer.mDownsampleShader->setVec2("srcResolution", mip.size);
			// Set current mip as texture input for next iteration
			glBindTexture(GL_TEXTURE_2D, mip.texture);
			// Disable Karis average for consequent downsamples
			if (i == 0) { bloomRenderer.mDownsampleShader->setInt("mipLevel", 1); }
		}

		glUseProgram(0);
	}
	void ALL_with_Bloom::RenderUpsamples(BloomRenderer& bloomRenderer, float filterRadius)
	{
		const std::vector<bloomMip>& mipChain = bloomRenderer.mFBO.MipChain();

		bloomRenderer.mUpsampleShader->use();
		bloomRenderer.mUpsampleShader->setFloat("filterRadius", filterRadius);

		// Enable additive blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		for (int i = (int)mipChain.size() - 1; i > 0; i--)
		{
			const bloomMip& mip = mipChain[i];
			const bloomMip& nextMip = mipChain[i - 1];

			// Bind viewport and texture from where to read
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mip.texture);

			// Set framebuffer render target (we write to this texture)
			glViewport(0, 0, nextMip.size.x, nextMip.size.y);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, nextMip.texture, 0);

			// Render screen-filled quad of resolution of current mip
			renderImage();
		}

		// Disable additive blending
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);

		glUseProgram(0);
	}
	void ALL_with_Bloom::RenderBloomTexture(BloomRenderer& bloomRenderer, unsigned int srcTexture, float filterRadius)
	{
		bloomRenderer.mFBO.BindForWriting();

		RenderDownsamples(bloomRenderer, srcTexture);
		RenderUpsamples(bloomRenderer, filterRadius);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Restore viewport
		glViewport(0, 0, bloomRenderer.mSrcViewportSize.x, bloomRenderer.mSrcViewportSize.y);
	}

	void ALL_with_Bloom::SetUpPlane()
	{
		glGenVertexArrays(1, &planeVAO);
		glGenBuffers(1, &planeVBO);

		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		glBindVertexArray(planeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

		glGenBuffers(1, &planeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ModelMatrices), &planeMatrice, GL_STATIC_DRAW);
		//set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices), nullptr);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices),
			reinterpret_cast<void*>(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices),
			reinterpret_cast<void*>(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices),
			reinterpret_cast<void*>(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices),
			reinterpret_cast<void*>(sizeof(glm::mat4)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices),
			reinterpret_cast<void*>(sizeof(glm::mat4) + sizeof(glm::vec4)));
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices),
			reinterpret_cast<void*>(sizeof(glm::mat4) + 2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(10);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(ModelMatrices),
			reinterpret_cast<void*>(sizeof(glm::mat4) + 3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);
		glVertexAttribDivisor(10, 1);

		glBindVertexArray(0);
	}
	void ALL_with_Bloom::renderCube()
	{
		// initialize (if necessary)
		if (cubeVAO == 0)
		{
			float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
				1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
				1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
				1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
				1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
				-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
				-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
				// right face
				1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
				1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
				1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
				1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
				1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
				1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
				// bottom face
				-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
				1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
				1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
				1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
				-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
				// top face
				-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
				1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
				1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
				1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
				-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
				-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        
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
	void ALL_with_Bloom::renderImage()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
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
	void ALL_with_Bloom::renderEnvironmentCube()
	{
		// initialize (if necessary)
		if (cubeVAO == 0)
		{
			float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
				1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right      
				// front face
				-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
				1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
				1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
				-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
				1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
				// left face
				-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
				-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
				-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
				-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
				// right face
				1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
				1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
				1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
				1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
				1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
				1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
				// bottom face
				1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
				1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
				1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
				-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
				// top face
				-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
				1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
				-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom-left        
				1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
				-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
				1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f // top-right     
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
	float ALL_with_Bloom::ourLerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void ALL_with_Bloom::Begin()
	{
#pragma region OpenGL Settings

		// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
		stbi_set_flip_vertically_on_load(true);

		// configure global opengl state
		// -----------------------------
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		//IBL
		// set depth function to less than AND equal for skybox depth trick.
		glDepthFunc(GL_LEQUAL);
		// enable seamless cubemap sampling for lower mip levels in the pre-filter map.
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#pragma endregion

		backpack = Model("data/objects/backpack/backpack.obj");
		rock = Model("data/objects/rock/rock.obj");
		poulpe = Model("data/objects/poulpe/PoulpeSam.obj");

		//Wall material
		wall.textures.emplace_back(LoadTexture("data/textures/pbr/wall/albedo.png"));
		wall.textures.emplace_back(LoadTexture("data/textures/pbr/wall/normal.png"));
		wall.textures.emplace_back(LoadTexture("data/textures/pbr/wall/metallic.png"));
		wall.textures.emplace_back(LoadTexture("data/textures/pbr/wall/roughness.png"));
		wall.textures.emplace_back(LoadTexture("data/textures/pbr/wall/ao.png"));
		//Gold
		gold.textures.emplace_back(LoadTexture("data/textures/pbr/gold/albedo.png"));
		gold.textures.emplace_back(LoadTexture("data/textures/pbr/gold/normal.png"));
		gold.textures.emplace_back(LoadTexture("data/textures/pbr/gold/metallic.png"));
		gold.textures.emplace_back(LoadTexture("data/textures/pbr/gold/roughness.png"));
		gold.textures.emplace_back(LoadTexture("data/textures/pbr/gold/ao.png"));


#pragma region Shader Loading

		// build and compile shaders

		//Geometry pass 0
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/geom_pass.vert",
			"data/shaders/ALL_with_Bloom/geom_pass.frag");
		//Lighting pass 1
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/light_pass.vert",
			"data/shaders/ALL_with_Bloom/light_pass.frag");
		//Light Boxes 2
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/simple_box.vert",
			"data/shaders/ALL_with_Bloom/simple_box.frag");
		//SSAO 3
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/ssao.vert",
			"data/shaders/ALL_with_Bloom/ssao.frag");
		//SSAO 4
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/ssao.vert",
			"data/shaders/ALL_with_Bloom/ssao_blur.frag");

		//IBL

		//equirectangularToCubemapShader 5
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/cubemap.vert",
			"data/shaders/ALL_with_Bloom/equirectangular_to_cubemap.frag");
		//irradianceShader 6
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/cubemap.vert",
			"data/shaders/ALL_with_Bloom/irradiance_convolution.frag");
		//prefilterShader 7
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/cubemap.vert",
			"data/shaders/ALL_with_Bloom/prefilter.frag");
		//brdfShader 8
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/brdf.vert",
			"data/shaders/ALL_with_Bloom/brdf.frag");
		//backgroundShader 9
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/background.vert",
			"data/shaders/ALL_with_Bloom/background.frag");

		//Shadow 10
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/shadow_map.vert",
			"data/shaders/ALL_with_Bloom/shadow_map.frag");

		//Bloom_final 11
		pipelines.emplace_back(
			"data/shaders/ALL_with_Bloom/bloom_final.vert",
			"data/shaders/ALL_with_Bloom/bloom_final.frag");

#pragma endregion

#pragma region ModelMatrices setting

		backpackMatrices.resize(backPackAmount);
		rockMatrices.resize(rockAmount);

		srand(13);
		for (unsigned int i = 0; i < backPackAmount; i++)
		{
			float radiusFromCenter = 10.0f;

			int maxRotation = 360;
			int minRotation = 0;
			int rotation = (rand() % (maxRotation - minRotation + 1) + maxRotation);

			backpackMatrices[i].SetObject(
				glm::vec3(radiusFromCenter * std::sin(i), 1.5f, radiusFromCenter * std::cos(i)),
				VEC3_UP, rotation);
		}

		for (unsigned int i = 0; i < rockAmount; i++)
		{
			int maxRotation = 360;
			int minRotation = 0;
			int rotation = (rand() % (maxRotation - minRotation + 1) + maxRotation);

			rockMatrices[i].SetObject(
				glm::vec3(static_cast<float>(((rand() % 100) - 50)), 0.0f, static_cast<float>(((rand() % 100) - 50))),
				VEC3_UP, rotation);
		}

		poulpeMatrices.resize(1);
		poulpeMatrices[0].SetObject(glm::vec3(0.0f, 8.5f, 0.0f));
		planeMatrice.SetObject(VEC3_ZERO, VEC3_UP, 0.0f, glm::vec3(50.0f, 0.1f, 50.0f));

#pragma endregion

#pragma region Objects setting

		backpack.SetUpVBO(backpackMatrices);
		rock.SetUpVBO(rockMatrices);
		poulpe.SetUpVBO(poulpeMatrices);

		SetUpPlane();

#pragma endregion

#pragma region ShadowMap

		// configure depth map FBO
		// -----------------------
		glGenFramebuffers(1, &depthMapFBO);
		// create depth texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
			NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

#pragma region gBuffer setting

		//Geom pass
		//------------------------
		pipelines[0].use();
		pipelines[0].setInt("texture_diffuse1", 0);
		pipelines[0].setInt("texture_normal1", 1);
		pipelines[0].setInt("texture_metallic1", 2);
		pipelines[0].setInt("texture_roughness1", 3);
		pipelines[0].setInt("texture_ao1", 4);

		// Lighting pass
		// -------------
		pipelines[1].use();
		pipelines[1].setInt("gPosition", 0);
		pipelines[1].setInt("gBaseColor", 1);
		pipelines[1].setInt("gNormal", 2);
		pipelines[1].setInt("gARM", 3);
		pipelines[1].setInt("gSSAO", 4);

		//IBL
		pipelines[1].setInt("irradianceMap", 5);
		pipelines[1].setInt("prefilterMap", 6);
		pipelines[1].setInt("brdfLUT", 7);

		//Shadow
		pipelines[1].setInt("shadowMap", 8);

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
		// color buffer
		glGenTextures(1, &gBaseColor);
		glBindTexture(GL_TEXTURE_2D, gBaseColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBaseColor, 0);
		// normal color buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gNormal, 0);
		//ARM
		glGenTextures(1, &gARM);
		glBindTexture(GL_TEXTURE_2D, gARM);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gARM, 0);
		//SSAO
		glGenTextures(1, &gSSAO);
		glBindTexture(GL_TEXTURE_2D, gSSAO);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gSSAO, 0);

		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[5] = {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 , /*GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 */ };

		glDrawBuffers(5, attachments);
		// create and attach depth buffer (renderbuffer)
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

#pragma region SSAO setting

		//SSAO
		pipelines[3].use();
		pipelines[3].setInt("gPosition", 0);
		pipelines[3].setInt("gNormal", 1);
		pipelines[3].setInt("texNoise", 2);
		pipelines[3].setInt("gSSAO", 3);
		pipelines[4].use();
		pipelines[4].setInt("ssaoInput", 0);

		// also create framebuffer to hold SSAO processing stage 
		// -----------------------------------------------------
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		// SSAO color buffer
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Framebuffer not complete!" << std::endl;
		// and blur stage
		glGenFramebuffers(1, &ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glGenTextures(1, &ssaoColorBufferBlur);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// generate sample kernel
		// ----------------------
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / 64.0f;

			// scale samples s.t. they're more aligned to center of kernel
			scale = ourLerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}

		// generate noise texture
		// ----------------------
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
			// rotate around z-axis (in tangent space)
			ssaoNoise.push_back(noise);
		}
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


#pragma endregion

#pragma region IBL setting

		// pbr: setup framebuffer
		// ----------------------
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 2048, 2048);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		// pbr: load the HDR environment map
		// ---------------------------------
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float* data = stbi_loadf(environmentPath, &width, &height, &nrComponents, 0);
		if (data)
		{
			glGenTextures(1, &hdrTexture);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
			// note how we specify the texture's data value to be float

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load HDR image." << std::endl;
		}

		// pbr: setup cubemap to render to and attach to framebuffer
		// ---------------------------------------------------------
		glGenTextures(1, &envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// enable pre-filter mipmap sampling (combatting visible dots artifact)
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
		// ----------------------------------------------------------------------------------------------
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		// pbr: convert HDR equirectangular environment map to cubemap equivalent
		// ----------------------------------------------------------------------
		pipelines[5].use();
		pipelines[5].setInt("equirectangularMap", 0);
		pipelines[5].setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glViewport(0, 0, 2048, 2048); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			pipelines[5].setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap,
				0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderEnvironmentCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
		// --------------------------------------------------------------------------------
		glGenTextures(1, &irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
		// -----------------------------------------------------------------------------
		pipelines[6].use();
		pipelines[6].setInt("environmentMap", 0);
		pipelines[6].setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			pipelines[6].setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderEnvironmentCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
		// --------------------------------------------------------------------------------
		glGenTextures(1, &prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// be sure to set minification filter to mip_linear 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
		// ----------------------------------------------------------------------------------------------------
		pipelines[7].use();
		pipelines[7].setInt("environmentMap", 0);
		pipelines[7].setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
			unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			pipelines[7].setFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				pipelines[7].setMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					prefilterMap, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				renderEnvironmentCube();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// pbr: generate a 2D LUT from the BRDF equations used.
		// ----------------------------------------------------
		glGenTextures(1, &brdfLUTTexture);
		// pre-allocate enough memory for the LUT texture.
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 2048, 2048, 0, GL_RG, GL_FLOAT, 0);
		// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 2048, 2048);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		glViewport(0, 0, 2048, 2048);
		pipelines[8].use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderImage();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// initialize static shader uniforms before rendering
		// --------------------------------------------------
		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
			0.1f, 100.0f);
		pipelines[1].use();
		pipelines[1].setMat4("projection", projection);

		pipelines[9].use();
		pipelines[9].setMat4("projection", projection);

		//Reset viewport
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

#pragma endregion

#pragma region LightBoxes setting

		SetPointLights();

#pragma endregion

#pragma region BloomBlur

		//Bloom Blur
		// create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
		glGenFramebuffers(1, &hdrBuffer);
		glGenRenderbuffers(1, &hdrRBO);
		glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCREEN_WIDTH, SCREEN_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRBO);

		glGenTextures(2, colorBuffers);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach texture to framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
		}
		unsigned int bloomAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, bloomAttachments);
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//FinalBlur
		pipelines[11].use();
		pipelines[11].setInt("scene", 0);
		pipelines[11].setInt("bloomBlur", 1);

		// bloom renderer
		// --------------
		bloomRenderer.Init(SCREEN_WIDTH, SCREEN_HEIGHT);

#pragma endregion

	}

	void ALL_with_Bloom::Update(float dt)
	{
		time_ += dt;

#pragma region Shadows

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		//glCullFace(GL_FRONT);
		//lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene

		float ShadowFrustrum = shadowFrustrum;
		lightProjection = glm::ortho(
			-10.0f * ShadowFrustrum,
			10.0f * ShadowFrustrum,
			-10.0f * ShadowFrustrum,
			10.0f * ShadowFrustrum,
			near_plane, far_plane);
		glm::vec3 target{ 0.0f, 0.0f, 0.0f };

		DirectionalLightDirection.x = dirLightDir[0];
		DirectionalLightDirection.y = dirLightDir[1];
		DirectionalLightDirection.z = dirLightDir[2];

		lightView = glm::lookAt(target - DirectionalLightDirection, target, glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		// render scene from light's point of view
		pipelines[10].use();
		pipelines[10].setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		poulpe.DrawShadow(poulpeMatrices);
		rock.DrawShadow(rockMatrices);
		backpack.DrawShadow(backpackMatrices);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// reset viewport
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);

#pragma endregion

#pragma region Geometry Pass

		//// render
		//// ------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. geometry pass: render scene's geometry/color data into gbuffer
		// -----------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
			0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();

		pipelines[0].use();
		pipelines[0].setMat4("projection", projection);
		pipelines[0].setMat4("view", view);

		DrawScene(pipelines[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

#pragma region SSAO

		// 2. generate SSAO texture
		// ------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		pipelines[3].use();
		// Send kernel + rotation 
		for (unsigned int i = 0; i < 64; ++i)
		{
			pipelines[3].setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
		}
		pipelines[3].setMat4("projection", projection);
		pipelines[3].setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gSSAO);
		renderImage();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. blur SSAO texture to remove noise
		// ------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		pipelines[4].use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		renderImage();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

#pragma region LightPass

		glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);

		// 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
		// -----------------------------------------------------------------------------------------------------------------------
		pipelines[1].use();
		pipelines[1].setVec3("camPos", camera->Position);
		pipelines[1].setMat4("view", view);
		pipelines[1].setMat4("projection", projection);
		pipelines[1].setInt("NR_LIGHTS", NR_LIGHTS);
		// set light uniforms
		pipelines[1].setMat4("lightSpaceMatrix", lightSpaceMatrix);

		//ImGui Parameters
		DirectionalLightDirection.x = dirLightDir[0];
		DirectionalLightDirection.y = dirLightDir[1];
		DirectionalLightDirection.z = dirLightDir[2];
		DirectionalLightColor.x = dirLightColor[0] * dirLightPower;
		DirectionalLightColor.y = dirLightColor[1] * dirLightPower;
		DirectionalLightColor.z = dirLightColor[2] * dirLightPower;

		pipelines[1].setVec3("directionnalLightDir", DirectionalLightDirection);
		pipelines[1].setVec3("directionnalLightColor", DirectionalLightColor);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBaseColor);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gARM);

		//SSAO
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

		// bind pre-computed IBL data
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

		//Shadow
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		// send light relevant uniforms
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{

			//ImGui Parameters
			float radiusFromCenter = pointLightRadiusFromCenter;
			float xPos = pointLightPos[0] + radiusFromCenter * std::sin(i);
			float yPos = pointLightPos[1];
			float zPos = pointLightPos[2] + radiusFromCenter * std::cos(i);
			lightPositions[i] = glm::vec3(xPos, yPos, zPos);

			lightColors[i].x = pointLightColor[0]; 
			lightColors[i].y = pointLightColor[1]; 
			lightColors[i].z = pointLightColor[2]; 
			glm::vec3 lightColor = lightColors[i];
			lightColor *= LightColorPower;

			//glm::vec3 lightPosView = glm::vec3(camera->GetViewMatrix() * glm::vec4(lightPositions[i], 1.0));
			glm::vec3 lightPosView = lightPositions[i];
			pipelines[1].use();
			pipelines[1].setVec3("lights[" + std::to_string(i) + "].Position", lightPosView);
			pipelines[1].setVec3("lights[" + std::to_string(i) + "].Color", lightColor);
			// update attenuation parameters and calculate radius
			const float constant = 1.0f;
			// note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = 0.7f;
			const float quadratic = 1.8f;
			pipelines[1].setFloat("lights[" + std::to_string(i) + "].Linear", linear);
			pipelines[1].setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
			// then calculate radius of light volume/sphere
			const float maxBrightness = std::fmaxf(std::fmaxf(lightColor.r, lightColor.g), lightColor.b);
			float radius = (-linear + std::sqrt(
				linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
			pipelines[1].setFloat("lights[" + std::to_string(i) + "].Radius", radius);
		}
		// finally render quad
		renderImage();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

#pragma region Light Boxes

		// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ----------------------------------------------------------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrBuffer); // write to default framebuffer
		// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
		// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
		// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
		glBlitFramebuffer(0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);
		////glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 3. render lights on top of scene
		// --------------------------------
		glm::mat4 model = glm::mat4(1.0f);
		pipelines[2].use();
		pipelines[2].setMat4("projection", projection);
		pipelines[2].setMat4("view", view);
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{

			//ImGui Parameters
			float radiusFromCenter = pointLightRadiusFromCenter;
			float xPos = pointLightPos[0] + radiusFromCenter * std::sin(i);
			float yPos = pointLightPos[1];
			float zPos = pointLightPos[2] + radiusFromCenter * std::cos(i);
			lightPositions[i] = glm::vec3(xPos, yPos, zPos);

			lightColors[i].x = pointLightColor[0];
			lightColors[i].y = pointLightColor[1];
			lightColors[i].z = pointLightColor[2];
			glm::vec3 lightColor = lightColors[i];
			lightColor *= LightColorPower;

			model = glm::mat4(1.0f);
			model = glm::translate(model, lightPositions[i]);
			model = glm::scale(model, glm::vec3(0.125f));
			pipelines[2].setMat4("model", model);
			pipelines[2].setVec3("lightColor", lightColor);
			renderCube();
		}
#pragma endregion

#pragma region Environment CubeMap

		// render skybox (render as last to prevent overdraw)
		pipelines[9].use();
		pipelines[9].setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		renderEnvironmentCube();

#pragma endregion

#pragma region BloomBlur

		RenderBloomTexture(bloomRenderer, colorBuffers[1], bloomFilterRadius);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
		// --------------------------------------------------------------------------------------------------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		pipelines[11].use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bloomRenderer.BloomTexture());
		pipelines[11].setFloat("exposure", exposure);
		renderImage();

#pragma endregion

		assert(glGetError() == 0);

	}

	void ALL_with_Bloom::End()
	{
		glDeleteBuffers(1, &gBuffer);

		glDeleteVertexArrays(1, &cubeVAO);
		glDeleteBuffers(1, &cubeVBO);
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteBuffers(1, &ssaoFBO);
		glDeleteBuffers(1, &ssaoBlurFBO);
		glDeleteBuffers(1, &ssaoColorBuffer);
		glDeleteBuffers(1, &ssaoColorBufferBlur);

		glDeleteVertexArrays(1, &planeVAO);
		glDeleteBuffers(1, &planeVAO);

		bloomRenderer.Destroy();
	}

	void ALL_with_Bloom::DrawScene(Pipeline& pipeline, std::vector<Model> models)
	{

		wall.setPipeline(pipeline);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		rock.DrawStaticInstances(pipeline, rockAmount);

		gold.setPipeline(pipeline);
		//Draw objects
		poulpe.DrawStaticInstances(pipeline, 1);
		backpack.DrawStaticInstances(pipeline, backPackAmount);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

int main(int argc, char** argv)
{
	gpr5300::Camera camera;
	gpr5300::ALL_with_Bloom scene;
	scene.camera = &camera;
	gpr5300::Engine engine(&scene);
	engine.Run();

	return EXIT_SUCCESS;
}
