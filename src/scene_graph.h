#ifndef SCENE_GRAPH_H_
#define SCENE_GRAPH_H_

#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene_node.h"
#include "resource.h"
#include "camera.h"

#define FRAME_BUFFER_WIDTH 1920
#define FRAME_BUFFER_HEIGHT 1080

namespace game {

    // Class that manages all the objects in a scene
    class SceneGraph {

    private:
        // Background color
        glm::vec3 background_color_;

        // Scene nodes to render
        std::vector<SceneNode*> node_;

        // Frame buffer for drawing to texture
        GLuint frame_buffer_;
        // Quad vertex array for drawing from texture
        GLuint quad_array_buffer_;
        // Render targets
        GLuint texture_;
        GLuint depth_buffer_;

    public:
        static int blurrSamples;
        static float bloodFactor;
        static float pixelSpacing;

        // Constructor and destructor
        SceneGraph(void);
        ~SceneGraph();

        // Background color
        void SetBackgroundColor(glm::vec3 color);
        glm::vec3 GetBackgroundColor(void) const;

        // Create a scene node from the specified resources
        SceneNode* CreateNode(std::string node_name, Resource* geometry, Resource* material, Resource* texture = NULL);
        // Add an already-created node
        void AddNode(SceneNode* node);
        // Find a scene node with a specific name
        SceneNode* GetNode(std::string node_name) const;
        // Get node const iterator
        std::vector<SceneNode*>::const_iterator begin() const;
        std::vector<SceneNode*>::const_iterator end() const;

        // Draw the entire scene
        void Draw(Camera* camera);

        // Update entire scene
        //void Update(void);
        void Update(Camera* camera, float deltaTime);

        // Drawing from/to a texture
        // Setup the texture
        void SetupDrawToTexture(void);
        // Draw the scene into a texture
        void DrawToTexture(Camera* camera);
        // Process and draw the texture on the screen
        void DisplayTexture(GLuint program);
        // Save texture to a file in ppm format
        void SaveTexture(char* filename);
    }; // class SceneGraph

} // namespace game

#endif // SCENE_GRAPH_H_
