#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <span>
#include <vector>

#include "mesh.h"
#include "pipeline.h"
#include "settings.h"

namespace gpr5300
{
    //unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma);

    struct ModelMatrices
    {
        glm::mat4 model{};
        glm::mat4 normal{};

        void SetObject(
            glm::vec3 position = VEC3_ZERO, 
            glm::vec3 rotationAxis = VEC3_UP, 
            float angle = 0.0f,
            glm::vec3 scale = VEC3_ONE);
    };

    class Model
    {
    public:
        std::vector<Texture> textures_loaded;
        std::vector<Mesh> meshes;
        bool gammaCorrection;
        std::string directory;
        unsigned int VBO;

        Model();
        Model(const char* path, bool gamma = false);

        void Draw(Pipeline& pipeline);
        void DrawInstances(Pipeline& pipeline, std::span<const ModelMatrices> modelMatrices);
        void DrawInstances(Pipeline& pipeline, std::span <ModelMatrices> modelMatrices, const int count, const glm::mat4 projection, const glm::mat4 view);
        void DrawStaticInstances(Pipeline& pipeline, const int count);
        void DrawStaticInstances(Pipeline& pipeline, const int count, const glm::mat4 projection, const glm::mat4 view);

        void DrawShadow(std::span<const ModelMatrices> modelMatrices) const;

        void SetUpVBO(std::span<ModelMatrices> modelMatrices);

    private:

        void loadModel(const std::string& path);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        void processNode(aiNode* node, const aiScene* scene);

        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    };
}
