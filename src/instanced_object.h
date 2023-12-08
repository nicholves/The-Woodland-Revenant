#ifndef INSTANCED_SCENE_NODE_H
#define INSTANCED_SCENE_NODE_H

#include <vector>
#include "scene_node.h"
#include <glm/gtc/quaternion.hpp>
#include "renderable.h"

namespace game {
	class InstancedObject : public Renderable {
	public:
		InstancedObject(const std::string name, const Resource* geometry, const Resource* material, const std::vector<glm::vec3>& instancePositions, 
			const std::vector<glm::vec3>& instanceScales, const std::vector<glm::quat>& instanceOrientations, const Resource* texture = NULL);

		virtual void Update(void) override;

		virtual void Draw(Camera* camera) override;

	private:
		GLuint instanceVBO = 0;
        GLuint array_buffer_ = 0; // References to geometry: vertex and array buffers
        GLuint element_array_buffer_ = 0;
		GLuint VAO = 0;
        GLenum mode_; // Type of geometry
        GLsizei size_; // Number of primitives in geometry
        GLuint material_; // Reference to shader program
        GLuint texture_; // Reference to texture resource

		size_t instance_count_;

		void CalculateTransforms(glm::mat4* arr, const std::vector<glm::vec3>& instancePositions,
			const std::vector<glm::vec3>& instanceScales, const std::vector<glm::quat>& instanceOrientations) const;

		virtual void SetupShader(GLuint program) override;

		static glm::mat4 CalculateTransform(const glm::vec3& position_, const glm::vec3& scale_, const glm::quat& orientation_);
	};


}

#endif // INSTANCED_SCENE_NODE_H
