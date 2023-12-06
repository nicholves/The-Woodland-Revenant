#ifndef SKYBOX_H
#define SKYBOX_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene_node.h"

namespace game {

	class Skybox : public SceneNode
	{
	public:
		Skybox(const std::string name, const Resource* geometry, const Resource* material, const Resource* texture = NULL);

		virtual void Draw(Camera* camera) override;
	private:
		virtual void SetupShader(GLuint program) override;

		virtual glm::mat4 Skybox::CalculateTransform(float current_time) const override;
	};


}


#endif // SKYBOX_H