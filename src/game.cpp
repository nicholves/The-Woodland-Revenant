#define NOMINMAX
#include <iostream>
#include <time.h>
#include <sstream>
#include <algorithm>

#include "game.h"
#include "skybox.h"
#include "path_config.h"
#include "instanced_object.h"

#define GAMEPLAY_MUSIC_VOLUME 0.2f
#define INITIAL_MENU_MUSIC_VOLUME 0.3f

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
const glm::vec3 camera_position_g(0.0, 0.0, 100.0);
const glm::vec3 camera_look_at_g(0.0, 0.0, 110.0);
const glm::vec3 camera_up_g(0.0, 1.0, 0.0);

// Materials 
const std::string material_directory_g = MATERIAL_DIRECTORY;


void Game::Init(void){

    // Run all initialization steps
    InitWindow();
    InitView();
    InitEventHandlers();

    // Set variables
    lastMousePos_ = glm::vec2(window_width_g / 2, window_height_g / 2);
    last_interacted_ = glfwGetTime();

    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        MessageBox(NULL, "BASS initialization failed!", "Error", MB_OK | MB_ICONERROR);
        throw ("ERROR: BASS LIBRARY FAILED TO INITIALIZE");
    }
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

    // UI & Wall
    resman_.CreatePlane("UI");
    resman_.CreatePlane("Wall", 20);

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

    // Tree
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/tree1.obj");
    resman_.LoadResource(Mesh, "Tree1", filename.c_str());

    // Tree 2
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/laubbaum.obj");
    resman_.LoadResource(Mesh, "Tree2", filename.c_str());

    //SphereParticles
    resman_.CreateSphereParticles("SphereParticles", 20);

    //Invisible vertex that follows camera
    resman_.CreateVertex("CameraVertex");

    //Road geometry
    resman_.CreateWall("RoadRect", glm::vec3(0.5f, 0.5f, 0.5f));

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

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/tree_tex1.png");
    resman_.LoadResource(Texture, "TreeTexture1", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture_laubbaum.png");
    resman_.LoadResource(Texture, "TreeTexture2", filename.c_str());

    // Moon Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/moon_texture.jpg");
    resman_.LoadResource(Texture, "MoonTexture", filename.c_str());

    // Grass Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/grass_tex.png");
    resman_.LoadResource(Texture, "GrassTexture", filename.c_str());

    // Rock Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/rock_tex.png");
    resman_.LoadResource(Texture, "RockTexture", filename.c_str());

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

    // Water Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/water_tex.jpg");
    resman_.LoadResource(Texture, "WaterText", filename.c_str());

    // Asphalt Texture
    filename = std::string(MATERIAL_DIRECTORY) + std::string("/road_tex.png");
    resman_.LoadResource(Texture, "RoadText", filename.c_str());

    //-------------------------------Materials-----------------------------
    filename = std::string(SHADERS_DIRECTORY) + std::string("/material");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/textured_material");
    resman_.LoadResource(Material, "TextureShader", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/lit_textured_material");
    resman_.LoadResource(Material, "LitTextureShader", filename.c_str());

    filename = std::string(SHADERS_DIRECTORY) + std::string("/lit_textured_material_instanced");
    resman_.LoadResource(Material, "LitTextureInstanceShader", filename.c_str());

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

    const char* filepath = AUDIO_DIRECTORY "/oof.wav";
    HSAMPLE oofSample = BASS_SampleLoad(FALSE, filepath, 0, 0, 3, BASS_SAMPLE_OVER_POS);
    if (oofSample == 0) {
        MessageBox(NULL, "Failed to load sound file!", "Error", MB_OK | MB_ICONERROR);
        BASS_Free();
        throw ("ERROR: BASS FAILED TO LOAD SOUND FILE" + std::string(filepath));
    }

    oofChannel_ = BASS_SampleGetChannel(oofSample, FALSE);
    BASS_ChannelSetAttribute(oofChannel_, BASS_ATTRIB_VOL, 0.5f);

    const char* filepath2 = AUDIO_DIRECTORY "/death.wav";
    HSAMPLE deathSample = BASS_SampleLoad(FALSE, filepath2, 0, 0, 3, BASS_SAMPLE_OVER_POS);
    if (oofSample == 0) {
        MessageBox(NULL, "Failed to load sound file!", "Error", MB_OK | MB_ICONERROR);
        BASS_Free();
        throw ("ERROR: BASS FAILED TO LOAD SOUND FILE" + std::string(filepath2));
    }

    deathChannel_ = BASS_SampleGetChannel(deathSample, FALSE);
    BASS_ChannelSetAttribute(deathChannel_, BASS_ATTRIB_VOL, 0.5f);
}


void Game::SetupScene(void){

    // Set background color for the scene
    scene_.SetBackgroundColor(viewport_background_color_g);

    // Move the camera up a bit so its like its the players head
    //camera_.Translate(camera_.GetUp() * 20.0f);
    
    
    Resource* geom = resman_.GetResource("TerrainMesh");
    Resource* mat  = resman_.GetResource("TerrainShader");
    Resource* text;
    Resource* mtext = resman_.GetResource("GrassTexture");
    
    SceneNode* terrain = scene_.CreateNode("Terrain", geom, mat, mtext);
    constexpr int bumpyNess = 2; // at 1 the terrain will vary between 1 and -1 in the y. Increasing this causes more jagged terrain
    int terrain_offset = 100;
    terrain->SetScale(glm::vec3(100.0f + terrain_offset, 25.0f, 100.0f + terrain_offset));
    terrain->SetPosition(glm::vec3(terrain_offset * 2, 0, terrain_offset * 2));

    // Road
    geom = resman_.GetResource("Wall");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("RoadText");
    SceneNode* road = scene_.CreateNode("Road", geom, mat, text);
    road->Translate(glm::vec3(650,13,-150));
    road->Scale(glm::vec3(1300,1,30));
    road->Rotate(glm::angleAxis(glm::pi<float>(), glm::vec3(0, 0, 1)));

    // River
    geom = resman_.GetResource("Wall");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("WaterText");
    SceneNode* river = scene_.CreateNode("River", geom, mat, text);
    river->Translate(glm::vec3(650, 10, 175));
    river->Scale(glm::vec3(1300, 1, 70));
    //river->Rotate(glm::angleAxis(glm::pi<float>() / 2, glm::vec3(0, 1, 0)));
    river->Rotate(glm::angleAxis(glm::pi<float>(), glm::vec3(0, 0, 1)));

    // Vertex that follows camera
    geom = resman_.GetResource("CameraVertex");
    mat = resman_.GetResource("ObjectMaterial");
    SceneNode* cam_vertex = scene_.CreateNode("CameraVertex", geom, mat);

    // Setup drawing to texture
    scene_.SetupDrawToTexture();
    use_screen_space_effects_ = false;

    // -- Camera --
    camera_.SetPosition(glm::vec3(0, 50, 0)); // Initialize to UI pos
    camera_.UpdateYPos();

    // -- Skybox --
    geom = resman_.GetResource("SkyboxMesh");
    mat = resman_.GetResource("SkyboxProg");
    text = resman_.GetResource("SkyboxText");
    SceneNode* node = new Skybox("skybox", geom, mat, text);
    scene_.AddNode(node);

    node->Translate(glm::vec3(0, 0, 0));

    // -- UI --
    SummonUI("MainMenu", "MainText");
    SummonUI("LoseScreen", "LoseText");
    SummonUI("WinScreen", "WinText");

    // -- Ghost --
    SummonGhost("Ghost1", glm::vec3(1600, 35, 1570));

    // -- Car -- 
    SummonCar("Car", glm::vec3(-200, 100, -150));

    // -- Border --
    // Border Rock Wall
    SummonPlane("TreeBorder1", "RockTexture", glm::vec3(690, 0, -250), glm::vec3(970, 1, 40), 0);
    SummonPlane("TreeBorder2", "RockTexture", glm::vec3(-250, 0, 690), glm::vec3(970, 1, 40), 90);
    SummonPlane("TreeBorder3", "RockTexture", glm::vec3(690, 0, 1650), glm::vec3(970, 1, 40), 180);
    SummonPlane("TreeBorder4", "RockTexture", glm::vec3(1650, 0, 690), glm::vec3(970, 1, 40), 270);

    // -- Instanced Objects --
    const std::vector<boundingArea> posesToIgnore{
        // road
        boundingArea{-300, 1700, -200, -100},
        // river
        boundingArea{-300, 1700, 90, 250},
        // cabin
        // ruins
    };
    SummonInstancedObjects("TreeInstance1", "Tree1", "TreeTexture1", 100, glm::vec3(3, 3, 3), posesToIgnore, 100);
    SummonInstancedObjects("TreeInstance2", "Tree2", "TreeTexture2", 100, glm::vec3(9, 9, 9), posesToIgnore, 107);

    const std::vector<boundingArea> rockPosesToIgnore{
        // road and everything north of it
        boundingArea{-300, 1700, -200, 1700},
    };
    SummonInstancedObjects("RockInstance1", "Rock_1", "Rock_1Texture", 50, glm::vec3(0.2f, 0.2f, 0.2f), rockPosesToIgnore, 101);
    SummonInstancedObjects("RockInstance2", "Rock_2", "Rock_2Texture", 50, glm::vec3(1, 1, 1), rockPosesToIgnore, 102);
    SummonInstancedObjects("RockInstance3", "Rock_3", "Rock_3Texture", 50, glm::vec3(2, 2, 2), rockPosesToIgnore, 103, glm::vec3(0,0,-70));

    // Only summon on top right of map
    const std::vector<boundingArea> gravestonePosesToIgnore{
        // left of map
        boundingArea{400, 1700, -300, 1700},
        // bottom of map
        boundingArea{-300, 1700, -300, 300}
    };
    SummonInstancedObjects("GraveInstance1", "Gravestone", "GravestoneTexture", 100, glm::vec3(30, 30, 30), gravestonePosesToIgnore, 104);

    // -- Animated Trees --
    SummonTree("Tree1", glm::vec3(-50, 0, -50));
    SummonTree("Tree2", glm::vec3(-100, 0, 50));
    SummonTree("Tree3", glm::vec3(50, 0, -100));

    // -- Cabin --
    SummonCabin("Cabin", glm::vec3(1000, 0, 0), 180);

    // -- Signs --
    SummonSign("Sign1", glm::vec3(-150, 0, -80), 210);

    // -- Fences --
    // Fences right of car
    for (int i = 0; i < 2; ++i) {
        SummonFence("Fence" + std::to_string(i + 3), glm::vec3(-240 - 20 * i, 0, -120));
    }

    // Fences left of car
    for (int i = 0; i < 40; ++i) {
		SummonFence("Fence" + std::to_string(i+3), glm::vec3(-160 + 20*i, 0, -120));
	}

    /*
    geom = resman_.GetResource("StoneWallBent");
    mat = resman_.GetResource("LitTextureShader");
    text = resman_.GetResource("RuinTex");
    sWallBent_ = scene_.CreateNode("StoneWallBent", geom, mat, text);
    sWallBent_->Scale(glm::vec3(0.2, 0.2, 0.2));
    sWallBent_->Translate(glm::vec3(-200, 30, -200));
    sWallBent_->UpdateYPos(terrain_grid_, 0);*/

    // Tree Border
    /*for (int i = 0; i < 10; ++i) {
        std::cout << i << std::endl;
        SetupTree("TreeBorder" + std::to_string(i) + "A");
        SceneNode* treeA = scene_.GetNode("TreeBorder" + std::to_string(i) + "A_branch0");
        treeA->Scale(glm::vec3(5, 5, 5));
        treeA->Translate(glm::vec3(-230 + i * 200, 0, -230));
    }*/

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

void Game::SummonFence(std::string name, glm::vec3 position, float rotation) {
    Resource* geom = resman_.GetResource("Fence");
    Resource* mat = resman_.GetResource("LitTextureShader");
    Resource* text = resman_.GetResource("FenceTexture");
    SceneNode* node = scene_.CreateNode(name, geom, mat, text);
    node->Scale(glm::vec3(30, 30, 30));
    node->Translate(position);
    node->Rotate(glm::angleAxis(glm::radians(rotation), glm::vec3(0, 1, 0)));
    node->UpdateYPos(terrain_grid_, 1);

    Entity entity(20.0f, 25.0f, 5.0f, camera_.clampToGround(glm::vec3(position.x, 50, position.z + 15), -3));
    entities.push_back(entity);
}

void Game::SummonCar(std::string name, glm::vec3 position, float rotation) {
    Resource* geom = resman_.GetResource("Car");
    Resource* mat = resman_.GetResource("LitTextureShader");
    Resource* text = resman_.GetResource("CarTexture");
    SceneNode* node = scene_.CreateNode(name, geom, mat, text);
    node->Scale(glm::vec3(5, 5, 5));
    node->Translate(position);
    node->Rotate(glm::angleAxis(glm::radians(rotation), glm::vec3(0, 1, 0)));
    node->UpdateYPos(terrain_grid_, 8);

    Entity entity(30.0f, 25.0f, 18.0f, camera_.clampToGround(glm::vec3(position.x, 50, position.z), -3));
    entities.push_back(entity);
}

void Game::SummonUI(std::string name, std::string texture) {
    Resource* geom = resman_.GetResource("UI");
    Resource* mat = resman_.GetResource("TextureShader");
    Resource* text = resman_.GetResource(texture);
    SceneNode* node = scene_.CreateNode(name, geom, mat, text);

    node->Scale(glm::vec3(160, 100, 90));
    node->SetOrientation(glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0)));
    node->Rotate(glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1)));
    node->Translate(glm::vec3(0, 35, 200));
}

void Game::SummonGhost(std::string name, glm::vec3 position) {
    Resource* geom = resman_.GetResource("Ghost");
    Resource* mat = resman_.GetResource("LitTextureShader");
    Resource* text = resman_.GetResource("ClothTexture");

    ghost = new Ghost(name, geom, mat, text);
    ghost->Scale(glm::vec3(0.3, 0.3, 0.3));
    ghost->Translate(position);
    scene_.AddNode(ghost);
}

void Game::SummonInstancedObjects(std::string name, std::string geometry, std::string texture, int amount, glm::vec3 scale, std::vector<boundingArea> posesToIgnore, int seed, glm::vec3 offset) {
    Resource* geom = resman_.GetResource(geometry);
    Resource* mat = resman_.GetResource("LitTextureInstanceShader");
    Resource* text = resman_.GetResource(texture);

    srand(seed);

    std::vector<glm::vec3> positions;
    positions.reserve(amount);

    std::vector<glm::quat> orientations;
    orientations.reserve(amount);

    std::vector<glm::vec3> scales;
    scales.reserve(amount);

    int j = 0;
    for (int i = 0; i < amount; ++i) {
        if ((120 * i) % 1800 >= 1800 - 120) {
            j++;
        }

        float randomX = (rand() % (261)) - 130;
        float randomZ = (rand() % (261)) - 130;
        float xpos = std::min(std::max(-150 + ((120 * i) % 1800) + randomX, -225.0f), 1625.0f);
        float zpos = std::min(std::max(-170 + (60 * j) + randomZ, -215.0f), 1591.0f);

        if (!posesToIgnore.empty()) {
            bool ignore = false;
            for (const auto& box : posesToIgnore) {
                if (xpos + offset.x < box.maxx && xpos + offset.x > box.minx && zpos + offset.z > box.minz && zpos + offset.z < box.maxz) {
                    ignore = true;
                    break;
                }
            }
            if (ignore)
                continue;
		}

        orientations.push_back(glm::angleAxis(glm::radians(0.0f), glm::vec3(0, 1, 0)));
        scales.push_back(glm::vec3(scale));
        positions.push_back(camera_.clampToGround(glm::vec3(xpos, 50, zpos), -3));

        Entity entity(5.0f, 25.0f, 5.0f, camera_.clampToGround(glm::vec3(xpos, 50, zpos), -3));
        entities.push_back(entity);
    }

    InstancedObject* objects = new InstancedObject(name, geom, mat, positions, scales, orientations, text);
    scene_.AddNode(objects);

    srand(time(NULL));
}

void Game::SummonTree(std::string name, glm::vec3 position, float rotation) {
    SetupTree(name);
    SceneNode* node = scene_.GetNode(name + "_branch0");
    node->Scale(glm::vec3(5, 5, 5));
    node->Translate(position);
    node->Rotate(glm::angleAxis(glm::radians(rotation), glm::vec3(0, 1, 0)));
    node->UpdateYPos(terrain_grid_, 8);
}

void Game::SummonCabin(std::string name, glm::vec3 position, float rotation) {
    Resource* geom = resman_.GetResource("Cabin");
    Resource* mat = resman_.GetResource("LitTextureShader");
    Resource* text = resman_.GetResource("CabinTexture");
    SceneNode* node = scene_.CreateNode(name, geom, mat, text);
    node->Scale(glm::vec3(30, 30, 30));
    node->Translate(position);
    node->Rotate(glm::angleAxis(glm::radians(rotation), glm::vec3(0, 1, 0)));
    node->UpdateYPos(terrain_grid_, 0);
}

void Game::SummonSign(std::string name, glm::vec3 position, float rotation) {
    Resource* geom = resman_.GetResource("SignPost");
    Resource* mat = resman_.GetResource("LitTextureShader");
    Resource* text = resman_.GetResource("SignTexture");
    SceneNode* node = scene_.CreateNode(name, geom, mat, text);
    node->Scale(glm::vec3(8, 8, 8));
    node->Translate(position);
    node->Rotate(glm::angleAxis(glm::radians(rotation), glm::vec3(0, 1, 0)));
    node->UpdateYPos(terrain_grid_, 8);

    Entity entity(10.0f, 25.0f, 10.0f, camera_.clampToGround(glm::vec3(position.x - 10, 50, position.y), -3));
    entities.push_back(entity);
}

void Game::SummonPlane(std::string name, std::string texture, glm::vec3 position, glm::vec3 scale, float rotation) {
    Resource* geom = resman_.GetResource("Wall");
    Resource* mat = resman_.GetResource("LitTextureShader");
    Resource* text = resman_.GetResource(texture);
    SceneNode* node = scene_.CreateNode(name, geom, mat, text);
    node->SetScale(scale);
    node->SetPosition(position);
    node->SetOrientation(glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0)));
    node->Rotate(glm::angleAxis(glm::radians(180.0f), glm::vec3(1, 0, 0)));
    node->Rotate(glm::angleAxis(glm::radians(rotation), glm::vec3(0, 0, 1)));
    node->UpdateYPos(terrain_grid_, -6);
}

void Game::MainLoop(void){
    glfwSwapInterval(0);
    const char* ssShaders[] = {"None",
                               "NightVisionShader",
                               "WaveringShader",
                               "PixelatedShader",
                               "DrunkShader",
                               "BlurShader",
                               "BloodyShader"
                               };

    const char* filepath = AUDIO_DIRECTORY "/Agoraphobia.mp3";
    HSAMPLE gameplaySample = BASS_SampleLoad(FALSE, filepath, 0, 0, 3, BASS_SAMPLE_OVER_POS);
    if (gameplaySample == 0) {
        MessageBox(NULL, "Failed to load sound file!", "Error", MB_OK | MB_ICONERROR);
        BASS_Free();
        throw ("ERROR: BASS FAILED TO LOAD SOUND FILE" + std::string(filepath));
    }

    const char* filepath2 = AUDIO_DIRECTORY "/Aberrant.mp3";
    HSAMPLE menuSample = BASS_SampleLoad(FALSE, filepath2, 0, 0, 3, BASS_SAMPLE_OVER_POS);
    if (menuSample == 0) {
        MessageBox(NULL, "Failed to load sound file!", "Error", MB_OK | MB_ICONERROR);
        BASS_Free();
        throw ("ERROR: BASS FAILED TO LOAD SOUND FILE" + std::string(filepath));
    }

    HCHANNEL gameplayChannel = BASS_SampleGetChannel(gameplaySample, FALSE);
    BASS_ChannelSetAttribute(gameplayChannel, BASS_ATTRIB_VOL, 0.2f);

    float menuMusicVolume = INITIAL_MENU_MUSIC_VOLUME;
    HCHANNEL menuChannel = BASS_SampleGetChannel(menuSample, FALSE);
    BASS_ChannelSetAttribute(menuChannel, BASS_ATTRIB_VOL, menuMusicVolume);

    // Loop while the user did not close the window
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window_)){
        if (BASS_ChannelIsActive(menuChannel) != BASS_ACTIVE_PLAYING && gamePhase_ == GamePhase::title)
            BASS_ChannelPlay(menuChannel, FALSE);

        double currTime = glfwGetTime();
        double deltaTime = currTime - lastTime;
        lastTime = currTime;

        if (gamePhase_ != GamePhase::title && BASS_ChannelIsActive(menuChannel) == BASS_ACTIVE_PLAYING) {
            menuMusicVolume -= static_cast<float>(deltaTime) * (INITIAL_MENU_MUSIC_VOLUME / 6.0f);
            if (menuMusicVolume > 0.0f)
                BASS_ChannelSetAttribute(menuChannel, BASS_ATTRIB_VOL, menuMusicVolume);
            else
                BASS_ChannelStop(menuChannel);
        }

        if (gamePhase_ == GamePhase::gameplay) {
            checkKeys(deltaTime);

            scene_.Update(&camera_, deltaTime, gamePhase_);
        }

        // Move invisible camera vertex to the camera's current position
        SceneNode* cam_vertex = scene_.GetNode("CameraVertex");
        cam_vertex->SetPosition(camera_.GetPosition() - glm::vec3(0,3.5,0));
        cam_vertex->SetOrientation(camera_.GetOrientation());

        //check if contact with player has been made
        if (ghostContact()) {
            if (hp <= 0) {
                gamePhase_ = GamePhase::gameLost;
                camera_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
                camera_.SetPosition(glm::vec3(0, 50, 0)); // Initialize to UI pos
                camera_.UpdateYPos();
                // don't render sse on lose screen
                use_screen_space_effects_ = false;
            }
            else if (hp == 2) {
                use_screen_space_effects_ = true;
                scene_.bloodFactor = 0.1f;
                screen_space_effect_index_ = 6;
            }
            else if (hp == 1) {
                use_screen_space_effects_ = true;
                scene_.bloodFactor = 0.2f;
                screen_space_effect_index_ = 6;
            }
        }

        //std::cout << camera_.GetPosition().x << " " << camera_.GetPosition().z << std::endl;

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
            
            scene_.DrawToTexture(&camera_, gamePhase_);
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

        if (BASS_ChannelIsActive(gameplayChannel) != BASS_ACTIVE_PLAYING && gamePhase_ == GamePhase::gameplay && BASS_ChannelIsActive(menuChannel) != BASS_ACTIVE_PLAYING)
            BASS_ChannelPlay(gameplayChannel, FALSE);
    }

    BASS_ChannelFree(menuChannel);
    BASS_ChannelFree(gameplayChannel);
    BASS_ChannelFree(oofChannel_);
    BASS_ChannelFree(deathChannel_);
    BASS_SampleFree(menuSample);
    BASS_SampleFree(gameplaySample);
    BASS_Free();
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
            game->camera_.SetPosition(glm::vec3(-190, 50, -120));
            game->camera_.UpdateYPos();
        }
        else if (game->gamePhase_ == gameLost || game->gamePhase_ == gameWon) {
            // Quit game now that the game is over
            glfwSetWindowShouldClose(window, true);
        }
        else {
            std::cout << game->camera_.GetPosition().x << " " << game->camera_.GetPosition().z << std::endl;
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
    bool isGKeyPressed = glfwGetKey(window_, GLFW_KEY_G) == GLFW_PRESS; // Debug: Print camera coordinates
    bool isHKeyPressed = glfwGetKey(window_, GLFW_KEY_H) == GLFW_PRESS; // Debug: Allow player to traverse river at their position

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
    if (isGKeyPressed) {
        std::cout << camera_.GetPosition().x << std::endl;
        std::cout << camera_.GetPosition().y << std::endl;
        std::cout << camera_.GetPosition().z << std::endl;
    }
    if (isHKeyPressed) {
        camera_.CreateRiverPath();

        // TODO: Place log geometry



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
bool Game::ghostContact() {
    const std::pair<float, float> possibleSpawns[] = {
        std::pair<float, float>(-199.0f, -235.0f),
        std::pair<float, float>(1620.0f, -235.0f),
        std::pair<float, float>(1622.0f, 1611.0f),
        std::pair<float, float>(-225.0f, 1630.0f)
    };

    

    //player contacted
    if (ghost->getContacted() == true) {
        hp -= 1;

        if (hp > 0) {
            BASS_ChannelPlay(oofChannel_, FALSE);
        }
        else {
            BASS_ChannelPlay(deathChannel_, FALSE);
        }

        glm::vec3 playerPos = camera_.GetPosition();

        float farthestDistance = 0;
        size_t positionIndex = 0;
        for (size_t i = 0; i < 4; ++i) {
            const auto& pair = possibleSpawns[i];
            glm::vec3 potentialGhostSpawn(pair.first, 35.0f, pair.second);

            if (glm::length(potentialGhostSpawn - playerPos) > farthestDistance) {
                farthestDistance = glm::length(potentialGhostSpawn - playerPos);
                positionIndex = i;
            }
        }

        glm::vec3 ghostSpawnPoint(possibleSpawns[positionIndex].first, 35, possibleSpawns[positionIndex].second);

        //warp ghost to random position
        ghost->SetPosition(ghostSpawnPoint);

        //set contact to false
        ghost->setContacted(false);

        return true;
    }

    return false;
}

void Game::checkEntityCollision() {

    camera_.updateBoundingBox();

    //loop through entities
    for(const auto& entity : entities) {
        bool collision = entity.checkPlayerCollision(&camera_);
        
        //if colliding with an entity
        if (collision) {
            camera_.SetPosition(originalPos);
            camera_.updateBoundingBox();
        }
    }

    originalPos = camera_.GetPosition();

}

} // namespace game
