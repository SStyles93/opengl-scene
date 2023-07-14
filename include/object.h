#pragma once
#include <array>
#include <glm/glm.hpp>

#include "model.h"
#include "pipeline.h"

namespace gpr5300
{
	constexpr glm::vec3 X_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 Y_AXIS = glm::vec3(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 Z_AXIS = glm::vec3(0.0f, 1.0f, 1.0f);

	struct rotation
	{
		float angle;
		glm::vec3 axis;
	};

	class Object
	{
	public:

		Object() = default;
		/*Object(const Object& other);
		Object(const Object&& other) noexcept;*/
		//Object& operator=(const Object& other) {return *this = Object(other);}
		//Object& operator=(const Object&& other)noexcept

		Object(const Model& model, const Pipeline& pipeline);

		void Translate(glm::vec3 position = glm::vec3(0.0f));
		void Rotate(float angle = 0.0f, glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f));
		void Scale(glm::vec3 scale = glm::vec3(1.0f));

		//void SetPositionRotationScale(glm::vec3 position, std::array<glm::vec3, 2> rotation, glm::vec3 scale);
		void SetPositionRotationScale(
			glm::vec3 position = VEC3_ZERO, 
			float angle = 0.0f, 
			glm::vec3 axis = Y_AXIS, 
			glm::vec3 scale = glm::vec3(1.0f));

		void Draw();

		Model GetModel() { return m_model; }
		glm::mat4* GetModelMatrix() { return &m_modelMatrix; }

	private:

		Model m_model{};
		Pipeline m_pipeline{};
		glm::mat4 m_projectionMatrix{};
		glm::mat4 m_viewMatrix{};
		glm::mat4 m_modelMatrix = glm::mat4(1.0f);

		glm::vec3 m_position{};
		rotation m_rotation{};
		glm::vec3 m_scale{};

	};

	class ObjectManager
	{
	public:
		std::vector<Object> objects{};
		unsigned int buffer;
		unsigned int VAO;

		void ConfigureBuffer();
		void BindBuffer();
		void DrawObjects();
	};
}