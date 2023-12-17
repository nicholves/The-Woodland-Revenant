#ifndef GAME_H_
#define GAME_H_

#include <exception>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#ifdef USE_SOUND
#include <BASS/bass.h>
#endif

#include "scene_graph.h"
#include "resource_manager.h"
#include "camera.h"
#include "scene_graph.h"

#include "ghost.h"
#include "entities.h"

// Interaction related constants
#define INTERACT_COOLDOWN 2
#define INTERACT_RADIUS 40.0

namespace game {

    // Exception type for the game
    class GameException: public std::exception
    {
        private:
            std::string message_;
        public:
            GameException(std::string message) : message_(message) {};
            virtual const char* what() const throw() { return message_.c_str(); };
            virtual ~GameException() {};
    };

    // Game application
    class Game {

        public:
            // Constructor and destructor
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

            int hp = 3;

            // TODO: get rid of these
            SceneNode* sign_;
            SceneNode* car_;
            SceneNode* cabin_;
            SceneNode* rock1_;
            SceneNode* rock2_;
            SceneNode* rock3_;
            SceneNode* gravestone_;
            Ghost* ghost;
            SceneNode* gasCan_;
            SceneNode* door_;
            SceneNode* sWall_;
            SceneNode* sWallBent_;
            SceneNode* log;
            //InteractableNode* log2;

            //entities vector
            std::vector<Entity> entities;
            glm::vec3 originalPos;

            // Held interactable item
            InteractableNode* held_item_ = NULL;

            // Resources available to the game
            ResourceManager resman_;

            // Camera abstraction
            Camera camera_;

#ifdef USE_SOUND
            // sound effect channel
            HCHANNEL oofChannel_;
            HCHANNEL deathChannel_;
#endif

            // Flag to use screen space effects or not
            bool use_screen_space_effects_ = false;
            int screen_space_effect_index_ = 0;

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

            // Interaction
            double last_interacted_;
            void OnInteract();

            // Setting up tree
            void SetupTree(const std::string&);
            SceneNode* CreateBranch(const std::string&);
            SceneNode* CreateLeaf(const std::string&);

            // Mouse position
            glm::vec2 lastMousePos_;

            // Terrain Grid
            std::vector<std::vector<float>> terrain_grid_;

            // Game Phase
            GamePhase gamePhase_ = title;

            //collision with ghost
            bool ghostContact();

            //entity collisions
            void checkEntityCollision();

            void adjustBlurFactor();

            typedef struct { float minx; float maxx; float minz; float maxz; } boundingArea;

            // Summon Objects
            void SummonFence(std::string name, glm::vec3 position, float rotation = 0);
            void SummonCar(std::string name, glm::vec3 position, float rotation = 0);
            void SummonUI(std::string name, std::string texture);
            void SummonGhost(std::string name, glm::vec3 position);
            void SummonTree(std::string name, glm::vec3 position, float rotation = 0);
            void SummonCabin(std::string name, glm::vec3 position, float rotation = 0);
            void SummonSign(std::string name, glm::vec3 position, float rotation = 0);
            void SummonPlane(std::string name, std::string texture, glm::vec3 position, glm::vec3 scale, float rotation = 0);
            void SummonInstancedObjects(std::string name, std::string geometry, std::string texture, int amount, glm::vec3 scale, const glm::vec3& boundingBox, std::vector<boundingArea>, int seed, std::vector<glm::vec3>& filledPosses);
            void SummonRuins(std::string name, glm::vec3 position);
            void SummonDoor(std::string name, glm::vec3 position, float rotation = 0);
            void SummonGasCan(std::string name, glm::vec3 position, float rotation = 0);
            void SummonRuinWall(std::string name, glm::vec3 position, float rotation = 0);
            void SummonKey(std::string name, glm::vec3 position);
            void SummonInsects(std::string name, glm::vec3 position);
            void SummonLog(std::string name, glm::vec3 position, float rotation = 0);

    }; // class Game

} // namespace game

#endif // GAME_H_
