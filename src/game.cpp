#include <iostream>
#include <time.h>
#include <sstream>

#include "game.h"
#include "skybox.h"
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
float camera_near_clip_distance_g = 0.01f;
float camera_far_clip_distance_g = 1000.0;
float camera_fov_g = 60.0; // Field-of-view of camera (degrees)
const glm::vec3 viewport_background_color_g(0.0, 0.0, 0.0);
glm::vec3 camera_position_g(0.0, 0.0, 100.0);
glm::vec3 camera_look_at_g(0.0, 0.0, 110.0);
glm::vec3 camera_up_g(0.0, 1.0, 0.0);

// Materials 
const std::string material_directory_g = MATERIAL_DIRECTORY;


void Game::Init(void){

    // Run all initialization steps
    InitWindow();
    InitView();
    InitEventHandlers();

    // Set variables
    animating_ = true;
    lastMousePos_ = glm::vec2(window_width_g / 2, window_height_g / 2);
    last_interacted_ = glfwGetTime();
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
    camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, static_cast<GLfloat>(width), static_cast<GLfloat>(height));
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

    // UI
    resman_.CreatePlane("UI");

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

    //GasCan 
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/gasCan.obj");
    resman_.LoadResource(Mesh, "GasCan", filename.c_str());

    //Door 
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Door1.obj");
    resman_.LoadResource(Mesh, "Door", filename.c_str());

    //Ruin Wall 
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/stoneWall.obj");
    resman_.LoadResource(Mesh, "StoneWall", filename.c_str());

    //Ruin Wall Bent
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/stoneWallBent.obj");
    resman_.LoadResource(Mesh, "StoneWallBent", filename.c_str());

    //SphereParticles
    resman_.CreateSphereParticles("SphereParticles", 20);

    //Invisible vertex that follows camera
    resman_.CreateVertex("CameraVertex");

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

    // GasCan Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/GasCan_tex.png");
    resman_.LoadResource(Texture, "GasCanTex", filename.c_str());

    // Door Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/door_tex.png");
    resman_.LoadResource(Texture, "DoorTex", filename.c_str());

    // Ruin Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/Ruin_tex.png");
    resman_.LoadResource(Texture, "RuinTex", filename.c_str());

    // Main Menu Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/main.png");
    resman_.LoadResource(Texture, "MainText", filename.c_str());

    // Lose Screen Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/lose.png");
    resman_.LoadResource(Texture, "LoseText", filename.c_str());

    // Win Screen Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/win.png");
    resman_.LoadResource(Texture, "WinText", filename.c_str());


    //-------------------------------Materials-----------------------------
    filename = std::string(SHADERS_DIRECTORY) + std::string("/material");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/textured_material");
    resman_.LoadResource(Material, "TextureShader", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/lit_textured_material");
    resman_.LoadResource(Material, "LitTextureShader", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/lit_color");
    resman_.LoadResource(Material, "LitColorShader", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/textured_particle");
    resman_.LoadResource(Material, "Particle", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/terrain");
    resman_.LoadResource(Material, "TerrainShader", filename.c_str());

    //-------------------------------Screen Space Material------------------
    filename = std::string(SCREEN_SPACE_SHADERS_DIRECTORY) + std::string("/night_vision");
    resman_.LoadResource(SS_Material, "NightVisionShader", filename.c_str());

    filename = std::string(SCREEN_SPACE_SHADERS_DIRECTORY) + std::string("/wavering");
    resman_.LoadResource(SS_Material, "WaveringShader", filename.c_str());

    filename = std::string(SCREEN_SPACE_SHADERS_DIRECTORY) + std::string("/pixelated");
    resman_.LoadResource(SS_Material, "PixelatedShader", filename.c_str());

    filename = std::string(SCREEN_SPACE_SHADERS_DIRECTORY) + std::string("/drunk");
    resman_.LoadResource(SS_Material, "DrunkShader", filename.c_str());

    filename = std::string(SCREEN_SPACE_SHADERS_DIRECTORY) + std::string("/blur");
    resman_.LoadResource(SS_Material, "BlurShader", filename.c_str());

    filename = std::string(SCREEN_SPACE_SHADERS_DIRECTORY) + std::string("/bloody");
    resman_.LoadResource(SS_Material, "BloodyShader", filename.c_str());

    // Skybox
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/skybox/");
    resman_.LoadResource(SkyboxTexture, "SkyboxText", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/skybox");
    resman_.LoadResource(Material, "SkyboxProg", filename.c_str());

    resman_.AddResource(Mesh, "SkyboxMesh", resman_.GetSkyboxVBO(), 36);

    resman_.GenerateSkybox();
    // ---

    std::vector<std::vector<float>> terrain = resman_.LoadTerrainResource(Type::Mesh, "TerrainMesh", MATERIAL_DIRECTORY "/terrain.heightfield");
    terrain_grid_ = terrain;
    camera_.SetTerrainGrid(terrain);
    camera_.SetImpassableCells(resman_.GetImpassableCells(MATERIAL_DIRECTORY "/impassable.csv", terrain));
}


void Game::SetupScene(void){

    // Set background color for the scene
    scene_.SetBackgroundColor(viewport_background_color_g);

    // Move the camera up a bit so its like its the players head
    //camera_.Translate(camera_.GetUp() * 20.0f);
    
    
    Resource* geom = resman_.GetResource("TerrainMesh");
    Resource* mat  = resman_.GetResource("TerrainShader");
    Resource* text;
    Resource* mtext = resman_.GetResource("MoonTexture");
    
    SceneNode* terrain = scene_.CreateNode("Terrain", geom, mat, mtext);
    constexpr int bumpyNess = 2; // at 1 the terrain will vary between 1 and -1 in the y. Increasing this causes more jagged terrain
    int terrain_offset = 100;
    terrain->SetScale(glm::vec3(100.0f + terrain_offset, 25.0f, 100.0f + terrain_offset));
    terrain->SetPosition(glm::vec3(terrain_offset * 2, 0, terrain_offset * 2));

    // Vertex that follows camera
    geom = resman_.GetResource("CameraVertex");
    mat = resman_.GetResource("ObjectMaterial");
    SceneNode* cam_vertex = scene_.CreateNode("CameraVertex", geom, mat);

    // Setup drawing to texture
    scene_.SetupDrawToTexture();
    use_screen_space_effects_ = false;

    // Camera
    camera_.SetPosition(glm::vec3(0, 50, 0)); // Initialize to start position
    camera_.UpdateYPos();

    // Skybox
    geom = resman_.GetResource("SkyboxMesh");
    mat = resman_.GetResource("SkyboxProg");
    text = resman_.GetResource("SkyboxText");
    SceneNode* node = new Skybox("skybox", geom, mat, text);
    scene_.AddNode(node);

    node->Translate(glm::vec3(0, 0, 0));

    // MainMenu
    geom = resman_.GetResource("UI");
    mat = resman_.GetResource("TextureShader");
    text = resman_.GetResource("MainText");
    node = scene_.CreateNode("MainMenu", geom, mat, text);
    scene_.AddNode(node);

    node->Scale(glm::vec3(160, 100, 90));
    node->SetOrientation(glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0)));
    node->Rotate(glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1)));
    node->Translate(glm::vec3(0, 35, 200));

    // LoseScreen
    geom = resman_.GetResource("UI");
    mat = resman_.GetResource("TextureShader");
    text = resman_.GetResource("LoseText");
    node = scene_.CreateNode("LoseScreen", geom, mat, text);
    scene_.AddNode(node);

    node->Scale(glm::vec3(160, 100, 90));
    node->SetOrientation(glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0)));
    node->Rotate(glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1)));
    node->Translate(glm::vec3(0, 35, 200));

    // WinScreen
    geom = resman_.GetResource("UI");
    mat = resman_.GetResource("TextureShader");
    text = resman_.GetResource("WinText");
    node = scene_.CreateNode("WinScreen", geom, mat, text);
    scene_.AddNode(node);

    node->Scale(glm::vec3(160, 100, 90));
    node->SetOrientation(glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0)));
    node->Rotate(glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1)));
    node->Translate(glm::vec3(0, 35, 200));

    //Ghost
    geom = resman_.GetResource("Ghost");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("ClothTexture");

    ghost = new Ghost("Ghost", geom, mat, text);
    ghost->Scale(glm::vec3(0.3, 0.3, 0.3));
    ghost->Translate(glm::vec3(0, 35, 0));
    scene_.AddNode(ghost);

    // Car
    geom = resman_.GetResource("Car");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("CarTexture");
    car_ = scene_.CreateNode("Car", geom, mat, text);
    car_->Scale(glm::vec3(5, 5, 5));
    car_->Translate(glm::vec3(-200, 100, -220));
    car_->UpdateYPos(terrain_grid_, 8);

    // Trees
    SetupTree("Tree1");
    node = scene_.GetNode("Tree1_branch0");
    node->Scale(glm::vec3(5, 5, 5));
    node->Translate(glm::vec3(-100, 0, -200));
    node->UpdateYPos(terrain_grid_, 8);

    SetupTree("Tree2");
    node = scene_.GetNode("Tree2_branch0");
    node->Scale(glm::vec3(5, 5, 5));
    node->Translate(glm::vec3(-200, 0, -100));
    node->UpdateYPos(terrain_grid_, 8);

    SetupTree("Tree3");
    node = scene_.GetNode("Tree3_branch0");
    node->Scale(glm::vec3(5, 5, 5));
    node->Translate(glm::vec3(-50, 0, -50));
    node->UpdateYPos(terrain_grid_, 8);

    SetupTree("Tree4");
    node = scene_.GetNode("Tree4_branch0");
    node->Scale(glm::vec3(5, 5, 5));
    node->Translate(glm::vec3(-100, 0, 50));
    node->UpdateYPos(terrain_grid_, 8);

    SetupTree("Tree5");
    node = scene_.GetNode("Tree5_branch0");
    node->Scale(glm::vec3(5, 5, 5));
    node->Translate(glm::vec3(50, 0, -100));
    node->UpdateYPos(terrain_grid_, 8);

    // Gravestones
    geom = resman_.GetResource("Gravestone");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("GravestoneTexture");
    node = scene_.CreateNode("Gravestone1", geom, mat, text);
    node->Scale(glm::vec3(30, 30, 30));
    node->Translate(glm::vec3(-200, 0, -200));
    node->UpdateYPos(terrain_grid_, 0);

    // Tree Border
    for (int i = 0; i < 10; ++i) {
        std::cout << i << std::endl;
        SetupTree("TreeBorder" + std::to_string(i) + "A");
        SceneNode* treeA = scene_.GetNode("TreeBorder" + std::to_string(i) + "A_branch0");
        treeA->Scale(glm::vec3(5, 5, 5));
        treeA->Translate(glm::vec3(-230 + i * 200, 0, -230));
    }

    //Rock1
    /*geom = resman_.GetResource("Rock_1");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("Rock_1Texture");
    rock1_ = scene_.CreateNode("Rock1", geom, mat, text);
    rock1_->Scale(glm::vec3(1, 1, 1));
    rock1_->Translate(glm::vec3(175, 0, 0));

    // Tree Border
    for (int i = 0; i < 10; ++i) {
        std::cout << i << std::endl;
        SetupTree("TreeBorder" + std::to_string(i) + "A");
        SceneNode* treeA = scene_.GetNode("TreeBorder" + std::to_string(i) + "A_branch0");
        treeA->Scale(glm::vec3(5, 5, 5));
        treeA->Translate(glm::vec3(-230 + i * 200, 0, -230));
    }

    // Trees
    SetupTree("Tree1");
    SetupTree("Tree2");

    SceneNode* tree1 = scene_.GetNode("Tree1_branch0");
    tree1->Scale(glm::vec3(5, 5, 5));
    tree1->Translate(glm::vec3(-200, 0, -200));
    SceneNode* tree2 = scene_.GetNode("Tree2_branch0");
    tree2->Scale(glm::vec3(5, 5, 5));
    tree2->Translate(glm::vec3(-300, 0, -250));*/

    

    /*
    

    //GasCan
    geom = resman_.GetResource("GasCan");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("GasCanTex");
    gasCan_ = scene_.CreateNode("GasCan", geom, mat, text);
    gasCan_->Scale(glm::vec3(5, 5, 5));
    // Create a quaternion for a 90-degree rotation
    glm::quat rotationQuat = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    gasCan_->Rotate(rotationQuat);
    gasCan_->Translate(glm::vec3(-170, 35, -170));
    scene_.AddNode(gasCan_);

    //door
    geom = resman_.GetResource("Door");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("DoorTex");
    door_ = scene_.CreateNode("Door", geom, mat, text);
    door_->Scale(glm::vec3(0.15, 0.15, 0.15));
    door_->Translate(glm::vec3(-135, 30, -165));

    //Stone Wall for Ruin
    geom = resman_.GetResource("StoneWall");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("RuinTex");
    sWall_ = scene_.CreateNode("StoneWall", geom, mat, text);
    sWall_->Scale(glm::vec3(0.1, 0.1, 0.1));
    sWall_->Translate(glm::vec3(-85, 30, -165));

    //Stone Wall for Ruin
    geom = resman_.GetResource("StoneWallBent");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("RuinTex");
    sWallBent_ = scene_.CreateNode("StoneWallBent", geom, mat, text);
    sWallBent_->Scale(glm::vec3(0.2, 0.2, 0.2));
    sWallBent_->Translate(glm::vec3(-85, 30, -130));

    //Entity Object for testing
    geom = resman_.GetResource("GasCan");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("GasCanTex");
    Entity* sWall2 = new Entity("EntityCan", geom, mat, text);
    sWall2->Scale(glm::vec3(5, 5, 5));
    sWall2->Translate(glm::vec3(-85, 30, -90));
    scene_.AddNode(sWall2);
    entities.push_back(sWall2);*/

    //Sparkles
    /*geom = resman_.GetResource("SphereParticles");
    mat = resman_.GetResource("Particle");
    text = resman_.GetResource("SparkleTexture");

    SceneNode* particles = scene_.CreateNode("TestSparkles1", geom, mat, text);
    particles->SetBlending(true);
    particles->Translate(glm::vec3(0, 30, 100));*/

    //Clouds
    /*geom = resman_.GetResource("SphereParticles");
    mat = resman_.GetResource("Particle");
    text = resman_.GetResource("CloudTexture");

    particles = scene_.CreateNode("TestSparkles2", geom, mat, text);
    particles->SetBlending(true);
    particles->Translate(glm::vec3(0, 30, 110));

    
    particles->Translate(glm::vec3(0, 30, 110));*/

    


    // Test interactable geometry
    // Create 3 keys
    /*geom = resman_.GetResource("Key");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("KeyTexture");

    glm::vec3 key_held_pos = glm::vec3(-3, -1, 8);
    glm::vec3 key_held_scale = glm::vec3(12, 12, 12);
    glm::quat key_held_orientation = glm::angleAxis(glm::radians(-95.0f), glm::vec3(0, 1, 0));

    InteractableNode* obj1 = scene_.CreateInteractableNode("Interactable1", geom, mat, text);
    obj1->Translate(glm::vec3(0,30,0));
    obj1->Scale(glm::vec3(35, 35, 35));
    obj1->SetPositioning(key_held_pos, key_held_scale, key_held_orientation, obj1->GetScale(), obj1->GetOrientation());

    InteractableNode* obj2 = scene_.CreateInteractableNode("Interactable2", geom, mat, text);
    obj2->Translate(glm::vec3(0, 30, 40));
    obj2->Scale(glm::vec3(35, 35, 35));
    obj2->SetPositioning(key_held_pos, key_held_scale, key_held_orientation, obj2->GetScale(), obj2->GetOrientation());

    InteractableNode* obj3 = scene_.CreateInteractableNode("Interactable3", geom, mat, text);
    obj3->Translate(glm::vec3(0, 30, -40));
    obj3->Scale(glm::vec3(35, 35, 35));
    obj3->SetPositioning(key_held_pos, key_held_scale, key_held_orientation, obj3->GetScale(), obj3->GetOrientation());

    // Create starting particles for the keys
    geom = resman_.GetResource("SphereParticles");
    mat = resman_.GetResource("Particle");
    text = resman_.GetResource("SparkleTexture");

    SceneNode* particles1 = scene_.CreateNode("Sparkles1", geom, mat, text);
    particles1->SetBlending(true);
    SceneNode* particles2 = scene_.CreateNode("Sparkles2", geom, mat, text);
    particles2->SetBlending(true);
    SceneNode* particles3 = scene_.CreateNode("Sparkles3", geom, mat, text);
    particles3->SetBlending(true);

    obj1->SetParticles(particles1);
    obj2->SetParticles(particles2);
    obj3->SetParticles(particles3);*/

    
    
}


void Game::MainLoop(void){
    const char* ssShaders[] = {"None",
                               "NightVisionShader",
                               "WaveringShader",
                               "PixelatedShader",
                               "DrunkShader",
                               "BlurShader",
                               "BloodyShader"
                               };

    // Loop while the user did not close the window
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window_)){
        double currTime = glfwGetTime();
        double deltaTime = currTime - lastTime;
        lastTime = currTime;


        checkKeys(deltaTime);

        scene_.Update(&camera_, deltaTime, gamePhase_);

        SceneNode* cam_vertex = scene_.GetNode("CameraVertex");
        cam_vertex->SetPosition(camera_.GetPosition() - glm::vec3(0,3.5,0));
        cam_vertex->SetOrientation(camera_.GetOrientation());

        //check if contact with player has been made
        ghostContact();

        playerImmunity(static_cast<float>(deltaTime));

        std::cout << camera_.GetPosition().x << " " << camera_.GetPosition().z << std::endl;

        //check if player is at contact with an impassable entity
        checkEntityCollision();

        // Draw the scene
        // Enable writing to depth buffer
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);
        if (!use_screen_space_effects_) {
            scene_.Draw(&camera_, gamePhase_);
        }
        else {
            // enable these if using the drunk filter
            if (screen_space_effect_index_ == 4) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            
            scene_.DrawToTexture(&camera_);
            scene_.DisplayTexture(resman_.GetResource(ssShaders[screen_space_effect_index_])->GetResource());

            if (screen_space_effect_index_ == 4) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
        }

        // Push buffer drawn in the background onto the display
        glfwSwapBuffers(window_);

        // Update other events like input handling
        glfwPollEvents();

        // Enable writing to depth buffer
        glDepthMask(GL_TRUE);
    }
}


void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    // Get user data with a pointer to the game class
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;

    // Handle UI key presses
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if(game->gamePhase_ == title) {
            // Start game
            game->gamePhase_ = gameplay;
        }
        else if (game->gamePhase_ == gameLost || game->gamePhase_ == gameWon) {
            // Quit game now that the game is over
            glfwSetWindowShouldClose(window, true);
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        // Quit game early
        glfwSetWindowShouldClose(window, true);
    }
}

void Game::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    void* ptr = glfwGetWindowUserPointer(window);
    Game* game = (Game*)ptr;

    // Check if in UI
    if (game->gamePhase_ == title || game->gamePhase_ == gameLost || game->gamePhase_ == gameWon) {
        glfwSetCursorPos(window, game->lastMousePos_.x, game->lastMousePos_.y);
		return;
	}

    float xOffset = static_cast<float>(xpos) - game->lastMousePos_.x;
    float yOffset = static_cast<float>(ypos) - game->lastMousePos_.y;
    
    // Prevent the player from seeing behind them from the bottom
    if (yOffset > 0 && ypos > 1200) {
        glfwSetCursorPos(window, xpos, 1200);
        yOffset = 0;
        ypos = 1200;
    }

    // Prevent the player from seeing behind them from the top
    else if (yOffset < 0 && ypos < -1200) {
        glfwSetCursorPos(window, xpos, -1200);
        yOffset = 0;
        ypos = -1200;
    }

    game->lastMousePos_.x = static_cast<float>(xpos);
    game->lastMousePos_.y = static_cast<float>(ypos);

    const float sensitivity = 0.002f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    game->camera_.Pitch(-yOffset);
    game->camera_.Rotate(glm::normalize(glm::angleAxis(-xOffset, glm::vec3(0, 1, 0))));
}

void Game::checkKeys(double deltaTime) {
    // TODO: remove
    static double lastTime = glfwGetTime();

    // Check if in UI
    if (gamePhase_ == title || gamePhase_ == gameLost || gamePhase_ == gameWon) {
		return;
	}

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

    bool isEKeyPressed = glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS;

    // Handle camera movement based on key states
    float trans_factor = 60.0f * static_cast<float>(deltaTime);

    if (isWKeyPressed || isUpKeyPressed) {
        camera_.Translate(camera_.GetForward() * trans_factor);
        camera_.updateBoundingBox();
    }
    if (isSKeyPressed || isDownKeyPressed) {
        camera_.Translate(-camera_.GetForward() * trans_factor);
        camera_.updateBoundingBox();
    }
    if (isAKeyPressed || isLeftKeyPressed) {
        camera_.Translate(-camera_.GetSide() * trans_factor);
        camera_.updateBoundingBox();
    }
    if (isDKeyPressed || isRightKeyPressed) {
        camera_.Translate(camera_.GetSide() * trans_factor);
        camera_.updateBoundingBox();
    }

    // Handle interaction
    if (isEKeyPressed) {
        OnInteract();
    }
    if (isFKeyPressed) {
        glfwMaximizeWindow(window_);
    }
}


void Game::OnInteract() {
    double curr_time = glfwGetTime();
    if (curr_time >= last_interacted_ + INTERACT_COOLDOWN) {

        if (held_item_) { // Drop currently held item
            //std::cout << "Dropping held item" << std::endl;

            Resource* geom = resman_.GetResource("SphereParticles");
            Resource* mat = resman_.GetResource("Particle");
            Resource* text = resman_.GetResource("SparkleTexture");
            SceneNode* new_particles = scene_.CreateNode(held_item_->GetName() + "Sparkles", geom, mat, text);
            new_particles->SetBlending(true);

            held_item_->SetParticles(new_particles);
            held_item_->SetParent(NULL);
            held_item_->SetPosition(camera_.GetPosition() + 10.0f*camera_.GetForward() - glm::vec3(0, 6, 0));
            held_item_->SetOrientation(held_item_->GetWorldOrientation());
            held_item_->SetScale(held_item_->GetWorldScale());

            held_item_ = NULL;
        }
        else { // Find nearest valid interactable
            //std::cout << "Attempting to pick up an item" << std::endl;

            InteractableNode* chosen_interactable = NULL;

            std::vector<InteractableNode*> nodes = scene_.GetInteractableNodes();

            for (size_t i = 0; i < nodes.size(); ++i) { // Search through all interactables; find closest one within range
                if (chosen_interactable) {
                    float dist_to_chosen = glm::length(camera_.GetPosition() - chosen_interactable->GetPosition());
                    float dist_to_new = glm::length(camera_.GetPosition() - nodes[i]->GetPosition());
                    if (dist_to_new < dist_to_chosen) {
                        chosen_interactable = nodes[i];
                    }
                }
                else {
                    float dist_to_new = glm::length(camera_.GetPosition() - nodes[i]->GetPosition());
                    //std::cout << dist_to_new << std::endl;
                    if (dist_to_new < INTERACT_RADIUS) {
                        chosen_interactable = nodes[i];
                    }
                }
            }

            if (chosen_interactable) {
                scene_.DeleteNode(chosen_interactable->GetParticles()->GetName());
                held_item_ = chosen_interactable;
                held_item_->SetParent(scene_.GetNode("CameraVertex"));
                held_item_->SetPosition(held_item_->GetHeldPos());
                held_item_->SetScale(held_item_->GetHeldScale());
                held_item_->SetOrientation(held_item_->GetHeldOrientation());
                //std::cout << "held_item_ pos: " + std::to_string(held_item_->GetPosition().x) + " " + std::to_string(held_item_->GetPosition().y) + " " + std::to_string(held_item_->GetPosition().z) << std::endl;
            }
            else {
                //std::cout << "No valid interactable found near player" << std::endl;
            }
        }

        last_interacted_ = curr_time;
    }
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height){

    // Set up viewport and camera projection based on new window size
    glViewport(0, 0, width, height);
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;
    game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, static_cast<GLfloat>(width), static_cast<float>(height));
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


void Game::checkEntityCollision() {

    camera_.updateBoundingBox();

    //loop through entities
    for(const auto& entity : entities) {
        bool collision = entity->checkPlayerCollision(&camera_);
        
        //if colliding with an entity
        if (collision) {
            camera_.SetPosition(originalPos);
            camera_.updateBoundingBox();
        }
    }

    originalPos = camera_.GetPosition();

}

} // namespace game
