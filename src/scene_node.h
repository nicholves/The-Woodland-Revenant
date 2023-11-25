#ifndef SCENE_NODE_H_
#define SCENE_NODE_H_

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

#include "resource.h"
#include "camera.h"

namespace game {

    // Class that manages one object in a scene 
    class SceneNode {

    public:
        // Create scene node from given resources
        SceneNode(const std::string name, const Resource* geometry, const Resource* material, const Resource* texture = NULL);

        // Destructor
        ~SceneNode();

        // Get name of node
        const std::string GetName(void) const;

        // Get node attributes
        glm::vec3 GetPosition(void) const;
        glm::quat GetOrientation(void) const;
        glm::vec3 GetScale(void) const;

        // Set node attributes
        void SetPosition(glm::vec3 position);
        void SetOrientation(glm::quat orientation);
        void SetScale(glm::vec3 scale);

        // Perform transformations on node
        void Translate(glm::vec3 trans);
        void Rotate(glm::quat rot);
        void Scale(glm::vec3 scale);

        // Draw the node according to scene parameters in 'camera'
        // variable
        virtual void Draw(Camera* camera);

        // Set blending mode
        void SetBlending(bool blending);

        // Update the node
        virtual void Update(void);

        // OpenGL variables
        GLenum GetMode(void) const;
        GLuint GetArrayBuffer(void) const;
        GLuint GetElementArrayBuffer(void) const;
        GLsizei GetSize(void) const;
        GLuint GetMaterial(void) const;
        glm::vec3 GetOrbitTranslation(void) const;
        glm::quat GetOrbitRotation(void) const;

        void SetParent(SceneNode*);
        void SetOrbitTranslation(const glm::vec3);
        void SetOrbitRotation(const glm::quat);
        void SetWindAffected(const bool);

        glm::mat4 CalculateTransform(float) const;

    private:
        std::string name_; // Name of the scene node
        GLuint array_buffer_; // References to geometry: vertex and array buffers
        GLuint element_array_buffer_;
        GLenum mode_; // Type of geometry
        GLsizei size_; // Number of primitives in geometry
        GLuint material_; // Reference to shader program
        GLuint texture_; // Reference to texture resource
        glm::vec3 position_; // Position of node
        glm::quat orientation_; // Orientation of node
        glm::vec3 scale_; // Scale of node
        bool blending_;

        SceneNode* parent_ = NULL;
        bool wind_affected; // Whether to make it move with the wind
        glm::vec3 orbit_translation = glm::vec3(0, 0, 0);
        glm::quat orbit_rotation = glm::quat(0, 0, 0, 0);
        float wind_strength = 0.05f; // The amount the wind moves the tree

        // Set matrices that transform the node in a shader program
        void SetupShader(GLuint program);

    }; // class SceneNode

} // namespace game

#endif // SCENE_NODE_H_
