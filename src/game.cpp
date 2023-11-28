#include <iostream>
#include <time.h>
#include <sstream>

#include "game.h"
#include "path_config.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Main window settings
const std::string window_title_g = "The Woodland Revenant";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const bool window_full_screen_g = false;

// Viewport and camera settings
float camera_near_clip_distance_g = 0.01;
float camera_far_clip_distance_g = 1000.0;
float camera_fov_g = 60.0; // Field-of-view of camera (degrees)
const glm::vec3 viewport_background_color_g(0.0, 0.0, 0.0);
glm::vec3 camera_position_g(0.0, 0.0, 100.0);
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
    lastMousePos_ = glm::vec2(window_width_g / 2, window_height_g / 2);
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
    glfwSetCursorPosCallback(window_, MouseCallback);

    // Set pointer to game object, so that callbacks can access it
    glfwSetWindowUserPointer(window_, (void *) this);

    // Hide the cursor 
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


void Game::SetupResources(void){
    std::string signVerticesFilepath = std::string(MATERIAL_DIRECTORY) + std::string("/sign.customv");
    std::string signFacesFilepath = std::string(MATERIAL_DIRECTORY) + std::string("/sign.customf");


    //----------------------------------- Meshes ------------------------------------
    resman_.LoadCustomResource(Mesh, "SignPost", signVerticesFilepath.c_str(), signFacesFilepath.c_str());

    //Car Mesh
    std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/car.obj");
    resman_.LoadResource(Mesh, "Car", filename.c_str());

    //Cabin Mesh
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Cabin.obj");
    resman_.LoadResource(Mesh, "Cabin", filename.c_str());

    //Rock Meshes
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock1.obj");
    resman_.LoadResource(Mesh, "Rock_1", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock2.obj");
    resman_.LoadResource(Mesh, "Rock_2", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock3.obj");
    resman_.LoadResource(Mesh, "Rock_3", filename.c_str());

    //Tree
    resman_.CreateCylinder("BranchObject", 4.0, 0.5, 10, 10);

    //Gravestone
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/gravestoneRound.obj");
    resman_.LoadResource(Mesh, "Gravestone", filename.c_str());

    //Fence
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/fence.obj");
    resman_.LoadResource(Mesh, "Fence", filename.c_str());

    //Key
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/key.obj");
    resman_.LoadResource(Mesh, "Key", filename.c_str());

    //Ghost 
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/ghost2.obj");
    resman_.LoadResource(Mesh, "Ghost", filename.c_str());

    //SphereParticles
    resman_.CreateSphereParticles("SphereParticles", 20);

    //-------------------------------- Texture --------------------------------
    // Load texture to be used on the object
    //Sign Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Sign_tex.png");
    resman_.LoadResource(Texture, "SignTexture", filename.c_str());

    //Car Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Car_tex.png");
    resman_.LoadResource(Texture, "CarTexture", filename.c_str());

    //Cabin Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Cabin_tex.png");
    resman_.LoadResource(Texture, "CabinTexture", filename.c_str());

    //rock textures
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock1_tex.jpg");
    resman_.LoadResource(Texture, "Rock_1Texture", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock2_tex.png");
    resman_.LoadResource(Texture, "Rock_2Texture", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock3_tex.jpg");
    resman_.LoadResource(Texture, "Rock_3Texture", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock4_tex.png");
    resman_.LoadResource(Texture, "Rock_4Texture", filename.c_str());

    //Gravestone Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/gravestone_tex.jpg");
    resman_.LoadResource(Texture, "GravestoneTexture", filename.c_str());

    //Fence Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/fence_tex.jpg");
    resman_.LoadResource(Texture, "FenceTexture", filename.c_str());

    //Key Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/key_tex.jpeg");
    resman_.LoadResource(Texture, "KeyTexture", filename.c_str());

    // Tree Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/tree_tex.png");
    resman_.LoadResource(Texture, "TreeTexture", filename.c_str());

    // Moon Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/moon_texture.jpg");
    resman_.LoadResource(Texture, "MoonTexture", filename.c_str());

    // Cloth Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/whiteCloth_tex.png");
    resman_.LoadResource(Texture, "ClothTexture", filename.c_str());

    // Sparkle Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/sparkle_tex.png");
    resman_.LoadResource(Texture, "SparkleTexture", filename.c_str());

    // Cloud Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/cloud_tex.png");
    resman_.LoadResource(Texture, "CloudTexture", filename.c_str());

    //-------------------------------Materials-----------------------------
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/material");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/textured_material");
    resman_.LoadResource(Material, "TextureShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/lit_textured_material");
    resman_.LoadResource(Material, "LitTextureShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/lit_color");
    resman_.LoadResource(Material, "LitColorShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/textured_particle");
    resman_.LoadResource(Material, "Particle", filename.c_str());

    // Load material for screen-space effect
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/night_vision");
    resman_.LoadResource(Material, "NightVisionShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/wavering");
    resman_.LoadResource(Material, "WaveringShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/pixelated");
    resman_.LoadResource(Material, "PixelatedShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/drunk");
    resman_.LoadResource(Material, "DrunkShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/blur");
    resman_.LoadResource(Material, "BlurShader", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/bloody");
    resman_.LoadResource(Material, "BloodyShader", filename.c_str());

    std::vector<std::vector<float>> terrain = resman_.LoadTerrainResource(Type::Mesh, "TerrainMesh", MATERIAL_DIRECTORY "/terrain.heightfield");
    camera_.SetTerrainGrid(terrain);
    camera_.SetImpassableCells(resman_.GetImpassableCells(MATERIAL_DIRECTORY "/impassable.csv", terrain));

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/terrain");
    resman_.LoadResource(Material, "TerrainShader", filename.c_str());
}


void Game::SetupScene(void){

    // Set background color for the scene
    scene_.SetBackgroundColor(viewport_background_color_g);

    // Move the camera up a bit so its like its the players head
    camera_.Translate(camera_.GetUp() * 20.0f);
    
    Resource* geom = resman_.GetResource("TerrainMesh");
    Resource* mat  = resman_.GetResource("TerrainShader");
    Resource* text;
    Resource* mtext = resman_.GetResource("MoonTexture");
    
    SceneNode* terrain = scene_.CreateNode("Terrain", geom, mat, mtext);
    constexpr int bumpyNess = 2; // at 1 the terrain will vary between 1 and -1 in the y. Increasing this causes more jagged terrain
    terrain->SetScale(glm::vec3(100.0f, 25.0f, 100.0f));


    //Rock1
    geom = resman_.GetResource("Rock_1");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("Rock_1Texture");
    rock1_ = scene_.CreateNode("Rock1", geom, mat, text);
    rock1_->Scale(glm::vec3(1, 1, 1));
    rock1_->Translate(glm::vec3(175, 0, 0));

    // Trees
    SetupTree("Tree1");
    SetupTree("Tree2");

    SceneNode* tree1 = scene_.GetNode("Tree1_branch0");
    tree1->Scale(glm::vec3(5, 5, 5));
    tree1->Translate(glm::vec3(-200, 0, -200));
    SceneNode* tree2 = scene_.GetNode("Tree2_branch0");
    tree2->Scale(glm::vec3(5, 5, 5));
    tree2->Translate(glm::vec3(-300, 0, -250));

    //Ghost
    geom = resman_.GetResource("Ghost");
    mat = resman_.GetResource("LitTextureShader");
    //text = resman_.GetResource("Rock_1Texture");
    text = resman_.GetResource("ClothTexture");

    ghost = new Ghost("Ghost", geom, mat, text);
    //ghost->Scale(glm::vec3(0.2, 0.2, 0.2));
    ghost->Scale(glm::vec3(0.3, 0.3, 0.3));
    ghost->Translate(glm::vec3(0, 35, 0));
    scene_.AddNode(ghost);

    //Sparkles
    geom = resman_.GetResource("SphereParticles");
    mat = resman_.GetResource("Particle");
    text = resman_.GetResource("SparkleTexture");

    SceneNode* particles = scene_.CreateNode("TestSparkles1", geom, mat, text);
    particles->SetBlending(true);
    particles->Translate(glm::vec3(0, 30, 100));

    //Clouds
    geom = resman_.GetResource("SphereParticles");
    mat = resman_.GetResource("Particle");
    text = resman_.GetResource("CloudTexture");

    particles = scene_.CreateNode("TestSparkles2", geom, mat, text);
    particles->SetBlending(true);
    particles->Translate(glm::vec3(0, 30, 110));

    // Setup drawing to texture
    scene_.SetupDrawToTexture();
    use_screen_space_effects_ = true;
}


void Game::MainLoop(void){

    // Loop while the user did not close the window
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window_)){
        double currTime = glfwGetTime();
        double deltaTime = currTime - lastTime;
        lastTime = currTime;

        /*std::cout << camera_.GetPosition().x << std::endl;
        std::cout << camera_.GetPosition().y << std::endl;
        std::cout << camera_.GetPosition().z << std::endl;*/


        checkKeys(deltaTime);

        scene_.Update(&camera_, deltaTime);

        //check if contact with player has been made
        ghostContact();

        playerImmunity(deltaTime);

        // Draw the scene
        // Enable writing to depth buffer
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);
        if (!use_screen_space_effects_) {
            scene_.Draw(&camera_);
        }
        else {
            // enable these if using the drunk filter
            /*
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            */
            scene_.DrawToTexture(&camera_);
            scene_.DisplayTexture(resman_.GetResource("BloodyShader")->GetResource());
        }

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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    // Stop animation if space bar is pressed
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        game->animating_ = (game->animating_ == true) ? false : true;
    }
}

void Game::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    void* ptr = glfwGetWindowUserPointer(window);
    Game* game = (Game*)ptr;

    float xOffset = xpos - game->lastMousePos_.x;
    float yOffset = ypos - game->lastMousePos_.y;
    game->lastMousePos_.x = xpos;
    game->lastMousePos_.y = ypos;

    const float sensitivity = 0.002f;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    game->camera_.Pitch(-yOffset);
    game->camera_.Rotate(glm::normalize(glm::angleAxis(-xOffset, glm::vec3(0, 1, 0))));
}

void Game::checkKeys(double deltaTime) {
    // Check the state of keys for smooth input
    bool isWKeyPressed = glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS;
    bool isSKeyPressed = glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS;
    bool isAKeyPressed = glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS;
    bool isDKeyPressed = glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS;
    bool isFKeyPressed = glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS;

    bool isUpKeyPressed = glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS;
    bool isDownKeyPressed = glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS;
    bool isLeftKeyPressed = glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS;
    bool isRightKeyPressed = glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS;

    // Handle camera movement based on key states
    float trans_factor = 200.0f * deltaTime;

    if (isWKeyPressed || isUpKeyPressed) {
        camera_.Translate(camera_.GetForward() * trans_factor);
    }
    if (isSKeyPressed || isDownKeyPressed) {
        camera_.Translate(-camera_.GetForward() * trans_factor);
    }
    if (isAKeyPressed || isLeftKeyPressed) {
        camera_.Translate(-camera_.GetSide() * trans_factor);
    }
    if (isDKeyPressed || isRightKeyPressed) {
        camera_.Translate(camera_.GetSide() * trans_factor);
    }
    if (isFKeyPressed) {
        glfwMaximizeWindow(window_);
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

void Game::SetupTree(const std::string& tree_name) {
    int parts[50]; // Value of 1 = split and go diagonally, 2 = Go diagonally, 3 = split and go straight
    std::string parents[50];
    std::string names[50];

    parts[0] = 1;
    int j = 1;

    // Create Branches
    for (int i = 0; i < 30; ++i) {
        std::string name = tree_name + "_branch" + std::to_string(i);
        SceneNode* branch = CreateBranch(name);
        SceneNode* parent = scene_.GetNode(parents[i]);
        branch->SetParent(parent);
        names[i] = name;

        if (i != 0) {
            branch->SetPosition(glm::vec3(0, 3.5f, 0));
        }


        if (parts[i] == 1) { // Aim to the left and split to have two children (1 and 2)
            parts[j] = 1; // Append 1 and 2 to the part array for part behaviour
            parts[j + 1] = 2;
            parents[j] = name; // Set the parent of the indexes the part behaviour was set to to be this
            parents[j + 1] = name;
            j += 2;

            if (i != 0) {
                // Translate so orbiting around the bottom of the branch
                branch->SetOrbitTranslation(glm::vec3(0, 2, 0));
                // Orbit to the left
                branch->SetOrbitRotation(glm::normalize(glm::angleAxis(glm::pi<float>() / 8, glm::vec3(0, 0, 1))));
                branch->SetWindAffected(true); // Only this part is affected by wind
            }
        }
        else if (parts[i] == 2) { // Aim to the right and have one child that goes straight (3)
            parts[j] = 3; // Append 3 to the part array for part behaviour
            parents[j] = name; // Set the parent of the indexes the part behaviour was set to to be this
            j += 1;

            if (i != 0) {
                // Translate so orbiting around the bottom of the branch
                branch->SetOrbitTranslation(glm::vec3(0, 2, 0));
                // Orbit to the right
                branch->SetOrbitRotation(glm::normalize(glm::angleAxis(glm::pi<float>() / 8, glm::vec3(0, 0, -1))));
            }
        }
        else { // Aim straight and split to have two children (1 and 2)
            parts[j] = 1; // Append 1 and 2 to the part array for part behaviour
            parts[j + 1] = 2;
            parents[j] = name; // Set the parent of the indexes the part behaviour was set to to be this
            parents[j + 1] = name;
            j += 2;

            if (i != 0) {
                branch->SetOrbitTranslation(glm::vec3(0, 0, 0)); // Set orbit to 0 to make it not orbit at all (and stay straight)
                branch->SetOrbitRotation(glm::normalize(glm::angleAxis(0.0f, glm::vec3(0, 0, 0))));
            }
        }
    }
}

SceneNode* Game::CreateBranch(const std::string& name) {
    // Get resources
    Resource* geom = resman_.GetResource("BranchObject");
    if (!geom) {
        throw(GameException(std::string("Could not find branch resource")));
    }

    Resource* mat = resman_.GetResource("LitTextureShader");
    if (!mat) {
        throw(GameException(std::string("Could not find object material")));
    }

    Resource* text = resman_.GetResource("TreeTexture");

    // Create asteroid instance
    SceneNode* node = new SceneNode(name, geom, mat, text);
    scene_.AddNode(node);
    return node;
}

SceneNode* Game::CreateLeaf(const std::string& name) {
    // Get resources
    Resource* geom = resman_.GetResource("LeafObject");
    if (!geom) {
        throw(GameException(std::string("Could not find branch resource")));
    }

    Resource* mat = resman_.GetResource("ObjectMaterial");
    if (!mat) {
        throw(GameException(std::string("Could not find object material")));
    }

    // Create asteroid instance
    SceneNode* node = new SceneNode(name, geom, mat);
    scene_.AddNode(node);
    return node;
}


//collision with ghost
void Game::ghostContact() {

    //player contacted
    if (ghost->getContacted() == true) {
        hp -= 1;
        //printf("Hp = %d", hp);

        //do something when hp has run out
        if (hp <= 0) {

        }

        //printf("warping num: %d", test);
        // Generate random X and Z coordinates for the ghost to warp to after contact
        float randomX = float(std::rand()) / float(RAND_MAX) * 800.0f - 5.0f; // Adjust the range as needed
        float randomZ = float(std::rand()) / float(RAND_MAX) * 600.0f - 5.0f; // Adjust the range as needed

        //warp ghost to random position
        ghost->SetPosition(glm::vec3(randomX, 35, randomZ));

        //set contact to false
        ghost->setContacted(false);

        camera_.setImmune(true);
        camera_.setTimer(3.0f);
    }
}


void Game::playerImmunity(float deltaTime) {
    //player immunity
    if (camera_.getImmune() == true) {
        float time = camera_.getTimer() - deltaTime;
        camera_.setTimer(time);

        if (camera_.getTimer() <= 0.0f) {
            // Immunity has expired after 3 seconds
            camera_.setImmune(false);
        }
    }
}

} // namespace game
