#ifndef ENTITIES_H_
#define ENTITIES_H_

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

#include "camera.h"

namespace game {

    //struct BoundingBox {
    //    glm::vec3 min;  // Minimum coordinates of the bounding box
    //    glm::vec3 max;  // Maximum coordinates of the bounding box
    //};
    class Entity {

    public:
        Entity(const glm::vec3& min, const glm::vec3& max, const glm::vec3& pos);
        Entity(float x, float y, float z, const glm::vec3& pos);

        // Destructor
        ~Entity();

        //bounding box
        void updateBoundingBox();
        void setBBox(glm::vec3 min, glm::vec3 max);
        void setBBoxValues(float x, float y, float z);

        bool checkPlayerCollision(Camera* camera) const;
        void setPos(const glm::vec3& pos);

        // Update geometry configuration
        void Update(void);

        void setOrientation(glm::quat orientation);

    private:
        glm::quat orientation_ = glm::quat(glm::angleAxis(0.0f, glm::vec3(-1, 0, 0)));
        BoundingBox boundingBox;
        glm::vec3 position_;

        //values to set BoundingBox checks
        float boxX = 7.0;
        float boxY = 10.0;
        float boxZ = 7.0;
       

    }; // class Entity


} // namespace game

#endif // ENTITIES_H_
