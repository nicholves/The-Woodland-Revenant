#include "entities.h"

namespace game {

    Entity::Entity(const glm::vec3& min, const glm::vec3& max, const glm::vec3& pos) : position_(pos) {
        boundingBox = BoundingBox{min, max};
    }

    Entity::Entity(float x, float y, float z, const glm::vec3& pos) : position_(pos) {
        setBBoxValues(x, y, z);
        updateBoundingBox();
    }


    Entity::~Entity() {
    }


    // Function to update the Entity's bounding box
    void Entity::updateBoundingBox() {
        // Set the bounding box around the camera position
        boundingBox.min = position_ - glm::vec3(boxX, boxY, boxZ);  // Assuming a unit cube
        boundingBox.max = position_ + glm::vec3(boxX, boxY, boxZ);
    }


    bool Entity::checkPlayerCollision(Camera* camera) const {
        if (camera->getBBox().max.x < boundingBox.min.x || camera->getBBox().min.x > boundingBox.max.x) return false;
        if (camera->getBBox().max.y < boundingBox.min.y || camera->getBBox().min.y > boundingBox.max.y) return false;
        if (camera->getBBox().max.z < boundingBox.min.z || camera->getBBox().min.z > boundingBox.max.z) return false;
        
        return true;  // Collision detected
    }

    void Entity::setPos(const glm::vec3& pos) {
        position_ = pos;
    }


    void Entity::Update(void) {
        //update bounding box
        updateBoundingBox();
    }


    void Entity::setBBox(glm::vec3 min, glm::vec3 max) {
        boundingBox.min = min;
        boundingBox.max = max;
    }

    
    void Entity::setBBoxValues(float x, float y, float z) {
        boxX = x;
        boxY = y;
        boxZ = z;
    }

} // namespace game
