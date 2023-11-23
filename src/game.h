#ifndef GAME_H_
#define GAME_H_

#include <exception>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene_graph.h"
#include "resource_manager.h"
#include "camera.h"
#include "asteroid.h"

namespace game {

    // Exception type for the game
    class GameException: public std::exception
    {
        private:
            std::string message_;
        public:
            GameException(std::string message) : message_(message) {};
            virtual const char* what() const throw() { return message_.c_str(); };
            virtual ~GameException() throw() {};
    };

    // Game application
    class Game {

        public:
            // Constructor and destructor
            Game(void);
            ~Game();
            // Call Init() before calling any other method
            void Init(void); 
            // Set up resources for the game
            void SetupResources(void);
            // Set up initial scene
            void SetupScene(void);
            // Run the game: keep the application active
            void MainLoop(void); 

        private:
            // GLFW window
            GLFWwindow* window_;

            // Scene graph containing all nodes to render
            SceneGraph scene_;

            // TODO: get rid of these
            SceneNode* sign_;
            SceneNode* car_;
            SceneNode* cabin_;
            SceneNode* rock1_;
            SceneNode* rock2_;
            SceneNode* rock3_;
            SceneNode* gravestone_;

            // Resources available to the game
            ResourceManager resman_;

            // Camera abstraction
            Camera camera_;

            // Flag to turn animation on/off
            bool animating_;

            // Methods to initialize the game
            void InitWindow(void);
            void InitView(void);
            void InitEventHandlers(void);
 
            // Methods to handle events
            static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void ResizeCallback(GLFWwindow* window, int width, int height);
            static void MouseCallback(GLFWwindow* window, double xpos, double ypos);

            //keys
            void checkKeys(double deltaTime);

            // Setting up tree
            void SetupTree(const std::string&);
            SceneNode* CreateBranch(const std::string&);
            SceneNode* CreateLeaf(const std::string&);

            // Mouse position
            glm::vec2 lastMousePos_;


    }; // class Game

} // namespace game

#endif // GAME_H_
