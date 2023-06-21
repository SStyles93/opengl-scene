#include "scene.h"


namespace gpr5300
{
	unsigned int Scene::LoadTexture(std::string_view filePath, bool gammaCorrection)
	{
		unsigned int textureID;

		//Generate texture ID
		glGenTextures(1, &textureID);
		//Load Texture
		int width, height, nrComponents;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filePath.data(), &width, &height, &nrComponents, 0);
		//Load texture
		if (data)
		{
			GLenum internalFormat;
			GLenum dataFormat;
			if (nrComponents == 1)
			{
				internalFormat = dataFormat = GL_RED;
			}
			else if (nrComponents == 3)
			{
				internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
				dataFormat = GL_RGB;
			}
			else if (nrComponents == 4)
			{
				internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
				dataFormat = GL_RGBA;
			}

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
			stbi_image_free(data);
		}
		return textureID;
	}
}