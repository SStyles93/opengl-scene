#include "object.h"
namespace gpr5300
{
	//Object::Object(const Object& other): m_model(other.m_model), m_pipeline(other.m_pipeline){}
	//Object::Object(const Object&& other) noexcept
	//{
	//	std::exchange(other, nullptr);
	//}

	Object::Object(const Model& model, const Pipeline& pipeline) : m_model(model), m_pipeline(pipeline)
	{}

	void Object::Translate(glm::vec3 position)
	{
		m_position = position;

		m_modelMatrix = glm::mat4(1.0f);
		m_modelMatrix = glm::translate(m_modelMatrix, position);
	}
	void Object::Rotate(float angle, glm::vec3 axis )
	{
		m_rotation.angle = angle;
		m_rotation.axis = axis;

		m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(angle), axis);
	}
	void Object::Scale(glm::vec3 scale)
	{
		m_scale = scale;

		m_modelMatrix = glm::scale(m_modelMatrix, scale);
	}

	void Object::SetPositionRotationScale(glm::vec3 position, float angle, glm::vec3 axis, glm::vec3 scale)
	{
		Translate(position);
		Rotate(angle, axis);
		Scale(scale);
	}

	void Object::Draw()
	{
		m_pipeline.setMat4("model", m_modelMatrix);
		m_model.Draw(m_pipeline);
	}

	void ObjectManager::ConfigureBuffer()
	{
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, objects.size() * sizeof(glm::mat4), objects[0].GetModelMatrix(), GL_STATIC_DRAW);
	}

	void ObjectManager::BindBuffer()
	{
		for (unsigned int i = 0; i < objects[0].GetModel().meshes.size(); i++)
		{
			VAO = objects[0].GetModel().meshes[i].VAO;
			glBindVertexArray(VAO);
			// set attribute pointers for matrix (4 times vec4)
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);

			glBindVertexArray(0);
		}
	}

	void ObjectManager::DrawObjects()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, objects[0].GetModel().textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
		for (unsigned int i = 0; i < objects[0].GetModel().meshes.size(); i++)
		{
			glBindVertexArray(objects[0].GetModel().meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(objects[0].GetModel().meshes[i].indices.size()), GL_UNSIGNED_INT, 0, objects.size());
			glBindVertexArray(0);
		}
	}



}