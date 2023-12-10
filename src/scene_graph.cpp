#include <stdexcept>
#include <iostream>
#include <fstream>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene_graph.h"
#include "ghost.h"

namespace game {

    // the higher this number the blurrier the scene will get but also the bigger the performance impact
    int SceneGraph::blurrSamples = 10;

    // this should be between 0 and 1. Describes how bloody the screen gets. Bigger = more blood.
    float SceneGraph::bloodFactor = 0.1f;

    // Lower number means smaller pixels
    float SceneGraph::pixelSpacing = 0.004f;

    SceneGraph::SceneGraph(void) {

        background_color_ = glm::vec3(0.0, 0.0, 0.0);
    }


    SceneGraph::~SceneGraph() {
    }


    void SceneGraph::SetBackgroundColor(glm::vec3 color) {

        background_color_ = color;
    }


    glm::vec3 SceneGraph::GetBackgroundColor(void) const {

        return background_color_;
    }


    SceneNode* SceneGraph::CreateNode(std::string node_name, Resource* geometry, Resource* material, Resource* texture) {

        // Create scene node with the specified resources
        SceneNode* scn = new SceneNode(node_name, geometry, material, texture);

        // Add node to the scene
        node_.push_back(scn);

        return scn;
    }

    InteractableNode* SceneGraph::CreateInteractableNode(std::string node_name, Resource* geometry, Resource* material, Resource* texture) {

        // Create scene node with the specified resources
        InteractableNode* scn = new InteractableNode(node_name, geometry, material, texture);

        // Add node to list of interactable nodes
        interactable_nodes_.push_back(scn);

        // Add node to the scene
        node_.push_back((SceneNode*)scn);

        return scn;
    }


    void SceneGraph::AddNode(Renderable* node) {

        node_.push_back(node);
    }


    SceneNode* SceneGraph::GetNode(const std::string& node_name) const {

        // Find node with the specified name
        for (size_t i = 0; i < node_.size(); i++) {
            if (node_[i]->GetName() == node_name && dynamic_cast<SceneNode*>(node_[i])) {
                return dynamic_cast<SceneNode*>(node_[i]);
            }
        }
        return NULL;

    }

    Renderable* SceneGraph::getRenderable(const std::string& node_name) {
        // Find node with the specified name
        for (size_t i = 0; i < node_.size(); i++) {
            if (node_[i]->GetName() == node_name && node_[i]) {
                return node_[i];
            }
        }
        return NULL;
    }


    void SceneGraph::DeleteNode(std::string node_name) {
        // Find node with the specified name
        for (size_t i = 0; i < node_.size(); i++) {
            if (node_[i]->GetName() == node_name) {
                delete node_[i];
                node_.erase(node_.begin() + i);
            }
        }
    }

    
    std::vector<InteractableNode*> SceneGraph::GetInteractableNodes() {
        return interactable_nodes_;
    }


    std::vector<Renderable*>::const_iterator SceneGraph::begin() const {

        return node_.begin();
    }


    std::vector<Renderable*>::const_iterator SceneGraph::end() const {

        return node_.end();
    }


    void SceneGraph::Draw(Camera* camera, GamePhase gamePhase) {
        // Clear background
        glClearColor(background_color_[0],
            background_color_[1],
            background_color_[2], 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (gamePhase == title) {
            GetNode("MainMenu")->Draw(camera);
            return;
        }
        else if (gamePhase == gameLost) {
            GetNode("LoseScreen")->Draw(camera);
            return;
        }
        else if (gamePhase == gameWon) {
            GetNode("WinScreen")->Draw(camera);
            return;
        }

        // Draw all scene nodes
        for (size_t i = 0; i < node_.size(); i++) {
            if (node_[i]->GetName() == "skybox") continue;
            if (node_[i]->GetName() == "MainMenu") continue;
            if (node_[i]->GetName() == "LoseScreen") continue;
            if (node_[i]->GetName() == "WinScreen") continue;
            node_[i]->Draw(camera);
        }

        GetNode("skybox")->Draw(camera);
    }


    void SceneGraph::Update(Camera* camera, double deltaTime, GamePhase gamePhase) {
        if (gamePhase == title || gamePhase == gameLost || gamePhase == gameWon) {
			return; // Don't update anything if in UI
		}

        for (size_t i = 0; i < node_.size(); i++) {
            // Check if the current node is a Ghost
            Ghost* ghostNode = dynamic_cast<Ghost*>(node_[i]);

            if (ghostNode) {
                // If it's a Ghost, pass the camera to its Update function
                ghostNode->Update(camera, static_cast<float>(deltaTime));
            }
            else {
                node_[i]->Update();
            }

        }
    }

    void SceneGraph::SetupDrawToTexture(void) {

        // Set up frame buffer
        glGenFramebuffers(1, &frame_buffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);

        // Set up target texture for rendering
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);

        // Set up an image for the texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // Set up a depth buffer for rendering
        glGenRenderbuffers(1, &depth_buffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

        // Configure frame buffer (attach rendering buffers)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, DrawBuffers);

        // Check if frame buffer was setup successfully 
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw(std::ios_base::failure(std::string("Error setting up frame buffer")));
        }

        // Reset frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Set up quad for drawing to the screen
        static const GLfloat quad_vertex_data[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        };

        // Create buffer for quad
        glGenBuffers(1, &quad_array_buffer_);
        glBindBuffer(GL_ARRAY_BUFFER, quad_array_buffer_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_data), quad_vertex_data, GL_STATIC_DRAW);
    }


    void SceneGraph::DrawToTexture(Camera* camera, GamePhase gamePhase) {

        // Save current viewport
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        // Enable frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
        glViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

        // Enable writing to depth buffer
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);

        // Clear background
        glClearColor(background_color_[0],
            background_color_[1],
            background_color_[2], 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (gamePhase == title) {
            GetNode("MainMenu")->Draw(camera);
            return;
        }
        else if (gamePhase == gameLost) {
            GetNode("LoseScreen")->Draw(camera);
            return;
        }
        else if (gamePhase == gameWon) {
            GetNode("WinScreen")->Draw(camera);
            return;
        }

        // Draw all scene nodes
        for (size_t i = 0; i < node_.size(); i++) {
            if (node_[i]->GetName() == "skybox") continue;
            if (node_[i]->GetName() == "MainMenu") continue;
            if (node_[i]->GetName() == "LoseScreen") continue;
            if (node_[i]->GetName() == "WinScreen") continue;
            node_[i]->Draw(camera);
        }

        GetNode("skybox")->Draw(camera);

        // Enable writing to depth buffer
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);

        // Reset frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Restore viewport
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }

    void SceneGraph::ApplySSE(GLuint program) {
        // Save current viewport
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        // Enable frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
        glViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

        // Configure output to the screen
       //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        // Set up quad geometry
        glBindBuffer(GL_ARRAY_BUFFER, quad_array_buffer_);

        // Select proper material (shader program)
        glUseProgram(program);

        // Setup attributes of screen-space shader
        GLint pos_att = glGetAttribLocation(program, "position");
        glEnableVertexAttribArray(pos_att);
        glVertexAttribPointer(pos_att, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

        GLint tex_att = glGetAttribLocation(program, "uv");
        glEnableVertexAttribArray(tex_att);
        glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

        GLint ratio_var = glGetUniformLocation(program, "aspect_ratio");
        float aspect_ratio = static_cast<float>(viewport[2]) / viewport[3];
        glUniform1f(ratio_var, aspect_ratio);

        GLint blur_samples = glGetUniformLocation(program, "num_samples");
        glUniform1i(blur_samples, blurrSamples);

        GLint blood_factor = glGetUniformLocation(program, "blood_factor");
        // 1 is fully bloody. 0 is no blood
        glUniform1f(blood_factor, bloodFactor);

        GLint pixel_spacing = glGetUniformLocation(program, "pixelSpacing");
        glUniform1f(pixel_spacing, pixelSpacing);

        // Timer
        GLint timer_var = glGetUniformLocation(program, "timer");
        float current_time = static_cast<float>(glfwGetTime());
        glUniform1f(timer_var, current_time);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);

        // Draw geometry
        glDrawArrays(GL_TRIANGLES, 0, 6); // Quad: 6 coordinates

        // Reset current geometry
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }


    void SceneGraph::DisplayTexture(GLuint program) {

        // Configure output to the screen
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        // Set up quad geometry
        glBindBuffer(GL_ARRAY_BUFFER, quad_array_buffer_);

        // Select proper material (shader program)
        glUseProgram(program);

        // Setup attributes of screen-space shader
        GLint pos_att = glGetAttribLocation(program, "position");
        glEnableVertexAttribArray(pos_att);
        glVertexAttribPointer(pos_att, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

        GLint tex_att = glGetAttribLocation(program, "uv");
        glEnableVertexAttribArray(tex_att);
        glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

        GLint ratio_var = glGetUniformLocation(program, "aspect_ratio");
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float aspect_ratio = static_cast<float>(viewport[2]) / viewport[3];
        glUniform1f(ratio_var, aspect_ratio);

        GLint blur_samples = glGetUniformLocation(program, "num_samples");
        glUniform1i(blur_samples, blurrSamples);

        GLint blood_factor = glGetUniformLocation(program, "blood_factor");
        // 1 is fully bloody. 0 is no blood
        glUniform1f(blood_factor, bloodFactor);

        GLint pixel_spacing = glGetUniformLocation(program, "pixelSpacing");
        glUniform1f(pixel_spacing, pixelSpacing);

        // Timer
        GLint timer_var = glGetUniformLocation(program, "timer");
        float current_time = static_cast<float>(glfwGetTime());
        glUniform1f(timer_var, current_time);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);

        // Draw geometry
        glDrawArrays(GL_TRIANGLES, 0, 6); // Quad: 6 coordinates

        // Reset current geometry
        glEnable(GL_DEPTH_TEST);
    }


    void SceneGraph::SaveTexture(char* filename) {

        unsigned char *data = new unsigned char[FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 4];

        // Retrieve image data from texture
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
        glReadPixels(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Create file in ppm format
        // Open the file
        std::ofstream f;
        f.open(filename);
        if (f.fail()) {
            throw(std::ios_base::failure(std::string("Error opening file ") + std::string(filename)));
        }

        // Write header
        f << "P3" << std::endl;
        f << FRAME_BUFFER_WIDTH << " " << FRAME_BUFFER_HEIGHT << std::endl;
        f << "255" << std::endl;

        // Write data
        for (int i = 0; i < FRAME_BUFFER_HEIGHT; i++) {
            for (int j = 0; j < FRAME_BUFFER_WIDTH; j++) {
                for (int k = 0; k < 3; k++) {
                    int dt = data[i * FRAME_BUFFER_WIDTH * 4 + j * 4 + k];
                    f << dt << " ";
                }
            }
            f << std::endl;
        }

        // Close the file
        f.close();

        // Reset frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        delete[] data;
    }

} // namespace game
