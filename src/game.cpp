#include <iostream>
#include <time.h>
#include <sstream>

#include "game.h"
#include "path_config.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Main window settings
const std::string window_title_g = "Asteroid Field";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const bool window_full_screen_g = false;

// Viewport and camera settings
float camera_near_clip_distance_g = 0.01;
float camera_far_clip_distance_g = 1000.0;
float camera_fov_g = 60.0; // Field-of-view of camera (degrees)
const glm::vec3 viewport_background_color_g(0.0, 0.0, 0.0);
glm::vec3 camera_position_g(0.0, 0.0, 800.0);
glm::vec3 camera_look_at_g(0.0, 0.0, 0.0);
glm::vec3 camera_up_g(0.0, 1.0, 0.0);

// Materials 
const std::string material_directory_g = MATERIAL_DIRECTORY;


Game::Game(void){

    // Don't do work in the constructor, leave it for the Init() function
}


void Game::Init(void){

    // Run all initialization steps
    InitWindow();
    InitView();
    InitEventHandlers();

    // Set variables
    animating_ = true;
}

       
void Game::InitWindow(void){

    // Initialize the window management library (GLFW)
    if (!glfwInit()){
        throw(GameException(std::string("Could not initialize the GLFW library")));
    }

    // Create a window and its OpenGL context
    if (window_full_screen_g){
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), glfwGetPrimaryMonitor(), NULL);
    } else {
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
    }
    if (!window_){
        glfwTerminate();
        throw(GameException(std::string("Could not create window")));
    }

    // Make the window's context the current 
    glfwMakeContextCurrent(window_);

    // Initialize the GLEW library to access OpenGL extensions
    // Need to do it after initializing an OpenGL context
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        throw(GameException(std::string("Could not initialize the GLEW library: ")+std::string((const char *) glewGetErrorString(err))));
    }
}


void Game::InitView(void){

    // Set up z-buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Set viewport
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    glViewport(0, 0, width, height);

    // Set up camera
    // Set current view
    camera_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
    // Set projection
    camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
}


void Game::InitEventHandlers(void){

    // Set event callbacks
    glfwSetKeyCallback(window_, KeyCallback);
    glfwSetFramebufferSizeCallback(window_, ResizeCallback);

    // Set pointer to game object, so that callbacks can access it
    glfwSetWindowUserPointer(window_, (void *) this);
}


void Game::SetupResources(void){
    std::string signVerticesFilepath = std::string(MATERIAL_DIRECTORY) + std::string("/sign.customv");
    std::string signFacesFilepath = std::string(MATERIAL_DIRECTORY) + std::string("/sign.customf");


    resman_.LoadCustomResource(Mesh, "SignPost", signVerticesFilepath.c_str(), signFacesFilepath.c_str());
    std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/car.obj");
    resman_.LoadResource(Mesh, "Car", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Cabin.obj");
    resman_.LoadResource(Mesh, "Cabin", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/material");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/textured_material");
    resman_.LoadResource(Material, "TextureShader", filename.c_str());

    // Load texture to be used on the object
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Sign_tex.png");
    resman_.LoadResource(Texture, "SignTexture", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Car_tex.png");
    resman_.LoadResource(Texture, "CarTexture", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Cabin_tex.png");
    resman_.LoadResource(Texture, "CabinTexture", filename.c_str());
}


void Game::SetupScene(void){

    // Set background color for the scene
    scene_.SetBackgroundColor(viewport_background_color_g);

    Resource* geom = resman_.GetResource("SignPost");
    Resource* mat = resman_.GetResource("TextureShader");
    Resource* text = resman_.GetResource("SignTexture");
    sign_ = scene_.CreateNode("SignPost", geom, mat, text);
    sign_->Scale(glm::vec3(100, 100, 100));

    geom = resman_.GetResource("Car");
    mat = resman_.GetResource("TextureShader");
    text = resman_.GetResource("CarTexture");
    car_ = scene_.CreateNode("Car", geom, mat, text);
    car_->Scale(glm::vec3(10, 10, 10));
    car_->Translate(glm::vec3(0, 0, 50));

    geom = resman_.GetResource("Cabin");
    mat = resman_.GetResource("TextureShader");
    text = resman_.GetResource("CabinTexture");
    cabin_ = scene_.CreateNode("Cabin", geom, mat, text);
    cabin_->Scale(glm::vec3(100, 100, 100));
    cabin_->Translate(glm::vec3(0, 0, 1000));


}


void Game::MainLoop(void){

    // Loop while the user did not close the window
    while (!glfwWindowShouldClose(window_)){
        // Animate the scene
        if (animating_){
            static double last_time = 0;
            double current_time = glfwGetTime();
            if ((current_time - last_time) > 0.05){
                scene_.Update();
                last_time = current_time;
            }
        }

        // Draw the scene
        scene_.Draw(&camera_);

        // Push buffer drawn in the background onto the display
        glfwSwapBuffers(window_);

        // Update other events like input handling
        glfwPollEvents();
    }
}


void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    // Get user data with a pointer to the game class
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;

    // Quit game if 'q' is pressed
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    // Stop animation if space bar is pressed
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        game->animating_ = (game->animating_ == true) ? false : true;
    }

    // View control
    float rot_factor(2 * glm::pi<float>() / 180); // amount the ship turns per keypress
    float trans_factor = 10.0; // amount the ship steps forward per keypress
    if (key == GLFW_KEY_UP){
        game->camera_.Pitch(rot_factor);
    }
    if (key == GLFW_KEY_DOWN){
        game->camera_.Pitch(-rot_factor);
    }
    if (key == GLFW_KEY_LEFT){
        game->camera_.Yaw(rot_factor);
    }
    if (key == GLFW_KEY_RIGHT){
        game->camera_.Yaw(-rot_factor);
    }
    if (key == GLFW_KEY_S){
        game->camera_.Roll(-rot_factor);
    }
    if (key == GLFW_KEY_X){
        game->camera_.Roll(rot_factor);
    }
    if (key == GLFW_KEY_A){
        game->camera_.Translate(game->camera_.GetForward()*trans_factor);
    }
    if (key == GLFW_KEY_Z){
        game->camera_.Translate(-game->camera_.GetForward()*trans_factor);
    }
    if (key == GLFW_KEY_J){
        game->camera_.Translate(-game->camera_.GetSide()*trans_factor);
    }
    if (key == GLFW_KEY_L){
        game->camera_.Translate(game->camera_.GetSide()*trans_factor);
    }
    if (key == GLFW_KEY_I){
        game->camera_.Translate(game->camera_.GetUp()*trans_factor);
    }
    if (key == GLFW_KEY_K){
        game->camera_.Translate(-game->camera_.GetUp()*trans_factor);
    }
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height){

    // Set up viewport and camera projection based on new window size
    glViewport(0, 0, width, height);
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;
    game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
}


Game::~Game(){
    
    glfwTerminate();
}


} // namespace game
