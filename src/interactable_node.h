#ifndef INTERACTABLE_NODE_
#define INTERACTABLE_NODE_

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

#include "resource.h"
#include "scene_node.h"

namespace game {

    // A node that the player can interact with
    class InteractableNode : public SceneNode {

        public:
            // Create asteroid from given resources
            InteractableNode(const std::string name, const Resource *geometry, const Resource *material, const Resource* texture = NULL);

            // Destructor
            ~InteractableNode();

            // Sets all the information related to the positioning of the node
            void SetPositioning(glm::vec3 held_pos, glm::vec3 held_scale, glm::quat held_orientation, glm::vec3 world_scale, glm::quat world_orientation);

            // Get positioning information
            glm::vec3 GetHeldPos();
            glm::vec3 GetHeldScale();
            glm::quat GetHeldOrientation();

            glm::vec3 GetWorldScale();
            glm::quat GetWorldOrientation();

            // Sets/removes/gets the particles of the interactable
            void SetParticles(SceneNode*);
            SceneNode* GetParticles();

            // Update geometry configuration
            void Update(void);
            
        private:
            
            // How the node appears when being held by the player
            glm::vec3 held_pos_;
            glm::vec3 held_scale_;
            glm::quat held_orientation_;

            // How the node appears in the world (position unecessary as it gets placed relative to the camera when dropped)
            glm::vec3 world_scale_;
            glm::quat world_orientation_;

            // Particles when not held
            SceneNode* particles_ = NULL;
    };

} // namespace game

#endif // INTERACTABLE_NODE_
