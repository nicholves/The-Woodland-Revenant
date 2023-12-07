#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>

#include "scene_node.h"

#define RGB(A, B, C, D) 255 / A, 255 / B, 255 / C, 255 / D



namespace game {

    SceneNode::SceneNode(const std::string name, const Resource* geometry, const Resource* material, const Resource* texture) {

        // Set name of scene node
        name_ = name;

        // Set geometry
        if (geometry->GetType() == PointSet) {
            mode_ = GL_POINTS;
        }
        else if (geometry->GetType() == Mesh) {
            mode_ = GL_TRIANGLES;
        }
        else {
            throw(std::invalid_argument(std::string("Invalid type of geometry")));
        }

        array_buffer_ = geometry->GetArrayBuffer();
        element_array_buffer_ = geometry->GetElementArrayBuffer();
        size_ = geometry->GetSize();

        // Set material (shader program)
        if (material->GetType() != Material) {
            throw(std::invalid_argument(std::string("Invalid type of material")));
        }

        material_ = material->GetResource();

        // Set texture
        if (texture) {
            texture_ = texture->GetResource();
        }
        else {
            texture_ = 0;
        }

        // Other attributes
        scale_ = glm::vec3(1.0, 1.0, 1.0);
        blending_ = false;
    }


    SceneNode::~SceneNode() {
    }


    const std::string SceneNode::GetName(void) const {

        return name_;
    }


    glm::vec3 SceneNode::GetPosition(void) const {

        return position_;
    }


    glm::quat SceneNode::GetOrientation(void) const {

        return orientation_;
    }


    glm::vec3 SceneNode::GetScale(void) const {

        return scale_;
    }


    void SceneNode::SetPosition(glm::vec3 position) {

        position_ = position;
    }


    void SceneNode::SetOrientation(glm::quat orientation) {

        orientation_ = orientation;
    }


    void SceneNode::SetScale(glm::vec3 scale) {

        scale_ = scale;
    }


    void SceneNode::Translate(glm::vec3 trans) {

        position_ += trans;
    }


    void SceneNode::Rotate(glm::quat rot) {

        orientation_ *= rot;
        orientation_ = glm::normalize(orientation_);
    }


    void SceneNode::Scale(glm::vec3 scale) {

        scale_ *= scale;
    }


    GLenum SceneNode::GetMode(void) const {

        return mode_;
    }


    GLuint SceneNode::GetArrayBuffer(void) const {

        return array_buffer_;
    }


    GLuint SceneNode::GetElementArrayBuffer(void) const {

        return element_array_buffer_;
    }


    GLsizei SceneNode::GetSize(void) const {

        return size_;
    }


    GLuint SceneNode::GetMaterial(void) const {

        return material_;
    }

    void SceneNode::SetOrbitTranslation(const glm::vec3 tr) {
        orbit_translation = tr;
    }

    void SceneNode::SetOrbitRotation(const glm::quat rot) {
        orbit_rotation = rot;
    }

    glm::vec3 SceneNode::GetOrbitTranslation(void) const {
        return orbit_translation;
    }

    glm::quat SceneNode::GetOrbitRotation(void) const {
        return orbit_rotation;
    }

    void SceneNode::SetParent(SceneNode* parent) {
        parent_ = parent;
    }

    void SceneNode::SetWindAffected(const bool wind) {
        wind_affected = wind;
    }

    glm::mat4 SceneNode::CalculateTransform(float current_time) const {
        glm::quat adjusted_orbit = orbit_rotation;
        // Adjusting orbit to handle wind as well if its wind affected
        if (wind_affected) adjusted_orbit *= glm::normalize(glm::angleAxis(glm::sin(current_time) * wind_strength, glm::vec3(0, 0, 1)));

        glm::mat4 orbit = glm::inverse(glm::translate(glm::mat4(1.0), orbit_translation)) * glm::mat4_cast(adjusted_orbit) * glm::translate(glm::mat4(1.0), orbit_translation);
        glm::mat4 scaling = glm::scale(glm::mat4(1.0), scale_);
        glm::mat4 rotation = glm::mat4_cast(orientation_);
        glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);
        glm::mat4 transf = translation * rotation * orbit * scaling;

        if (parent_) {
            transf = parent_->CalculateTransform(current_time) * transf; // Affect the transform based on the parent transform
        }

        return transf;
    }

    void SceneNode::UpdateYPos(std::vector<std::vector<float>> terrain_grid_, float object_offset) {
        constexpr float sizeOfQuad = 0.1f;
        const int coord_offset = 300; // This is to avoid negative indices, seems to be the right value
        const float height_scalar = 25.0f;

        // Get indices in the grid for all 4 points
        int x1 = static_cast<int>(glm::floor((position_.x + coord_offset) * sizeOfQuad));
        int x2 = static_cast<int>(glm::ceil((position_.x + coord_offset) * sizeOfQuad));
        int z1 = static_cast<int>(glm::floor((position_.z + coord_offset) * sizeOfQuad));
        int z2 = static_cast<int>(glm::ceil((position_.z + coord_offset) * sizeOfQuad));

        glm::vec3 p1 = glm::vec3(x1, terrain_grid_[z1 / 2][x1 / 2], z1);
        glm::vec3 p2 = glm::vec3(x1, terrain_grid_[z1 / 2][x2 / 2], z2);
        glm::vec3 p3 = glm::vec3(x2, terrain_grid_[z2 / 2][x1 / 2], z1);
        glm::vec3 p4 = glm::vec3(x2, terrain_grid_[z2 / 2][x2 / 2], z2);

        float s = (position_.x + coord_offset) * sizeOfQuad - x1;
        float t = (position_.z + coord_offset) * sizeOfQuad - z1;

        position_.y = ((1 - t) * ((1 - s) * p1 + s * p2) + t * ((1 - s) * p3 + s * p4)).y * height_scalar + object_offset;
    }

    void SceneNode::Draw(Camera* camera) {

        // Select particle blending or not
        if (blending_) {
            // Disable depth write
            glDepthMask(GL_FALSE);

            // Enable blending
            glEnable(GL_BLEND);
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Simpler form
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
        }
        else {
            // Enable z-buffer
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glDepthFunc(GL_LESS);
        }

        // Select proper material (shader program)
        glUseProgram(material_);

        // Set geometry to draw
        glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_);

        // Set globals for camera
        camera->SetupShader(material_);

        // Set world matrix and other shader input variables
        SetupShader(material_);

        // Draw geometry
        if (mode_ == GL_POINTS) {
            glDrawArrays(mode_, 0, size_);
        }
        else {
            // glDrawElementsInstanced(mode_, size_, GL_UNSIGNED_INT, 0, 200);
            glDrawElements(mode_, size_, GL_UNSIGNED_INT, 0);
        }
    }

    void SceneNode::SetBlending(bool blending) {
        blending_ = blending;
    }


    void SceneNode::Update(void) {

        // Do nothing for this generic type of scene node
    }


    void SceneNode::SetupShader(GLuint program) {


        // Set attributes for shaders
        GLint vertex_att = glGetAttribLocation(program, "vertex");
        glVertexAttribPointer(vertex_att, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(vertex_att);

        GLint normal_att = glGetAttribLocation(program, "normal");
        glVertexAttribPointer(normal_att, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(normal_att);

        GLint color_att = glGetAttribLocation(program, "color");
        glVertexAttribPointer(color_att, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(color_att);

        GLint tex_att = glGetAttribLocation(program, "uv");
        glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));
        glEnableVertexAttribArray(tex_att);

        // World transformation
        glm::mat4 scaling = glm::scale(glm::mat4(1.0), scale_);

        glm::mat4 rotation = glm::mat4_cast(orientation_);
        glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);

        glm::mat4 orbit = glm::mat4(1.0); // identity -- left out for now
        glm::mat4 transf = translation * orbit * rotation * scaling;

        // Texture
        if (texture_) {
            GLint tex = glGetUniformLocation(program, "texture_map");
            glUniform1i(tex, 0); // Assign the first texture to the map
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_); // First texture we bind
            // Define texture interpolation
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        }

        // Timer
        GLint timer_var = glGetUniformLocation(program, "timer");
        float current_time = static_cast<float>(glfwGetTime());
        glUniform1f(timer_var, (float)current_time);

        // Transformations
        GLint world_mat = glGetUniformLocation(program, "world_mat");
        glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(CalculateTransform(current_time)));

        // Specular Power
        GLint specular_power = glGetUniformLocation(program, "specular_power");
        glUniform1f(specular_power, 41);

        // Light Color
        GLint light_color = glGetUniformLocation(program, "light_color");
        glUniform4f(light_color, 1, 1, 0.4f, 1);

        // Ambient Light Color
        GLint amb_light_color = glGetUniformLocation(program, "ambient_light_color");
        glUniform4f(amb_light_color, RGB(176, 224, 230, 255));

        // Object Color
        GLint object_color = glGetUniformLocation(program, "object_color");
        glUniform3f(object_color, 0.0f, 0.7f, 0.9f);
    }

} // namespace game;
