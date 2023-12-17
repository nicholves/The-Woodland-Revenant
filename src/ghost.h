#ifndef GHOST_H_
#define GHOST_H_

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "resource.h"
#include "scene_node.h"

namespace game {

    // Abstraction of an Ghost
    class Ghost : public SceneNode {

    public:
        // Create asteroid from given resources
        Ghost(const std::string name, const Resource* geometry, const Resource* material, const Resource* texture);

        // Destructor
        ~Ghost();

        //getters
        bool getContacted();

        //setters
        void setContacted(bool contact);

        // Update geometry configuration
        void Update(Camera* camera, float deltaTime);

    private:

        bool contacted = false; //boolean to see if in contact with player
        float speed = 55.0;

    }; // class Ghost

} // namespace game

#endif // GHOST_H_
