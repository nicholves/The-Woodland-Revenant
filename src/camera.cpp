#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

#include "camera.h"

#define _USE_MATH_DEFINES
#include <math.h>
#define FLASHLIGHT_ANGLE_DEGREES 25.0f

// smaller number falls off faster with angle
#define LIGHT_FALLOFF_RATE 0.6f

// bigger number falls off faster with distance
#define DISTANCE_FACTOR 0.0001f

namespace game {

    Camera::Camera(void) {
    }


    Camera::~Camera() {
    }


    glm::vec3 Camera::GetPosition(void) const {

        return position_;
    }


    glm::quat Camera::GetOrientation(void) const {

        return orientation_;
    }


    void Camera::SetPosition(glm::vec3 position) {

        position_ = position;
    }


    void Camera::SetOrientation(glm::quat orientation) {

        orientation_ = orientation;
    }

    void Camera::SetTerrainGrid(std::vector<std::vector<float>> grid) {
        terrain_grid_ = grid;
    }

    void Camera::SetImpassableCells(std::vector<std::vector<bool>> grid) {
        impassable_cells_ = grid;
    }

    void Camera::Translate(glm::vec3 trans) {
        constexpr float sizeOfQuad = 0.1f;
        const int coord_offset = 300; // This is to avoid negative indices, seems to be the right value

        float xpos = position_.x + trans.x;
        float zpos = position_.z + trans.z;
        
        xpos = std::max(std::min(xpos, 1645.0f), -245.0f);
        zpos = std::max(std::min(zpos, 1645.0f), -245.0f);

        trans.x = xpos - position_.x;
        trans.z = zpos - position_.z;

        // Get the lower x and z value of the cell for the terrain grid (corresponds to the coordinates of the cell in the impassable cell grid)
        int x1 = static_cast<int>(glm::floor((position_.x + trans.x + coord_offset) * sizeOfQuad));
        int z1 = static_cast<int>(glm::floor((position_.z + trans.z + coord_offset) * sizeOfQuad));

        if (impassable_cells_[x1/2][z1/2]) {
            return;
        }

        position_ += trans;
        UpdateYPos();
    }

    glm::vec3 Camera::clampToGround(glm::vec3 pos, float offset) const {
        constexpr float sizeOfQuad = 0.1f;
        constexpr int coord_offset = 300; // This is to avoid negative indices, seems to be the right value
        constexpr float height_scalar = 25.0f;

        // Get indices in the grid for all 4 points
        int x1 = static_cast<int>(glm::floor((pos.x + coord_offset) * sizeOfQuad));
        int x2 = static_cast<int>(glm::ceil((pos.x + coord_offset) * sizeOfQuad));
        int z1 = static_cast<int>(glm::floor((pos.z + coord_offset) * sizeOfQuad));
        int z2 = static_cast<int>(glm::ceil((pos.z + coord_offset) * sizeOfQuad));

        if (terrain_grid_.size() <= std::max(z1, z2) / 2 || terrain_grid_[0].size() <= std::max(x1, x2) / 2)
            return pos;

        glm::vec3 p1 = glm::vec3(x1, terrain_grid_[z1 / 2][x1 / 2], z1);
        glm::vec3 p2 = glm::vec3(x1, terrain_grid_[z1 / 2][x2 / 2], z2);
        glm::vec3 p3 = glm::vec3(x2, terrain_grid_[z2 / 2][x1 / 2], z1);
        glm::vec3 p4 = glm::vec3(x2, terrain_grid_[z2 / 2][x2 / 2], z2);

        // Interpolate the y position

        float s = (pos.x + coord_offset) * sizeOfQuad - x1;
        float t = (pos.z + coord_offset) * sizeOfQuad - z1;

        pos.y = ((1 - t) * ((1 - s) * p1 + s * p2) + t * ((1 - s) * p3 + s * p4)).y * height_scalar + offset;

        return pos;
    }

    void Camera::UpdateYPos() {
        constexpr float sizeOfQuad = 0.1f;
        const int coord_offset = 300; // This is to avoid negative indices, seems to be the right value
        const float player_height = 20.0f;
        const float height_scalar = 25.0f;

        // Get indices in the grid for all 4 points
        int x1 = static_cast<int>(glm::floor((position_.x + coord_offset) * sizeOfQuad));
        int x2 = static_cast<int>(glm::ceil((position_.x + coord_offset) * sizeOfQuad));
        int z1 = static_cast<int>(glm::floor((position_.z + coord_offset) * sizeOfQuad));
        int z2 = static_cast<int>(glm::ceil((position_.z + coord_offset) * sizeOfQuad));

        if (terrain_grid_.size() <= std::max(z1, z2) / 2 || terrain_grid_[0].size() <= std::max(x1, x2) / 2)
            return;

        glm::vec3 p1 = glm::vec3(x1, terrain_grid_[z1 / 2][x1 / 2], z1);
        glm::vec3 p2 = glm::vec3(x1, terrain_grid_[z1 / 2][x2 / 2], z2);
        glm::vec3 p3 = glm::vec3(x2, terrain_grid_[z2 / 2][x1 / 2], z1);
        glm::vec3 p4 = glm::vec3(x2, terrain_grid_[z2 / 2][x2 / 2], z2);

        // Interpolate the y position

        float s = (position_.x + coord_offset) * sizeOfQuad - x1;
        float t = (position_.z + coord_offset) * sizeOfQuad - z1;

        position_.y = ((1 - t) * ((1 - s) * p1 + s * p2) + t * ((1 - s) * p3 + s * p4)).y * height_scalar + player_height;
    }

    void Camera::MoveForward(float amount) {
        Translate(amount * GetForward());
    }


    void Camera::Rotate(glm::quat rot) {
        // apply this rotation to the camera's current orientation
        orientation_ = rot * orientation_;
        orientation_ = glm::normalize(orientation_);
    }


    glm::vec3 Camera::GetForward(void) const {

        glm::vec3 current_forward = orientation_ * forward_;
        return -glm::normalize(current_forward); // Return -forward since the camera coordinate system points in the opposite direction
    }

    glm::vec3 Camera::GetStraigth(void) const {
        glm::vec3 result = GetForward();
        result.y = 0.0f;

        return glm::normalize(result);
    }


    glm::vec3 Camera::GetSide(void) const {
        // how do you get the side vector?
        glm::vec3 current_side = orientation_ * side_;
        return current_side;
    }


    glm::vec3 Camera::GetUp(void) const {
        glm::vec3 current_up = glm::cross(GetSide(), GetForward());
        return current_up;
    }


    void Camera::Pitch(float angle) {
        glm::quat rotation = glm::angleAxis(angle, GetSide()); // Rotate around Side For Pitch 
        glm::quat new_orientation = rotation * orientation_;
        new_orientation = glm::normalize(new_orientation);
        if ((new_orientation * forward_).y <= -0.999 || (new_orientation * forward_).y >= 0.999) {
            return;
        }
        orientation_ = new_orientation;
    }


    void Camera::Yaw(float angle) {
        glm::quat rotation = glm::angleAxis(angle, GetUp()); // Rotate around Up For Yaw 
        orientation_ = rotation * orientation_;
        orientation_ = glm::normalize(orientation_);
    }


    void Camera::Roll(float angle) {
        glm::quat rotation = glm::angleAxis(angle, GetForward()); // Rotate around Forward For Roll 
        orientation_ = rotation * orientation_; // update the orientation
        orientation_ = glm::normalize(orientation_);
    }


    void Camera::SetView(glm::vec3 position, glm::vec3 look_at, glm::vec3 up) {

        // Store initial forward and side vectors
        // See slide in "Camera control" for details
        // probably will not be much used (directly setting view a rare occurrence in games)
        forward_ = look_at - position;
        forward_ = -glm::normalize(forward_);
        side_ = glm::cross(up, forward_);
        side_ = glm::normalize(side_);

        // Reset orientation and position of camera
        position_ = position;
        orientation_ = glm::quat();
    }


    void Camera::SetProjection(GLfloat fov, GLfloat near, GLfloat far, GLfloat w, GLfloat h) {

        // Set projection based on field-of-view
        float top = static_cast<float>(tan((fov / 2.0) * (glm::pi<float>() / 180.0)) * near);
        float right = top * w / h;
        projection_matrix_ = glm::frustum(-right, right, -top, top, near, far);
    }


    void Camera::SetupShader(GLuint program) {

        // Update view matrix
        SetupViewMatrix();

        // Set view matrix in shader
        GLint view_mat = glGetUniformLocation(program, "view_mat");
        glUniformMatrix4fv(view_mat, 1, GL_FALSE, glm::value_ptr(view_matrix_));

        // Set projection matrix in shader
        GLint projection_mat = glGetUniformLocation(program, "projection_mat");
        glUniformMatrix4fv(projection_mat, 1, GL_FALSE, glm::value_ptr(projection_matrix_));

        GLint flashlight_pos = glGetUniformLocation(program, "flashlight_pos");
        GLint flashlight_dir = glGetUniformLocation(program, "flashlight_dir");
        GLint cutoff = glGetUniformLocation(program, "cutoff");
        GLint falloff_rate = glGetUniformLocation(program, "falloffRate");
        GLint distanceFactor = glGetUniformLocation(program, "distanceFactor");

        if (flashlight_pos && flashlight_dir && falloff_rate) {
            //glm::vec3 flashlight_offset = (GetUp() * -10.0f) + (GetSide() * 5.0f) + (GetForward() * 0.01f);
            glm::vec3 flashlight_pos_vec = position_;// +flashlight_offset;

            glm::vec3 flashlight_dir_vec = GetForward();

            glUniform3fv(flashlight_pos, 1, glm::value_ptr(flashlight_pos_vec));
            glUniform3fv(flashlight_dir, 1, glm::value_ptr(flashlight_dir_vec));

            glUniform1f(cutoff, cos(FLASHLIGHT_ANGLE_DEGREES * (static_cast<float>(M_PI) / 180.0f)));

            // Flashlight Fade out Rate with angle
            glUniform1f(falloff_rate, LIGHT_FALLOFF_RATE);

            // Flashlight Fade out Rate with Distance
            glUniform1f(distanceFactor, DISTANCE_FACTOR);
        }

        glm::vec3 camera_pos = GetPosition();
        GLint camera_position = glGetUniformLocation(program, "camera_position");
        glUniform3f(camera_position, camera_pos.x, camera_pos.y, camera_pos.z);
    }

    void Camera::SetupShaderSkybox(GLuint program) {
        // Update view matrix
        SetupViewMatrix();

        // Set view matrix in shader
        GLint view_mat = glGetUniformLocation(program, "view_mat");
        glUniformMatrix4fv(view_mat, 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(view_matrix_))));

        // Set projection matrix in shader
        GLint projection_mat = glGetUniformLocation(program, "projection_mat");
        glUniformMatrix4fv(projection_mat, 1, GL_FALSE, glm::value_ptr(projection_matrix_));
    }


    void Camera::SetupViewMatrix(void) {

        //view_matrix_ = glm::lookAt(position, look_at, up);

        // Get current vectors of coordinate system
        // [side, up, forward]
        // See slide in "Camera control" for details
        glm::vec3 current_forward = orientation_ * forward_;
        glm::vec3 current_side = orientation_ * side_;
        glm::vec3 current_up = glm::cross(current_forward, current_side);
        current_up = glm::normalize(current_up);

        // Initialize the view matrix as an identity matrix
        view_matrix_ = glm::mat4(1.0);

        // Copy vectors to matrix
        // Add vectors to rows, not columns of the matrix, so that we get
        // the inverse transformation
        // Note that in glm, the reference for matrix entries is of the form
        // matrix[column][row]
        view_matrix_[0][0] = current_side[0]; // First row
        view_matrix_[1][0] = current_side[1];
        view_matrix_[2][0] = current_side[2];
        view_matrix_[0][1] = current_up[0]; // Second row
        view_matrix_[1][1] = current_up[1];
        view_matrix_[2][1] = current_up[2];
        view_matrix_[0][2] = current_forward[0]; // Third row
        view_matrix_[1][2] = current_forward[1];
        view_matrix_[2][2] = current_forward[2];

        // Create translation to camera position
        glm::mat4 trans = glm::translate(glm::mat4(1.0), -position_);

        // Combine translation and view matrix in proper order
        view_matrix_ *= trans;
    }

    // Function to update the camera's bounding box
    void Camera::updateBoundingBox() {
        // Set the bounding box around the camera position
        boundingBox.min = position_ - glm::vec3(1.0f, 10.0f, 1.0f);  // Assuming a unit cube
        boundingBox.max = position_ + glm::vec3(1.0f, 10.0f, 1.0f);
    }

    BoundingBox Camera::getBBox() {
        return boundingBox;
    }


    int Camera::CreateRiverPath() {
        // 1: Calculate proper index in the impassible and terrain grids (x1/2 and z1/2, as used in camera.cpp)
        // 2: Using the indices, clear a path through the impassible grid and elevate the y-pos so the player "walks" over the log
        // 3: Return the real x position, which is where the log will be placed

        constexpr float sizeOfQuad = 0.1f;
        constexpr int coord_offset = 300;

        const int z1 = 20;
        const int z2 = 26;

        const float y_pos = 0.7;

        // Convert x-coord to index
        int x1 = static_cast<int>(glm::floor((position_.x + coord_offset) * sizeOfQuad));
        
        // Create a path in the impassable cells
        impassable_cells_[x1 / 2][z1] = 0;
        impassable_cells_[x1 / 2][z2] = 0;

        impassable_cells_[(x1 / 2) + 1][z1] = 0;
        impassable_cells_[(x1 / 2) + 1][z2] = 0;

        impassable_cells_[(x1 / 2) - 1][z1] = 0;
        impassable_cells_[(x1 / 2) - 1][z2] = 0;

        for (int i = z1 + 1; i <= z2; ++i) {

            // Create barrier along the path
            impassable_cells_[(x1 / 2) + 1][i] = 1;
            impassable_cells_[(x1 / 2) - 1][i] = 1;

            // Elevate y-pos
            terrain_grid_[i][(x1 / 2)] = y_pos;
            terrain_grid_[i][(x1 / 2) + 1] = y_pos;
            terrain_grid_[i][(x1 / 2) - 1] = y_pos;
        }

        // Return real x-coord (doesn't seem to be quite right)
        int x2 = static_cast<int>(glm::floor(position_.x * sizeOfQuad)) / sizeOfQuad;
        int x3 = static_cast<int>(glm::ceil(position_.x * sizeOfQuad)) / sizeOfQuad;

        return (x2 + x3)/2;
    }

} // namespace game
