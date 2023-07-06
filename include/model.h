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
#include <vector>

#include "mesh.h"
#include "pipeline.h"

namespace gpr5300
{
    //unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma);

    struct ModelMatrices
    {
        glm::mat4 model{};
        glm::mat4 normal{};
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
        void DrawInstances(Pipeline& pipeline, ModelMatrices* modelMatrices, const int count);
        void DrawInstances(Pipeline& pipeline, ModelMatrices* modelMatrices, const int count, const glm::mat4 projection, const glm::mat4 view);
        void DrawStaticInstances(Pipeline& pipeline, const int count);
        void DrawStaticInstances(Pipeline& pipeline, const int count, const glm::mat4 projection, const glm::mat4 view);

        void SetUpVBO(ModelMatrices* modelMatrices, const int count);

    private:

        void loadModel(const std::string& path);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        void processNode(aiNode* node, const aiScene* scene);

        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    };
}