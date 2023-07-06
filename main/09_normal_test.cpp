#include "engine.h"
#include "scene.h"
#include "pipeline.h"
#include "model.h"
#include "file_utility.h"
#include "settings.h"

namespace gpr5300
{
    class normal_test final : public Scene
    {
    public:
        void Begin() override;
        void Update(float dt) override;
        void End() override;

        void renderQuad();

        Pipeline pipeline;

        float time_;

        unsigned int diffuseMap;
        unsigned int normalMap;

        unsigned int quadVAO = 0;
        unsigned int quadVBO;

        glm::vec3 lightPos{ 0.0f, 0.0f, 3.0f };
        glm::mat4 model;
        glm::mat4 model2;
    };

    void normal_test::renderQuad()
    {
        if (quadVAO == 0)
        {
            // positions
            glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
            glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
            glm::vec3 pos3(1.0f, -1.0f, 0.0f);
            glm::vec3 pos4(1.0f, 1.0f, 0.0f);
            // texture coordinates
            glm::vec2 uv1(0.0f, 1.0f);
            glm::vec2 uv2(0.0f, 0.0f);
            glm::vec2 uv3(1.0f, 0.0f);
            glm::vec2 uv4(1.0f, 1.0f);
            // normal vector
            glm::vec3 nm(0.0f, 0.0f, 1.0f);

            // calculate tangent/bitangent vectors of both triangles
            glm::vec3 tangent1, bitangent1;
            glm::vec3 tangent2, bitangent2;
            // triangle 1
            // ----------
            glm::vec3 edge1 = pos2 - pos1;
            glm::vec3 edge2 = pos3 - pos1;
            glm::vec2 deltaUV1 = uv2 - uv1;
            glm::vec2 deltaUV2 = uv3 - uv1;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            // triangle 2
            // ----------
            edge1 = pos3 - pos1;
            edge2 = pos4 - pos1;
            deltaUV1 = uv3 - uv1;
            deltaUV2 = uv4 - uv1;

            f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


            bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


            float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
            };
            // configure plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void normal_test::Begin()
    {
        time_ = 0.0f;
        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        // build and compile shaders
        // -------------------------
        pipeline = Pipeline("data/shaders/normal/normal.vert", "data/shaders/normal/normal.frag");

        // load textures
		// -------------
        diffuseMap = LoadTexture("data/textures/brickwall.jpg");
        normalMap = LoadTexture("data/textures/brickwall_normal.jpg");

        // shader configuration
        // --------------------
        pipeline.use();
        pipeline.setInt("diffuseMap", 0);
        pipeline.setInt("normalMap", 1);

        model = glm::mat4(1.0f);
        model2 = glm::mat4(1.0f);
    }

    void normal_test::Update(float dt)
    {
        time_ += dt;
        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // configure view/projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera->GetViewMatrix();
        pipeline.use();
        pipeline.setMat4("projection", projection);
        pipeline.setMat4("view", view);
        // render normal-mapped quad
        //model = glm::rotate(model, glm::radians(time_ * 0.00000001f), glm::normalize(glm::vec3(0.0, 1.0, 0.0))); // rotate the quad to show normal mapping from multiple directions
        pipeline.setMat4("model", model);
        pipeline.setVec3("viewPos", camera->Position);
        pipeline.setVec3("lightPos", lightPos);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        renderQuad();

        // render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
        const float radiusFromCenter = 1.0f;
        lightPos = glm::vec3(radiusFromCenter * std::sin(time_), 0.0f, radiusFromCenter * std::cos(time_));
        model2 = glm::translate(model2, lightPos);
        model2 = glm::scale(model2, glm::vec3(0.1f));
        pipeline.setMat4("model", model2);
        renderQuad();
    }

    void normal_test::End()
    {
    }
}

int main(int argc, char** argv)
{
    gpr5300::Camera camera;
    gpr5300::normal_test scene;
    scene.camera = &camera;
    gpr5300::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}