#include "ghost.h"

namespace game {

    Ghost::Ghost(const std::string name, const Resource* geometry, const Resource* material, const Resource* texture) : SceneNode(name, geometry, material, texture) {

    }


    Ghost::~Ghost() {
    }

    bool Ghost::getContacted() {
        return contacted;
    }

    void Ghost::setContacted(bool contact) {
        this->contacted = contact;
    }

    void Ghost::Update(Camera* camera, float deltaTime) {
        // Calculate direction from ghost to player camera
        glm::vec3 toPlayer = camera->GetPosition() - GetPosition();
        toPlayer.y = 0.0f; // Assuming the ghost should move only on the XZ plane

        // Calculate rotation to face the player's camera

        glm::mat4 viewMatrix = glm::lookAt(GetPosition(), camera->GetPosition(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat rotation = glm::quat_cast(glm::inverse(viewMatrix));
        rotation.x = 0.0f;
        rotation.z = 0.0f;

        // Set the rotation of the ghost
        SetOrientation(rotation);


        // Move towards the player's camera at a set rate
        glm::vec3 movement = glm::normalize(toPlayer) * speed * deltaTime;

        // Set the new position of the ghost
        SetPosition(GetPosition() + movement);

        //check for player contact.    Change values to determine how large the range is for contact
        if (toPlayer.x <= 15.0f && toPlayer.x >= -15.0f && toPlayer.z <= 15.0f && toPlayer.z >= -15.0f && contacted == false) {

            //player is not immune
            if (camera->getImmune() == false) {
                contacted = true;
            }
        }
        else {
            contacted = false;
        }
    }

} // namespace game
