#include "engine.h"
#include "scene.h"
#include "pipeline.h"
#include "model.h"
#include "settings.h"

namespace gpr5300
{
	class model_test final : public Scene
	{
		void Begin() override;
		void End() override;
		void Update(float dt) override;

		Pipeline modelPipeline_{};
		Model model_{};
	};

	void model_test::Begin()
	{
		//Enable depth testing
		glEnable(GL_DEPTH_TEST);

		//Pipeline init
		modelPipeline_ = Pipeline("data/shaders/hello_model/model.vert", "data/shaders/hello_model/model.frag");

		model_ = Model("data/objects/backpack.obj", false);
	};
	void model_test::End()
	{
		//glDeleteVertexArrays(1, &cubeVAO);
		//glDeleteVertexArrays(1, &lightCubeVAO);
		//glDeleteBuffers(1, &VBO);
		//glDeleteBuffers(1, &ebo_);
	};
	void model_test::Update(float dt)
	{
		// don't forget to enable shader before setting uniforms
		modelPipeline_.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), 1280.0f / 720.0f, 0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		modelPipeline_.setMat4("projection", projection);
		modelPipeline_.setMat4("view", view);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		modelPipeline_.setMat4("model", model);
		model_.Draw(modelPipeline_);
	}

}//namespace
int main(int argc, char** argv)
{
	gpr5300::model_test scene;
	gpr5300::Camera camera;
	scene.camera = &camera;
	gpr5300::Engine engine(&scene);
	engine.Run();
	return EXIT_SUCCESS;
}