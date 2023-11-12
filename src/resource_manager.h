#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "resource.h"

// Default extensions for different shader source files
#define VERTEX_PROGRAM_EXTENSION "_vp.glsl"
#define FRAGMENT_PROGRAM_EXTENSION "_fp.glsl"

namespace game {

    // Class that manages all resources
    class ResourceManager {

        public:
            // Constructor and destructor
            ResourceManager(void);
            ~ResourceManager();
            // Add a resource that was already loaded and allocated to memory
            void AddResource(ResourceType type, const std::string name, GLuint resource, GLsizei size);
            void AddResource(ResourceType type, const std::string name, GLuint array_buffer, GLuint element_array_buffer, GLsizei size);
            // Load a resource from a file, according to the specified type
            void LoadResource(ResourceType type, const std::string name, const char *filename);
            void LoadCustomResource(ResourceType type, const std::string name, const char* verticesFilepath, const char* facesFilepath);
            std::vector<std::vector<float>> LoadTerrainResource(ResourceType type, const std::string name, const char* terrainFilePath);

            GLuint ParseVertices(const std::string& verticesText);
            GLuint ParseFaces(const std::string& facesText, GLsizei& faceSize);
            // Get the resource with the specified name
            Resource *GetResource(const std::string name) const;

            // Methods to create specific resources
            // Create the geometry for a torus and add it to the list of resources
            void CreateTorus(std::string object_name, float loop_radius = 0.6, float circle_radius = 0.2, int num_loop_samples = 90, int num_circle_samples = 30);
			// Create the geometry for a sphere
			void CreateSphere(std::string object_name, float radius = 0.6, int num_samples_theta = 90, int num_samples_phi = 45);

            // Create the geometry for a cylinder
            void CreateCylinder(std::string object_name, float height = 1.0, float radius = 0.6, int num_samples_theta = 90, int num_samples_phi = 45);
            // Create the geometry for a cone
            void CreateCone(std::string object_name, float height = 1.0, float radius = 0.6, int num_samples_theta = 90, int num_samples_phi = 45);

			
        private:
           
            // List storing all resources
            std::vector<Resource*> resource_; 
 
            // Methods to load specific types of resources
            // Load shaders programs
            void LoadMaterial(const std::string name, const char *prefix);
            // Load a text file into memory (could be source code)
            std::string LoadTextFile(const char *filename);
            // Load a texture from an image file: png, jpg, etc.
            void LoadTexture(const std::string name, const char* filename);
            // Loads a mesh in obj format
            void LoadMesh(const std::string name, const char* filename);

    }; // class ResourceManager

} // namespace game

#endif // RESOURCE_MANAGER_H_
