#ifndef ENTITIES_H_
#define ENTITIES_H_

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

    //struct BoundingBox {
    //    glm::vec3 min;  // Minimum coordinates of the bounding box
    //    glm::vec3 max;  // Maximum coordinates of the bounding box
    //};

    // Abstraction of an asteroid
    class Entity : public SceneNode {

    public:
        // Create asteroid from given resources
        Entity(const std::string name, const Resource* geometry, const Resource* material, const Resource* texture);

        // Destructor
        ~Entity();

        //bounding box
        void updateBoundingBox();
        //Entity::BoundingBox getBBox();
        void setBBox(glm::vec3 min, glm::vec3 max);
        void setBBoxValues(float x, float y, float z);

        bool checkPlayerCollision(Camera* camera);

        // Update geometry configuration
        void Update(void);

    private:

        BoundingBox boundingBox;

        //values to set BoundingBox checks
        float boxX = 7.0;
        float boxY = 10.0;
        float boxZ = 7.0;
       

    }; // class Entity


} // namespace game

#endif // ENTITIES_H_
