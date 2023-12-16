#include "skybox.h"
#include "resource_manager.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game {
	Skybox::Skybox(const std::string name, const Resource* geometry, const Resource* material, const Resource* texture) : 
		SceneNode(name, geometry, material, texture)
	{
        assert(texture->GetType() == ResourceType::SkyboxTexture);
    }

	void Skybox::Draw(Camera* camera)
	{

        // Enable z-buffer
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        

        // Select proper material (shader program)
        glUseProgram(material_);

        // draw skybox
        GLuint skyboxVAO, skyboxVBO;

        skyboxVAO = ResourceManager::GetSkyboxVAO();
        skyboxVBO = ResourceManager::GetSkyboxVBO();

        glDepthFunc(GL_LEQUAL);  
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        // Set globals for camera
        camera->SetupShaderSkybox(material_);
        // Set world matrix and other shader input variables
        SetupShader(material_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
	}

    void Skybox::SetupShader(GLuint program) {
        // Texture
        if (texture_) {
            GLint tex = glGetUniformLocation(program, "texture_map");
            glUniform1i(tex, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
        }

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    glm::mat4 Skybox::CalculateTransform(float current_time, bool base) const {
        glm::mat4 scaling = glm::scale(glm::mat4(1.0), scale_);
        glm::mat4 transf = scaling * glm::mat4();

        return transf;
    }
}