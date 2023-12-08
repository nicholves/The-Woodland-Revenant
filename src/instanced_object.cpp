#include "instanced_object.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>

#define RGB(A, B, C, D) 255 / A, 255 / B, 255 / C, 255 / D

namespace game {
   InstancedObject::InstancedObject(const std::string name, const Resource* geometry, const Resource* material, const std::vector<glm::vec3>& instancePositions,
       const std::vector<glm::vec3>& instanceScales, const std::vector<glm::quat>& instanceOrientations, const Resource* texture)  
       : Renderable(name) {
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

        blending_ = false;

        instance_count_ = instancePositions.size();

        assert(instanceOrientations.size() == instancePositions.size() && instancePositions.size() == instanceScales.size());
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        glm::mat4 *transforms = new glm::mat4[instance_count_];
        CalculateTransforms(transforms, instancePositions, instanceScales, instanceOrientations);

        glBufferData(GL_ARRAY_BUFFER, instance_count_ * sizeof(glm::mat4), transforms, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &VAO);

        delete[] transforms;
    }

   void InstancedObject::Update(void) {
       // do nothing
   }

   void InstancedObject::Draw(Camera* camera) {
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

       // Set globals for camera
       camera->SetupShader(material_);

       // Set world matrix and other shader input variables
       SetupShader(material_);

       glBindVertexArray(VAO);

       // Draw geometry
       if (mode_ == GL_POINTS) {
           glDrawArraysInstanced(mode_, 0, size_, instance_count_);
       }
       else {
           glDrawElementsInstanced(mode_, size_, GL_UNSIGNED_INT, 0, instance_count_);
       }

       glBindVertexArray(0);
   }

   void InstancedObject::CalculateTransforms(glm::mat4* arr, const std::vector<glm::vec3>& instancePositions,
       const std::vector<glm::vec3>& instanceScales, const std::vector<glm::quat>& instanceOrientations) const {
       assert(instanceOrientations.size() == instancePositions.size() && instancePositions.size() == instanceScales.size());

       std::vector<glm::mat4> result;
       result.reserve(instancePositions.size());

       for (int i = 0; i < instancePositions.size(); ++i) {
           arr[i] = CalculateTransform(instancePositions[i], instanceScales[i], instanceOrientations[i]);
       }
   }

   void InstancedObject::SetupShader(GLuint program) {
       glBindVertexArray(VAO);
       glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_);

       // Set attributes for shaders (common elements)
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


       glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
       GLint instance_tranforms_att = glGetAttribLocation(program, "instanceMatrix");
       // have to make four vec4s cause vec4 is the max alowed by opengl it seems
       glVertexAttribPointer(instance_tranforms_att + 0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0));
       glEnableVertexAttribArray(instance_tranforms_att);

       glVertexAttribPointer(instance_tranforms_att + 1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
       glEnableVertexAttribArray(instance_tranforms_att + 1);

       glVertexAttribPointer(instance_tranforms_att + 2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
       glEnableVertexAttribArray(instance_tranforms_att + 2);

       glVertexAttribPointer(instance_tranforms_att + 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
       glEnableVertexAttribArray(instance_tranforms_att + 3);

       glVertexAttribDivisor(instance_tranforms_att + 0, 1);
       glVertexAttribDivisor(instance_tranforms_att + 1, 1);
       glVertexAttribDivisor(instance_tranforms_att + 2, 1);
       glVertexAttribDivisor(instance_tranforms_att + 3, 1);

       glBindVertexArray(0);

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

   glm::mat4 InstancedObject::CalculateTransform(const glm::vec3& position_, const glm::vec3& scale_, const glm::quat& orientation_) {
       glm::mat4 scaling = glm::scale(glm::mat4(1.0), scale_);
       glm::mat4 rotation = glm::mat4_cast(orientation_);
       glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);
       glm::mat4 transf = translation * rotation * scaling;

       return transf;
   }
}