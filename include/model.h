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

    class Model
    {
    public:
        std::vector<Texture> textures_loaded;
        std::vector<Mesh> meshes;
        bool gammaCorrection;
        std::string directory;

        Model();
        Model(const char* path, bool gamma);

        void Draw(Pipeline& pipeline);

    private:

        void loadModel(const std::string& path);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        void processNode(aiNode* node, const aiScene* scene);

        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    };
}