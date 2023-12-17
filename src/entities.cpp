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

    void Entity::setOrientation(glm::quat orientation) {
        orientation_ = orientation;
        updateBoundingBox();
    }

    // Function to update the Entity's bounding box
    void Entity::updateBoundingBox() {
        // Set the bounding box around the camera position
        glm::mat4 rotate = glm::mat4_cast(orientation_);
        boundingBox.min = position_ - glm::vec3(glm::vec4(glm::vec3(boxX, boxY, boxZ), 1.0f) * rotate);  // Assuming a unit cube
        boundingBox.max = position_ + glm::vec3(glm::vec4(glm::vec3(boxX, boxY, boxZ), 1.0f) * rotate);
    }


    bool Entity::checkPlayerCollision(Camera* camera) const {
        float minx = std::min(boundingBox.min.x, boundingBox.max.x);
        float minz = std::min(boundingBox.min.z, boundingBox.max.z);
        float miny = std::min(boundingBox.min.y, boundingBox.max.y);

        float maxx = std::max(boundingBox.min.x, boundingBox.max.x);
        float maxz = std::max(boundingBox.min.z, boundingBox.max.z);
        float maxy = std::max(boundingBox.min.y, boundingBox.max.y);

        if (camera->getBBox().max.x < minx || camera->getBBox().min.x > maxx) return false;
        if (camera->getBBox().max.y < miny || camera->getBBox().min.y > maxy) return false;
        if (camera->getBBox().max.z < minz || camera->getBBox().min.z > maxz) return false;
        
        return true;  // Collision detected
    }

    void Entity::setPos(const glm::vec3& pos) {
        position_ = pos;
    }

    glm::vec3 Entity::getPos() {
        return position_;
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
