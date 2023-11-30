#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "camera.h"

#define _USE_MATH_DEFINES
#include <math.h>
#define FLASHLIGHT_ANGLE_DEGREES 25.0f

// smaller number falls off faster with angle
#define LIGHT_FALLOFF_RATE 2

// bigger number falls off faster with distance
#define DISTANCE_FACTOR 0.0002f

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
        const int coord_offset = 250; // This is to avoid negative indices, seems to be the right value

        // Get the lower x and z value of the cell for the terrain grid (corresponds to the coordinates of the cell in the impassable cell grid)
        int x1 = glm::floor((position_.x + trans.x + coord_offset) * sizeOfQuad);
        int z1 = glm::floor((position_.z + trans.z + coord_offset) * sizeOfQuad);

        if (impassable_cells_[x1/2][z1/2]) {
            return;
        }

        position_ += trans;
        UpdateYPos();
    }

    void Camera::UpdateYPos() {
        constexpr float sizeOfQuad = 0.1f;
        const int coord_offset = 250; // This is to avoid negative indices, seems to be the right value
        const float player_height = 20.0f;
        const float height_scalar = 25.0f;

        // Get indices in the grid for all 4 points
        int x1 = glm::floor((position_.x + coord_offset) * sizeOfQuad);
        int x2 = glm::ceil((position_.x + coord_offset) * sizeOfQuad);
        int z1 = glm::floor((position_.z + coord_offset) * sizeOfQuad);
        int z2 = glm::ceil((position_.z + coord_offset) * sizeOfQuad);

        /*std::cout << x1 << std::endl;
        std::cout << x2 << std::endl;
        std::cout << z1 << std::endl;
        std::cout << z2 << std::endl;*/

        //std::cout << terrain_grid_[0][0] << " " << terrain_grid_[0][1] << " " << terrain_grid_[0][2] << std::endl;
        std::cout << z1 / 2 << " " << x1 / 2 << std::endl;

        glm::vec3 p1 = glm::vec3(x1, terrain_grid_[z1 / 2][x1 / 2], z1);
        glm::vec3 p2 = glm::vec3(x1, terrain_grid_[z1 / 2][x2 / 2], z2);
        glm::vec3 p3 = glm::vec3(x2, terrain_grid_[z2 / 2][x1 / 2], z1);
        glm::vec3 p4 = glm::vec3(x2, terrain_grid_[z2 / 2][x2 / 2], z2);

        /*std::cout << p1.y << std::endl;
        std::cout << p2.y << std::endl;
        std::cout << p3.y << std::endl;
        std::cout << p4.y << std::endl;*/

        // Interpolate the y position

        float s = (position_.x + coord_offset) * sizeOfQuad - x1;
        float t = (position_.z + coord_offset) * sizeOfQuad - z1;

        /*std::cout << t << std::endl;
        std::cout << s << std::endl;*/

        position_.y = ((1 - t) * ((1 - s) * p1 + s * p2) + t * ((1 - s) * p3 + s * p4)).y * height_scalar + player_height;

       

        //std::cout << position_.y << std::endl;
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
        return -current_forward; // Return -forward since the camera coordinate system points in the opposite direction
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
        orientation_ = rotation * orientation_;
        orientation_ = glm::normalize(orientation_);
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
        float top = tan((fov / 2.0) * (glm::pi<float>() / 180.0)) * near;
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

            glUniform1f(cutoff, cos(FLASHLIGHT_ANGLE_DEGREES * (M_PI / 180)));

            // Flashlight Fade out Rate with angle
            glUniform1f(falloff_rate, LIGHT_FALLOFF_RATE);

            // Flashlight Fade out Rate with Distance
            glUniform1f(distanceFactor, DISTANCE_FACTOR);
        }

        glm::vec3 camera_pos = GetPosition();
        GLint camera_position = glGetUniformLocation(program, "camera_position");
        glUniform3f(camera_position, camera_pos.x, camera_pos.y, camera_pos.z);
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

    bool Camera::getImmune() {
        return isImmune;
    }

    float Camera::getTimer() {
        return immuneTimer;
    }

    void Camera::setImmune(bool immune) {
        isImmune = immune;
    }

    void Camera::setTimer(float timer) {
        immuneTimer = timer;
    }

} // namespace game
