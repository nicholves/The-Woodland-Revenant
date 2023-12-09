#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <SOIL/SOIL.h>

#include "model_loader.h"
#include "resource_manager.h"
#include "path_config.h"

namespace game {

    // from https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.1.cubemaps_skybox/cubemaps_skybox.cpp
    float ResourceManager::skyboxVertices_[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    bool ResourceManager::hasGeneratedSkybox_ = false;

    GLuint ResourceManager::skyboxVBO_ = 0;
    GLuint ResourceManager::skyboxVAO_ = 0;

ResourceManager::ResourceManager(void){
}


ResourceManager::~ResourceManager(){
}

const float* ResourceManager::GetSkyboxVertices() {
    return skyboxVertices_;
}

void ResourceManager::GenerateSkybox() {
    if (!hasGeneratedSkybox_) {
        glGenVertexArrays(1, &skyboxVAO_);
        glGenBuffers(1, &skyboxVBO_);
        glBindVertexArray(skyboxVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices_), &skyboxVertices_, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    hasGeneratedSkybox_ = true;
}

GLuint ResourceManager::GetSkyboxVBO() {
    return skyboxVBO_;
}

GLuint ResourceManager::GetSkyboxVAO() {
    return skyboxVAO_;
}

std::vector<std::string> StringSplit(const std::string& str, char separator) {
    int startIndex = 0, endIndex = 0;
    std::vector<std::string> tokens;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == separator) {
            endIndex = i;
            std::string token = "";
            token.append(str, startIndex, endIndex - startIndex);
            if (!token.empty())
                tokens.push_back(token);
            startIndex = endIndex + 1;
        }
    }
    std::string token = "";
    token.append(str, startIndex, str.size() - startIndex);
    if (!token.empty())
    tokens.push_back(token);

    return tokens;
}

bool IsComment(std::string& str) {
    if (str[0] == '/' && str[1] == '/')
        return true;

    return false;
}

void ResourceManager::AddResource(ResourceType type, const std::string name, GLuint resource, GLsizei size){

    Resource *res;

    res = new Resource(type, name, resource, size);

    resource_.push_back(res);
}


void ResourceManager::AddResource(ResourceType type, const std::string name, GLuint array_buffer, GLuint element_array_buffer, GLsizei size){

    Resource *res;

    res = new Resource(type, name, array_buffer, element_array_buffer, size);

    resource_.push_back(res);
}


void ResourceManager::LoadResource(ResourceType type, const std::string name, const char *filename){

    // Call appropriate method depending on type of resource
    if (type == Material || type == SS_Material) {
        LoadMaterial(name, filename, type);
    }
    else if (type == Texture) {
        LoadTexture(name, filename);
    }
    else if (type == SkyboxTexture) {
        LoadSkyboxTexture(name, filename);
    }
    else if (type == Mesh) {
        LoadMesh(name, filename);
    }
    else {
        throw(std::invalid_argument(std::string("Invalid type of resource")));
    }
}

void ResourceManager::LoadCustomResource(ResourceType type, const std::string name, const char* verticesFilepath, const char* facesFilepath) {
    std::string vertexText = LoadTextFile(verticesFilepath);
    std::string faceText = LoadTextFile(facesFilepath);

    GLuint vbo = ParseVertices(vertexText);
    GLsizei faceSize;
    GLuint ebo = ParseFaces(faceText, faceSize);

    // Create resource
    AddResource(Mesh, name, vbo, ebo, faceSize);
}

std::vector<std::vector<bool>> ResourceManager::GetImpassableCells(const char* impassableFilePath, std::vector<std::vector<float>> terrain) {
    constexpr float sizeOfQuad = 0.1f;
    const int coord_offset = 250; // This is to avoid negative indices, seems to be the right value

    std::vector<std::vector<bool>> impassable_cells;

    // Get the impassable data from the impassable file. 1 means that cell is impassable
    std::string fileContent = LoadTextFile(impassableFilePath);
    std::vector<std::string> lines = StringSplit(fileContent, '\n');

    const int width = StringSplit(lines[0], ',').size();
    const int height = lines.size();

    for (const std::string& line : lines) {
        std::vector<std::string> cells = StringSplit(line, ',');
        std::vector<bool> row;

        for (const std::string& cell : cells) {
            row.push_back(cell == "1");
        }

        impassable_cells.push_back(row);
    }

    // Set a cell to impassable if the maximum horizontal or vertical slope is greater than a threshold
    const float tall_slope_threshold = 0.1f;

    // Loop through all cells and get the max slope of each. Then compare against threshold
    /*for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            float maxHorizontalSlope = glm::max(glm::abs(terrain[i][j] - terrain[i][j+1]), glm::abs(terrain[i+1][j] - terrain[i+1][j+1]));
            float maxVerticalSlope = glm::max(glm::abs(terrain[i][j] - terrain[i+1][j]), glm::abs(terrain[i][j+1] - terrain[i+1][j+1]));
            
            float maxSlope = glm::max(maxHorizontalSlope, maxVerticalSlope);

            //std::cout << maxSlope << std::endl;

            if (maxSlope > tall_slope_threshold) {
                impassable_cells[i][j] = true;
            }
        }
    }*/

    return impassable_cells;
}

std::vector<std::vector<float>> ResourceManager::LoadTerrainResource(ResourceType type, const std::string name, const char* terrainFilePath) {
    constexpr float sizeOfQuad = 0.1f;
    
    std::vector<std::vector<float>> terrain_grid;

    // first create the vertices from heightfield data
    std::string fileContent = LoadTextFile(terrainFilePath);
    std::vector<std::string> lines = StringSplit(fileContent, '\n');
    const int width = StringSplit(lines[0], ' ').size();

    const int vertex_att = 11; // 11 attributes per vertex: 3D position (3), normal vector (3), tangent vector (3), 2D texture coordinates (2)
    const int height = lines.size();
    const int vertexNum = width * height;

    GLfloat* vertices = new GLfloat[vertex_att * (width * height)];

    assert(width % 2 == 0);
    assert(height % 2 == 0);

    int rowCtr = 0;
    for (const std::string& line : lines) {
        std::vector<std::string> verticesStrings = StringSplit(line, ' ');
        std::vector<float> row;

        terrain_grid.push_back(row);

        int columnCtr = 0;
        for (const std::string& vertex : verticesStrings) {
            std::vector<std::string> inputNums = StringSplit(vertex, ',');

            float zpos = -((height >> 2) - rowCtr) * sizeOfQuad;
            float xpos = -((width >> 2) - columnCtr) * sizeOfQuad;
            double ypos = atof(inputNums[0].c_str());
            double textCordu = atof(inputNums[1].c_str());
            double textCordv = atof(inputNums[2].c_str());

            vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 0] = static_cast<float>(xpos);
            if (rowCtr > 5 && rowCtr < 10) { // Create flat terrain for road
                vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 1] = 0.5f;
            }
            else if (rowCtr > 20 && rowCtr < 27) { // Create flat terrain for river
                vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 1] = 0.1f;
            }
            else {
                vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 1] = static_cast<float>(ypos);
            }
            vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 2] = static_cast<float>(zpos);

            vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 9] = static_cast<float>(textCordu);
            vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 10] = static_cast<float>(textCordv);

            terrain_grid[rowCtr].push_back(vertices[width * rowCtr * vertex_att + columnCtr * vertex_att + 1]);

            columnCtr++;
        }

        rowCtr++;
    }

    // now fill in the normal and tangent vectors

    /*
        I'm not sure this is correct but basically what I am doing is having all normal vectors point directly up and
        having all tangent vectors be tangential to the next vertex to the right, or down based on what's possible.
        This might give poor estimations for the surface normals when doing normal mapping but i'm not currently sure how to do
        better without duplicating vertices that aren't in the corners.

        For this assignment we aren't doing normal mapping yet so it might not matter

        NV
    */

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            vertices[width * i * vertex_att + j * vertex_att + 3] = 0.0;
            vertices[width * i * vertex_att + j * vertex_att + 4] = 1.0;
            vertices[width * i * vertex_att + j * vertex_att + 5] = 0.0;

            glm::vec3 vertexPos = glm::vec3(vertices[width * i * vertex_att + j * vertex_att + 0],
                vertices[width * i * vertex_att + j * vertex_att + 1], vertices[width * i * vertex_att + j * vertex_att + 2]);

            glm::vec3 comparisonVertex;
            if (j != width - 1) {
                comparisonVertex = glm::vec3(vertices[width * i * vertex_att + (j + 1) * vertex_att + 0],
                    vertices[width * i * vertex_att + (j + 1) * vertex_att + 1], vertices[width * i * vertex_att + (j + 1) * vertex_att + 2]);
            }
            else if (i != height - 1) {
                comparisonVertex = glm::vec3(vertices[width * (i + 1) * vertex_att + j * vertex_att + 0],
                    vertices[width * (i + 1) * vertex_att + j * vertex_att + 1], vertices[width * (i + 1) * vertex_att + j * vertex_att + 2]);
            }
            // we're in the bottom right corner
            else {
                comparisonVertex = glm::vec3(vertices[width * (i - 1) * vertex_att + j * vertex_att + 0],
                    vertices[width * (i - 1) * vertex_att + j * vertex_att + 1], vertices[width * (i - 1) * vertex_att + j * vertex_att + 2]);
            }

            glm::vec3 differenceVector = vertexPos - comparisonVertex;

            vertices[width * i * vertex_att + j * vertex_att + 6] = differenceVector.x;
            vertices[width * i * vertex_att + j * vertex_att + 7] = differenceVector.y;
            vertices[width * i * vertex_att + j * vertex_att + 8] = differenceVector.z;
        }
    }


    // Create OpenGL buffer for vertices
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexNum * vertex_att * sizeof(GLfloat), vertices, GL_STATIC_DRAW);


    // now let's do faces
    const int numQuads = (width - 1) * height;
    const int numFaces = numQuads * 2;

    const int face_att = 3; // Vertex indices (3)
    GLuint* indexes = new GLuint[numFaces * face_att];

    int quadCtr = 0;
    for (int i = 0; i < numFaces * face_att; i += 6) {
        if ((quadCtr - 99) % 100 == 0 && quadCtr != 0) {
        quadCtr++;
        continue;
    }

        GLuint vertex1 = quadCtr;
        GLuint vertex2 = quadCtr + 1;

        GLuint vertex3 = quadCtr + width;
        GLuint vertex4 = quadCtr + width + 1;

        // triangle 1
        indexes[i] = vertex1;
        indexes[i + 1] = vertex2;
        indexes[i + 2] = vertex4;

        // triangle 2
        indexes[i + 3] = vertex1;
        indexes[i + 4] = vertex3;
        indexes[i + 5] = vertex4;

        quadCtr++;
    }

    // Create OpenGL buffer for faces
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numFaces * face_att * sizeof(GLuint), indexes, GL_STATIC_DRAW);


    // Free data buffers
    delete[] vertices;
    delete[] indexes;

    AddResource(ResourceType::Mesh, name, vbo, ebo, numFaces * face_att);

    return terrain_grid;
}

GLuint ResourceManager::ParseVertices(const std::string& verticesText) {
    std::vector<std::string> vertexLinesWithBlanks = StringSplit(verticesText, '\n');
    std::vector<std::string> vertexLines;
    std::copy_if(vertexLinesWithBlanks.begin(), vertexLinesWithBlanks.end(), std::back_inserter(vertexLines), [](std::string& str) {return !str.empty() && !IsComment(str);});

    const int vertex_att = 11;  // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
    const int vertex_num = vertexLines.size();
    GLfloat* vertices = new GLfloat[vertex_num * vertex_att];

    int vertexIndex = 0;
    for (const std::string& vertex : vertexLines) {
        std::vector<std::string> atribs = StringSplit(vertex, ' ');

        if (atribs.size() != vertex_att) {
            throw(std::invalid_argument(std::string("Invalid resource data")));
        }

        for (int i = 0; i < vertex_att; ++i) {
            GLfloat value = static_cast<float>(atof(atribs[i].c_str()));

            vertices[(vertexIndex * vertex_att) + i] = value;
        }

        vertexIndex++;
    }

    // Create OpenGL buffer for vertices
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertices, GL_STATIC_DRAW);


    // Free data buffers
    delete[] vertices;


    return vbo;
}

GLuint ResourceManager::ParseFaces(const std::string& facesText, GLsizei& faceSize) {
    std::vector<std::string> faceLinesWithBlanks = StringSplit(facesText, '\n');
    std::vector<std::string> faceLines;
    std::copy_if(faceLinesWithBlanks.begin(), faceLinesWithBlanks.end(), std::back_inserter(faceLines), [](std::string& str) {return !str.empty() && !IsComment(str);});

    const int face_att = 3; // Vertex indices (3)
    const int face_num = faceLines.size();
    GLuint* indexes = new GLuint[face_num * face_att];

    int faceIndex = 0;
    for (const std::string& face : faceLines) {
        std::vector<std::string> vertexIndexes = StringSplit(face, ' ');

        if (vertexIndexes.size() != face_att) {
            throw(std::invalid_argument(std::string("Invalid resource data")));
        }

        for (int i = 0; i < face_att; ++i) {
            GLuint value = atoi(vertexIndexes[i].c_str());

            indexes[(faceIndex * face_att) + i] = value;
        }

        faceIndex++;
    }

    // Create OpenGL buffer for faces
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), indexes, GL_STATIC_DRAW);

    // Free data buffers
    delete[] indexes;

    faceSize = face_num * face_att;
    return ebo;
}


Resource *ResourceManager::GetResource(const std::string name) const {

    // Find resource with the specified name
    for (size_t i = 0; i < resource_.size(); i++){
        if (resource_[i]->GetName() == name){
            return resource_[i];
        }
    }
    return NULL;
}


void ResourceManager::LoadMaterial(const std::string name, const char* prefix, ResourceType type) {

    // Load vertex program source code
    std::string filename;
    if (type == Material) {
        filename = std::string(prefix) + std::string(VERTEX_PROGRAM_EXTENSION);
    }
    else if (type == SS_Material) {
        filename = SCREEN_SPACE_SHADERS_DIRECTORY "/ScreenSpace_vp.glsl";
    }
    else {
        throw(std::ios_base::failure(std::string("Error Non Existant Material Type")));
    }

    std::string vp = LoadTextFile(filename.c_str());

    // Load fragment program source code
    filename = std::string(prefix) + std::string(FRAGMENT_PROGRAM_EXTENSION);
    std::string fp = LoadTextFile(filename.c_str());

    // Create a shader from the vertex program source code
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const char* source_vp = vp.c_str();
    glShaderSource(vs, 1, &source_vp, NULL);
    glCompileShader(vs);

    // Check if shader compiled successfully
    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(vs, 512, NULL, buffer);
        throw(std::ios_base::failure(std::string("Error compiling vertex shader: ") + std::string(buffer)));
    }

    // Create a shader from the fragment program source code
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char* source_fp = fp.c_str();
    glShaderSource(fs, 1, &source_fp, NULL);
    glCompileShader(fs);

    // Check if shader compiled successfully
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(fs, 512, NULL, buffer);
        throw(std::ios_base::failure(std::string("Error compiling fragment shader: ") + std::string(buffer)));
    }

    // Try to also load a geometry shader
    filename = std::string(prefix) + std::string(GEOMETRY_PROGRAM_EXTENSION);
    bool geometry_program = false;
    std::string strPrefix(prefix);
    std::string gp = "";
    GLuint gs;
    if (strPrefix.find("particle") != std::string::npos) {
        geometry_program = true;
        gp = LoadTextFile(filename.c_str());
    }

    if (geometry_program) {
        // Create a shader from the geometry program source code
        gs = glCreateShader(GL_GEOMETRY_SHADER);
        const char* source_gp = gp.c_str();
        glShaderSource(gs, 1, &source_gp, NULL);
        glCompileShader(gs);

        // Check if shader compiled successfully
        GLint status;
        glGetShaderiv(gs, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            char buffer[512];
            glGetShaderInfoLog(gs, 512, NULL, buffer);
            throw(std::ios_base::failure(std::string("Error compiling geometry shader: ") + std::string(buffer)));
        }
    }

    // Create a shader program linking both vertex and fragment shaders
    // together
    GLuint sp = glCreateProgram();
    glAttachShader(sp, vs);
    glAttachShader(sp, fs);
    if (geometry_program) {
        glAttachShader(sp, gs);
    }
    glLinkProgram(sp);

    // Check if shaders were linked successfully
    glGetProgramiv(sp, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(sp, 512, NULL, buffer);
        throw(std::ios_base::failure(std::string("Error linking shaders: ") + std::string(buffer)));
    }

    // Delete memory used by shaders, since they were already compiled
    // and linked
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (geometry_program) {
        glDeleteShader(gs);
    }

    // Add a resource for the shader program
    AddResource(Material, name, sp, 0);
}


std::string ResourceManager::LoadTextFile(const char *filename){

    // Open file
    std::ifstream f;
    f.open(filename);
    if (f.fail()){
        throw(std::ios_base::failure(std::string("Error opening file ")+std::string(filename)));
    }

    // Read file
    std::string content;
    std::string line;
    while(std::getline(f, line)){
        content += line + "\n";
    }

    // Close file
    f.close();

    return content;
}


// Create the geometry for a cylinder
void ResourceManager::CreateCylinder(std::string object_name, float height, float circle_radius, int num_height_samples, int num_circle_samples) {

    // Create a cylinder

    // Number of vertices and faces to be created
    const GLuint vertex_num = num_height_samples * num_circle_samples + 2; // plus two for top and bottom
    const GLuint face_num = num_height_samples * (num_circle_samples - 1) * 2 + 2 * num_circle_samples; // two extra rings worth for top and bottom

    // Number of attributes for vertices and faces
    const int vertex_att = 11;  // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
    const int face_att = 3; // Vertex indices (3)

    // Data buffers for the shape
    GLfloat* vertex = NULL;
    GLuint* face = NULL;

    GLuint vbo, ebo;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att];
        face = new GLuint[face_num * face_att];
    }
    catch (std::exception& e) {
        throw e;
    }

    // Create vertices 
    double theta; // Angle for circle
    double h; // height
    float s, t; // parameters zero to one
    glm::vec3 loop_center;
    glm::vec3 vertex_position;
    glm::vec3 vertex_normal;
    glm::vec3 vertex_color;
    glm::vec2 vertex_coord;

    for (int i = 0; i < num_height_samples; i++) { // along the side

        s = i / (float)num_height_samples; // parameter s (vertical)
        h = (-0.5 + s) * height;
        for (int j = 0; j < num_circle_samples; j++) { // small circle
            t = j / (float)num_circle_samples;
            theta = 2.0 * glm::pi<GLfloat>() * t; // circle sample (angle theta)

            // Define position, normal and color of vertex
            vertex_normal = glm::vec3(static_cast<float>(cos(theta)), 0.0f, static_cast<float>(sin(theta)));
            vertex_position = glm::vec3(static_cast<float>(cos(theta)) * circle_radius, h, static_cast<float>(sin(theta)) * circle_radius);
            vertex_color = glm::vec3(1.0 - s,
                t,
                s);
            vertex_coord = glm::vec2(s, t);

            // Add vectors to the data buffer
            for (int k = 0; k < 3; k++) {
                vertex[(i * num_circle_samples + j) * vertex_att + k] = vertex_position[k];
                vertex[(i * num_circle_samples + j) * vertex_att + k + 3] = vertex_normal[k];
                vertex[(i * num_circle_samples + j) * vertex_att + k + 6] = vertex_color[k];
            }
            vertex[(i * num_circle_samples + j) * vertex_att + 9] = vertex_coord[0];
            vertex[(i * num_circle_samples + j) * vertex_att + 10] = vertex_coord[1];
        }
    }

    int topvertex = num_circle_samples * num_height_samples;
    int bottomvertex = num_circle_samples * num_height_samples + 1; // indices for top and bottom vertex

    vertex_position = glm::vec3(0, height * (num_height_samples - 1) / (float)num_height_samples - height * 0.5, 0); // location of top middle of cylinder
    vertex_normal = glm::vec3(0, 1, 0);
    vertex_color = glm::vec3(1, 0.6, 0.4);
    vertex_coord = glm::vec2(0, 0); // no good way to texture top and bottom

    for (int k = 0; k < 3; k++) {
        vertex[topvertex * vertex_att + k] = vertex_position[k];
        vertex[topvertex * vertex_att + k + 3] = vertex_normal[k];
        vertex[topvertex * vertex_att + k + 6] = vertex_color[k];
    }
    vertex[(topvertex)*vertex_att + 9] = vertex_coord[0];
    vertex[(topvertex)*vertex_att + 10] = vertex_coord[1];

    //================== bottom vertex

    vertex_position = glm::vec3(0, -0.5 * height, 0); // location of top middle of cylinder
    vertex_normal = glm::vec3(0, -1, 0);
    // leave the color and uv alone

    for (int k = 0; k < 3; k++) {
        vertex[bottomvertex * vertex_att + k] = vertex_position[k];
        vertex[bottomvertex * vertex_att + k + 3] = vertex_normal[k];
        vertex[bottomvertex * vertex_att + k + 6] = vertex_color[k];
    }
    vertex[(bottomvertex)*vertex_att + 9] = vertex_coord[0];
    vertex[(bottomvertex)*vertex_att + 10] = vertex_coord[1];

    //===================== end of vertices

    // Create triangles
    for (int i = 0; i < num_height_samples - 1; i++) {
        for (int j = 0; j < num_circle_samples; j++) {
            // Two triangles per quad
            glm::vec3 t1(((i + 1) % num_height_samples) * num_circle_samples + j,
                i * num_circle_samples + ((j + 1) % num_circle_samples),
                i * num_circle_samples + j);
            glm::vec3 t2(((i + 1) % num_height_samples) * num_circle_samples + j,
                ((i + 1) % num_height_samples) * num_circle_samples + ((j + 1) % num_circle_samples),
                i * num_circle_samples + ((j + 1) % num_circle_samples));
            // Add two triangles to the data buffer
            for (int k = 0; k < 3; k++) {
                face[(i * num_circle_samples + j) * face_att * 2 + k] = (GLuint)t1[k];
                face[(i * num_circle_samples + j) * face_att * 2 + k + face_att] = (GLuint)t2[k];
            }
        }
    }
    int cylbodysize = num_circle_samples * (num_height_samples - 1) * 2; // amount of array filled so far, start adding from here
    // triangles for top disc (fan shape)
    int i = num_height_samples - 1;
    for (int j = 0; j < num_circle_samples; j++) {
        // Bunch of wedges pointing to the centre
        glm::vec3 topwedge(
            i * num_circle_samples + j,
            topvertex,
            i * num_circle_samples + (j + 1) % num_circle_samples
        );

        // note order reversed so that all triangles point outward
        glm::vec3 botwedge(
            0 + (j + 1) % num_circle_samples,
            bottomvertex,
            0 + j
        );

        // Add the triangles to the data buffer
        for (int k = 0; k < 3; k++) {
            face[(cylbodysize + j) * face_att + k] = (GLuint)topwedge[k];
            face[(cylbodysize + j + num_circle_samples) * face_att + k] = (GLuint)botwedge[k];
        }
    }

    // Create OpenGL buffer for vertices
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    // Create OpenGL buffer for faces
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete[] vertex;
    delete[] face;


    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);

}


// Create the geometry for a cone
void ResourceManager::CreateCone(std::string object_name, float height, float circle_radius, int num_height_samples, int num_circle_samples) {

    // Create a cone (adapted from cylinder code)

    // Number of vertices and faces to be created
    const GLuint vertex_num = num_height_samples * num_circle_samples + 2; // plus two for top and bottom
    const GLuint face_num = num_height_samples * (num_circle_samples - 1) * 2 + 2 * num_circle_samples; // two extra rings worth for top and bottom

    // Number of attributes for vertices and faces
    const int vertex_att = 11;  // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
    const int face_att = 3; // Vertex indices (3)

    // Data buffers for the shape
    GLfloat* vertex = NULL;
    GLuint* face = NULL;

    GLuint vbo, ebo;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att];
        face = new GLuint[face_num * face_att];
    }
    catch (std::exception& e) {
        throw e;
    }

    // Create vertices 
    double theta; // Angle for circle
    float local_radius; // radius of this spot on the cone
    double h; // height
    float s, t; // parameters zero to one
    glm::vec3 loop_center;
    glm::vec3 vertex_position;
    glm::vec3 vertex_normal;
    glm::vec3 vertex_color;
    glm::vec2 vertex_coord;

    for (int i = 0; i < num_height_samples; i++) { // along the side

        s = i / (float)num_height_samples; // parameter s (vertical)
        h = (-0.5 + s) * height;
        for (int j = 0; j < num_circle_samples; j++) { // small circle
            t = j / (float)num_circle_samples;
            theta = 2.0 * glm::pi<GLfloat>() * t; // circle sample (angle theta)

            // Define position, normal and color of vertex
            vertex_normal = glm::vec3(cos(theta), 0.0f, sin(theta));
            local_radius = circle_radius * (num_height_samples - i) / num_height_samples; // fraction of the way along 
            vertex_position = glm::vec3(cos(theta) * local_radius, h, sin(theta) * local_radius);
            vertex_color = glm::vec3(1.0 - s,
                t,
                s);
            vertex_coord = glm::vec2(s, t);

            // Add vectors to the data buffer
            for (int k = 0; k < 3; k++) {
                vertex[(i * num_circle_samples + j) * vertex_att + k] = vertex_position[k];
                vertex[(i * num_circle_samples + j) * vertex_att + k + 3] = vertex_normal[k];
                vertex[(i * num_circle_samples + j) * vertex_att + k + 6] = vertex_color[k];
            }
            vertex[(i * num_circle_samples + j) * vertex_att + 9] = vertex_coord[0];
            vertex[(i * num_circle_samples + j) * vertex_att + 10] = vertex_coord[1];
        }
    }

    int topvertex = num_circle_samples * num_height_samples;
    int bottomvertex = num_circle_samples * num_height_samples + 1; // indices for top and bottom vertex

    vertex_position = glm::vec3(0, height * (num_height_samples) / (float)num_height_samples - height * 0.5, 0); // location of tip
    vertex_normal = glm::vec3(0, 1, 0);
    vertex_color = glm::vec3(1, 0.6, 0.4);
    vertex_coord = glm::vec2(0, 0); // no good way to texture top and bottom

    for (int k = 0; k < 3; k++) {
        vertex[topvertex * vertex_att + k] = vertex_position[k];
        vertex[topvertex * vertex_att + k + 3] = vertex_normal[k];
        vertex[topvertex * vertex_att + k + 6] = vertex_color[k];
    }
    vertex[(topvertex)*vertex_att + 9] = vertex_coord[0];
    vertex[(topvertex)*vertex_att + 10] = vertex_coord[1];

    //================== bottom vertex

    vertex_position = glm::vec3(0, -0.5 * height, 0); // location of middle of base of cone
    vertex_normal = glm::vec3(0, -1, 0);
    // leave the color and uv alone

    for (int k = 0; k < 3; k++) {
        vertex[bottomvertex * vertex_att + k] = vertex_position[k];
        vertex[bottomvertex * vertex_att + k + 3] = vertex_normal[k];
        vertex[bottomvertex * vertex_att + k + 6] = vertex_color[k];
    }
    vertex[(bottomvertex)*vertex_att + 9] = vertex_coord[0];
    vertex[(bottomvertex)*vertex_att + 10] = vertex_coord[1];

    //===================== end of vertices

    // Create triangles
    for (int i = 0; i < num_height_samples - 1; i++) {
        for (int j = 0; j < num_circle_samples; j++) {
            // Two triangles per quad
            glm::vec3 t1(((i + 1) % num_height_samples) * num_circle_samples + j,
                i * num_circle_samples + ((j + 1) % num_circle_samples),
                i * num_circle_samples + j);
            glm::vec3 t2(((i + 1) % num_height_samples) * num_circle_samples + j,
                ((i + 1) % num_height_samples) * num_circle_samples + ((j + 1) % num_circle_samples),
                i * num_circle_samples + ((j + 1) % num_circle_samples));
            // Add two triangles to the data buffer
            for (int k = 0; k < 3; k++) {
                face[(i * num_circle_samples + j) * face_att * 2 + k] = (GLuint)t1[k];
                face[(i * num_circle_samples + j) * face_att * 2 + k + face_att] = (GLuint)t2[k];
            }
        }
    }
    int cylbodysize = num_circle_samples * (num_height_samples - 1) * 2; // amount of array filled so far, start adding from here
    // triangles for top disc (fan shape)
    int i = num_height_samples - 1;
    for (int j = 0; j < num_circle_samples; j++) {
        // Bunch of wedges pointing to the centre
        glm::vec3 topwedge(
            i * num_circle_samples + j,
            topvertex,
            i * num_circle_samples + (j + 1) % num_circle_samples
        );

        // note order reversed so that all triangles point outward
        glm::vec3 botwedge(
            0 + (j + 1) % num_circle_samples,
            bottomvertex,
            0 + j
        );

        // Add the triangles to the data buffer
        for (int k = 0; k < 3; k++) {
            face[(cylbodysize + j) * face_att + k] = (GLuint)topwedge[k];
            face[(cylbodysize + j + num_circle_samples) * face_att + k] = (GLuint)botwedge[k];
        }
    }

    // Create OpenGL buffer for vertices
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    // Create OpenGL buffer for faces
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete[] vertex;
    delete[] face;


    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);

}

void ResourceManager::CreateTorus(std::string object_name, float loop_radius, float circle_radius, int num_loop_samples, int num_circle_samples){

    // Create a torus
    // The torus is built from a large loop with small circles around the loop

    // Number of vertices and faces to be created
    // Check the construction algorithm below to understand the numbers
    // specified below
    const GLuint vertex_num = num_loop_samples*num_circle_samples;
    const GLuint face_num = num_loop_samples*num_circle_samples*2;

    // Number of attributes for vertices and faces
    const int vertex_att = 11;
    const int face_att = 3;

    // Data buffers for the torus
    GLfloat *vertex = NULL;
    GLuint *face = NULL;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
        face = new GLuint[face_num * face_att]; // 3 indices per face
    }
    catch  (std::exception &e){
        throw e;
    }

    // Create vertices 
    double theta, phi; // Angles for circles
    glm::vec3 loop_center;
    glm::vec3 vertex_position;
    glm::vec3 vertex_normal;
    glm::vec3 vertex_color;
    glm::vec2 vertex_coord;

    for (int i = 0; i < num_loop_samples; i++){ // large loop
        
        theta = 2.0*glm::pi<GLfloat>()*i/num_loop_samples; // loop sample (angle theta)
        loop_center = glm::vec3(loop_radius*cos(theta), loop_radius*sin(theta), 0); // centre of a small circle

        for (int j = 0; j < num_circle_samples; j++){ // small circle
            
            phi = 2.0*glm::pi<GLfloat>()*j/num_circle_samples; // circle sample (angle phi)
            
            // Define position, normal and color of vertex
            vertex_normal = glm::vec3(cos(theta)*cos(phi), sin(theta)*cos(phi), sin(phi));
            vertex_position = loop_center + vertex_normal*circle_radius;
            vertex_color = glm::vec3(1.0 - ((float) i / (float) num_loop_samples), 
                                            (float) i / (float) num_loop_samples, 
                                            (float) j / (float) num_circle_samples);
            vertex_coord = glm::vec2(theta / (2.0*glm::pi<GLfloat>()),
                                     phi / (2.0*glm::pi<GLfloat>()));

            // Add vectors to the data buffer
            for (int k = 0; k < 3; k++){
                vertex[(i*num_circle_samples+j)*vertex_att + k] = vertex_position[k];
                vertex[(i*num_circle_samples+j)*vertex_att + k + 3] = vertex_normal[k];
                vertex[(i*num_circle_samples+j)*vertex_att + k + 6] = vertex_color[k];
            }
            vertex[(i*num_circle_samples+j)*vertex_att + 9] = vertex_coord[0];
            vertex[(i*num_circle_samples+j)*vertex_att + 10] = vertex_coord[1];
        }
    }

    // Create triangles
    for (int i = 0; i < num_loop_samples; i++){
        for (int j = 0; j < num_circle_samples; j++){
            // Two triangles per quad
            glm::vec3 t1(((i + 1) % num_loop_samples)*num_circle_samples + j, 
                         i*num_circle_samples + ((j + 1) % num_circle_samples),
                         i*num_circle_samples + j);    
            glm::vec3 t2(((i + 1) % num_loop_samples)*num_circle_samples + j,
                         ((i + 1) % num_loop_samples)*num_circle_samples + ((j + 1) % num_circle_samples),
                         i*num_circle_samples + ((j + 1) % num_circle_samples));
            // Add two triangles to the data buffer
            for (int k = 0; k < 3; k++){
                face[(i*num_circle_samples+j)*face_att*2 + k] = (GLuint) t1[k];
                face[(i*num_circle_samples+j)*face_att*2 + k + face_att] = (GLuint) t2[k];
            }
        }
    }

    // Create OpenGL buffers and copy data
    //GLuint vao;
    //glGenVertexArrays(1, &vao);
    //glBindVertexArray(vao);

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete [] vertex;
    delete [] face;

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);
}


void ResourceManager::CreateSphere(std::string object_name, float radius, int num_samples_theta, int num_samples_phi){

    // Create a sphere using a well-known parameterization

    // Number of vertices and faces to be created
    const GLuint vertex_num = num_samples_theta*num_samples_phi;
    const GLuint face_num = num_samples_theta*(num_samples_phi-1)*2;

    // Number of attributes for vertices and faces
    const int vertex_att = 11;
    const int face_att = 3;

    // Data buffers 
    GLfloat *vertex = NULL;
    GLuint *face = NULL;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
        face = new GLuint[face_num * face_att]; // 3 indices per face
    }
    catch  (std::exception &e){
        throw e;
    }

    // Create vertices 
    double theta, phi; // Angles for parametric equation
    glm::vec3 vertex_position;
    glm::vec3 vertex_normal;
    glm::vec3 vertex_color;
    glm::vec2 vertex_coord;
   
    for (int i = 0; i < num_samples_theta; i++){
            
        theta = 2.0*glm::pi<GLfloat>()*i/(num_samples_theta-1); // angle theta
            
        for (int j = 0; j < num_samples_phi; j++){
                    
            phi = glm::pi<GLfloat>()*j/(num_samples_phi-1); // angle phi

            // Define position, normal and color of vertex
            vertex_normal = glm::vec3(cos(theta)*sin(phi), sin(theta)*sin(phi), -cos(phi));
            // We need z = -cos(phi) to make sure that the z coordinate runs from -1 to 1 as phi runs from 0 to pi
            // Otherwise, the normal will be inverted
            vertex_position = glm::vec3(vertex_normal.x*radius, 
                                        vertex_normal.y*radius, 
                                        vertex_normal.z*radius),
            vertex_color = glm::vec3(((float)i)/((float)num_samples_theta), 1.0-((float)j)/((float)num_samples_phi), ((float)j)/((float)num_samples_phi));
            vertex_coord = glm::vec2(((float)i)/((float)num_samples_theta), 1.0-((float)j)/((float)num_samples_phi));

            // Add vectors to the data buffer
            for (int k = 0; k < 3; k++){
                vertex[(i*num_samples_phi+j)*vertex_att + k] = vertex_position[k];
                vertex[(i*num_samples_phi+j)*vertex_att + k + 3] = vertex_normal[k];
                vertex[(i*num_samples_phi+j)*vertex_att + k + 6] = vertex_color[k];
            }
            vertex[(i*num_samples_phi+j)*vertex_att + 9] = vertex_coord[0];
            vertex[(i*num_samples_phi+j)*vertex_att + 10] = vertex_coord[1];
        }
    }

    // Create faces
    for (int i = 0; i < num_samples_theta; i++){
        for (int j = 0; j < (num_samples_phi-1); j++){
            // Two triangles per quad
            glm::vec3 t1(((i + 1) % num_samples_theta)*num_samples_phi + j, 
                         i*num_samples_phi + (j + 1),
                         i*num_samples_phi + j);
            glm::vec3 t2(((i + 1) % num_samples_theta)*num_samples_phi + j, 
                         ((i + 1) % num_samples_theta)*num_samples_phi + (j + 1), 
                         i*num_samples_phi + (j + 1));
            // Add two triangles to the data buffer
            for (int k = 0; k < 3; k++){
                face[(i*(num_samples_phi-1)+j)*face_att*2 + k] = (GLuint) t1[k];
                face[(i*(num_samples_phi-1)+j)*face_att*2 + k + face_att] = (GLuint) t2[k];
            }
        }
    }

    // Create OpenGL buffers and copy data
    //GLuint vao;
    //glGenVertexArrays(1, &vao);
    //glBindVertexArray(vao);

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete [] vertex;
    delete [] face;

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);
}


void ResourceManager::CreateVertex(std::string object_name) {
    // Create a singular, invisible vertex

    // Number of vertices and faces to be created
    const GLuint vertex_num = 1;
    const GLuint face_num = 0;

    // Number of attributes for vertices and faces
    const int vertex_att = 11;
    const int face_att = 3;

    // Data buffers 
    GLfloat* vertex = NULL;
    GLuint* face = NULL;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
        face = new GLuint[face_num * face_att]; // 3 indices per face
    }
    catch (std::exception& e) {
        throw e;
    }

    // Create vertex
    glm::vec3 pos = glm::vec3(0, 0, 0);
    glm::vec3 norm = glm::vec3(0, 1, 0);
    glm::vec3 color = glm::vec3(1, 1, 1);

    for (int i = 0; i < 3; ++i) {
        vertex[i] = pos[i];
        vertex[i + 3] = norm[i];
        vertex[i + 6] = color[i];
    }
    vertex[9] = 0;
    vertex[10] = 0;

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete[] vertex;
    delete[] face;

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);
}


void ResourceManager::CreateWall(std::string object_name, glm::vec3 color) {

    GLfloat vertex[] = {
        // Position, normal, color, texture coordinates
        // Here, color stores the tangent of the vertex
        -1.0, -1.0, 0.0,  0.0, -1.0,  0.0,  color[0], color[1], color[2],  0.0, 0.0,
        -1.0,  1.0, 0.0,  0.0, -1.0,  0.0,  color[0], color[1], color[2],  0.0, 1.0,
         1.0,  1.0, 0.0,  0.0, -1.0,  0.0,  color[0], color[1], color[2],  1.0, 1.0,
         1.0, -1.0, 0.0,  0.0, -1.0,  0.0,  color[0], color[1], color[2],  1.0, 0.0 };
    GLuint face[] = { 0, 2, 1,
                     0, 3, 2 };

    // Create OpenGL buffers and copy data
    GLuint vbo, ebo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 11 * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, 2 * 3);
}


void ResourceManager::CreatePlane(std::string object_name, int repeatsX) {
    const GLuint vertex_num = 4;
    const GLuint face_num = 2;

    // Number of attributes for vertices and faces
    const int vertex_att = 11;
    const int face_att = 3;

    // Data buffers 
    GLfloat* vertex = NULL;
    GLuint* face = NULL;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
        face = new GLuint[face_num * face_att]; // 3 indices per face
    }
    catch (std::exception& e) {
        throw e;
    }

    // Create vertices
    for (int i = 0; i < 4; ++i) {
        glm::vec3 pos;

        switch (i)
        {
        case 0:
            pos = glm::vec3(-1, 0, -1);
            break;
        case 1:
            pos = glm::vec3(-1, 0, 1);
            break;
        case 2:
            pos = glm::vec3(1, 0, 1);
			break;
		case 3:
			pos = glm::vec3(1, 0, -1);
			break;
        default:
            break;
        }

        glm::vec3 norm = glm::vec3(0, -1, 0);
        glm::vec3 color = glm::vec3(1, 1, 1);

        for (int j = 0; j < 3; ++j) {
            vertex[i * vertex_att + j] = pos[j];
            vertex[i * vertex_att + j + 3] = norm[j];
            vertex[i * vertex_att + j + 6] = color[j];
        }
        // uv
        vertex[i * vertex_att + 9] = pos.x / 2 * repeatsX + 0.5;
        vertex[i * vertex_att + 10] = pos.z / 2 + 0.5;
    }

    // Create triangles
    glm::vec3 t1(0, 1, 2);
    glm::vec3 t2(0, 2, 3);

    for (int k = 0; k < 3; k++) {
		face[k] = (GLuint)t1[k];
		face[k + face_att] = (GLuint)t2[k];
	}

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete[] vertex;
    delete[] face;

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);
}


void ResourceManager::LoadTexture(const std::string name, const char* filename) {

    // Load texture from file
    GLuint texture = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
    if (!texture) {
        throw(std::ios_base::failure(std::string("Error loading texture ") + std::string(filename) + std::string(": ") + std::string(SOIL_last_result())));
    }

    // Create resource
    AddResource(Texture, name, texture, 0);
}

void ResourceManager::LoadSkyboxTexture(const std::string name, const char* filepath) {
    char* filenames[] = {"/posx.png",
                         "/negx.png",
                         "/posy.png",
                         "/negy.png",
                         "/posz.png",
                         "/negz.png"
                        
    };

    assert(strlen(filepath) + sizeof("/posx.png") + 1 < 256);

    char filepaths[6][256];

    // fill buffer with nulls
    memset(filepaths, 0, sizeof(filepaths));

    for (int i = 0; i < 6; ++i) {
        // so I can use strcpy and strcat
        #pragma warning( push )
        #pragma warning(disable:4996)
        strcpy(filepaths[i], filepath);
        strcat(filepaths[i], filenames[i]);
        #pragma warning( pop )
    }

    GLuint texture = SOIL_load_OGL_cubemap(filepaths[0], filepaths[1], filepaths[2],
                                             filepaths[3], filepaths[4], filepaths[5],
                                             SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
    if (!texture) {
        throw(std::ios_base::failure(std::string("Error loading texture ") + std::string(filepath) + std::string(": ") + std::string(SOIL_last_result())));
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    AddResource(SkyboxTexture, name, texture, 0);
}


void ResourceManager::LoadMesh(const std::string name, const char* filename) {

    // First load model into memory. If that goes well, we transfer the
    // mesh to an OpenGL buffer
    TriMesh mesh;

    // Parse file
    // Open file
    std::ifstream f;
    f.open(filename);
    if (f.fail()) {
        throw(std::ios_base::failure(std::string("Error opening file ") + std::string(filename)));
    }

    // Parse lines
    std::string line;
    std::string ignore(" \t\r\n");
    std::string part_separator(" \t");
    std::string face_separator("/");
    bool added_normal = false;
    while (std::getline(f, line)) {
        // Clean extremities of the string
        string_trim(line, ignore);
        // Ignore comments
        if ((line.size() <= 0) ||
            (line[0] == '#')) {
            continue;
        }
        // Parse string
        std::vector<std::string> part = string_split(line, part_separator);
        // Check commands
        if (!part[0].compare(std::string("v"))) {
            if (part.size() >= 4) {
                glm::vec3 position(str_to_num<float>(part[1].c_str()), str_to_num<float>(part[2].c_str()), str_to_num<float>(part[3].c_str()));
                mesh.position.push_back(position);
            }
            else {
                throw(std::ios_base::failure(std::string("Error: v command should have exactly 3 parameters")));
            }
        }
        else if (!part[0].compare(std::string("vn"))) {
            if (part.size() >= 4) {
                glm::vec3 normal(str_to_num<float>(part[1].c_str()), str_to_num<float>(part[2].c_str()), str_to_num<float>(part[3].c_str()));
                mesh.normal.push_back(normal);
                added_normal = true;
            }
            else {
                throw(std::ios_base::failure(std::string("Error: vn command should have exactly 3 parameters")));
            }
        }
        else if (!part[0].compare(std::string("vt"))) {
            if (part.size() >= 3) {
                glm::vec2 tex_coord(str_to_num<float>(part[1].c_str()), str_to_num<float>(part[2].c_str()));
                mesh.tex_coord.push_back(tex_coord);
            }
            else {
                throw(std::ios_base::failure(std::string("Error: vt command should have exactly 2 parameters")));
            }
        }
        else if (!part[0].compare(std::string("f"))) {
            if (part.size() >= 4) {
                if (part.size() > 5) {
                    throw(std::ios_base::failure(std::string("Error: f commands with more than 4 vertices not supported")));
                }
                else if (part.size() == 5) {
                    // Break a quad into two triangles
                    Quad quad;
                    for (int i = 0; i < 4; i++) {
                        std::vector<std::string> fd = string_split_once(part[i + 1], face_separator);
                        if (fd.size() == 1) {
                            quad.i[i] = static_cast<int>(str_to_num<float>(fd[0].c_str()) - 1);
                            quad.t[i] = -1;
                            quad.n[i] = -1;
                        }
                        else if (fd.size() == 2) {
                            quad.i[i] = static_cast<int>(str_to_num<float>(fd[0].c_str()) - 1);
                            quad.t[i] = static_cast<int>(str_to_num<float>(fd[1].c_str()) - 1);
                            quad.n[i] = -1;
                        }
                        else if (fd.size() == 3) {
                            quad.i[i] = static_cast<int>(str_to_num<float>(fd[0].c_str()) - 1);
                            if (std::string("").compare(fd[1]) != 0) {
                                quad.t[i] = static_cast<int>(str_to_num<float>(fd[1].c_str()) - 1);
                            }
                            else {
                                quad.t[i] = -1;
                            }
                            quad.n[i] = static_cast<int>(str_to_num<float>(fd[2].c_str()) - 1);
                        }
                        else {
                            throw(std::ios_base::failure(std::string("Error: f parameter should have 1 or 3 parameters separated by '/'")));
                        }
                    }
                    Face face1, face2;
                    face1.i[0] = quad.i[0]; face1.i[1] = quad.i[1]; face1.i[2] = quad.i[2];
                    face1.n[0] = quad.n[0]; face1.n[1] = quad.n[1]; face1.n[2] = quad.n[2];
                    face1.t[0] = quad.t[0]; face1.t[1] = quad.t[1]; face1.t[2] = quad.t[2];
                    face2.i[0] = quad.i[0]; face2.i[1] = quad.i[2]; face2.i[2] = quad.i[3];
                    face2.n[0] = quad.n[0]; face2.n[1] = quad.n[2]; face2.n[2] = quad.n[3];
                    face2.t[0] = quad.t[0]; face2.t[1] = quad.t[2]; face2.t[2] = quad.t[3];
                    mesh.face.push_back(face1);
                    mesh.face.push_back(face2);
                }
                else if (part.size() == 4) {
                    Face face;
                    for (int i = 0; i < 3; i++) {
                        std::vector<std::string> fd = string_split_once(part[i + 1], face_separator);
                        if (fd.size() == 1) {
                            face.i[i] = static_cast<int>(str_to_num<float>(fd[0].c_str()) - 1);
                            face.t[i] = -1;
                            face.n[i] = -1;
                        }
                        else if (fd.size() == 2) {
                            face.i[i] = static_cast<int>(str_to_num<float>(fd[0].c_str()) - 1);
                            face.t[i] = static_cast<int>(str_to_num<float>(fd[1].c_str()) - 1);
                            face.n[i] = -1;
                        }
                        else if (fd.size() == 3) {
                            face.i[i] = static_cast<int>(str_to_num<float>(fd[0].c_str()) - 1);
                            if (std::string("").compare(fd[1]) != 0) {
                                face.t[i] = static_cast<int>(str_to_num<float>(fd[1].c_str()) - 1);
                            }
                            else {
                                face.t[i] = -1;
                            }
                            face.n[i] = static_cast<int>(str_to_num<float>(fd[2].c_str()) - 1);
                        }
                        else {
                            throw(std::ios_base::failure(std::string("Error: f parameter should have 1, 2, or 3 parameters separated by '/'")));
                        }
                    }
                    mesh.face.push_back(face);
                }
            }
            else {
                throw(std::ios_base::failure(std::string("Error: f command should have 3 or 4 parameters")));
            }
        }
        // Ignore other commands
    }

    // Close file
    f.close();

    // Check if vertex references are correct
    for (unsigned int i = 0; i < mesh.face.size(); i++) {
        for (int j = 0; j < 3; j++) {
            if (mesh.face[i].i[j] >= static_cast<int>(mesh.position.size())) {
                throw(std::ios_base::failure(std::string("Error: index for triangle ") + num_to_str<int>(mesh.face[i].i[j]) + std::string(" is out of bounds")));
            }
        }
    }

    // Compute degree of each vertex
    std::vector<int> degree(mesh.position.size(), 0);
    for (unsigned int i = 0; i < mesh.face.size(); i++) {
        for (int j = 0; j < 3; j++) {
            degree[mesh.face[i].i[j]]++;
        }
    }

    // Compute vertex normals if no normals were ever added
    if (!added_normal) {
        mesh.normal = std::vector<glm::vec3>(mesh.position.size(), glm::vec3(0.0, 0.0, 0.0));
        for (unsigned int i = 0; i < mesh.face.size(); i++) {
            // Compute face normal
            glm::vec3 vec1, vec2;
            vec1 = mesh.position[mesh.face[i].i[0]] -
                mesh.position[mesh.face[i].i[1]];
            vec2 = mesh.position[mesh.face[i].i[0]] -
                mesh.position[mesh.face[i].i[2]];
            glm::vec3 norm = glm::cross(vec1, vec2);
            norm = glm::normalize(norm);
            // Add face normal to vertices
            mesh.normal[mesh.face[i].i[0]] += norm;
            mesh.normal[mesh.face[i].i[1]] += norm;
            mesh.normal[mesh.face[i].i[2]] += norm;
        }
        for (unsigned int i = 0; i < mesh.normal.size(); i++) {
            if (degree[i] > 0) {
                mesh.normal[i] /= degree[i];
            }
        }
    }

    // Debug
    //print_mesh(mesh);

    // If we got to this point, the file was parsed successfully and the
    // mesh is in memory
    // Now, transfer the mesh to OpenGL buffers
    // Create three new vertices for each face, in case vertex
    // normals/texture coordinates are not consistent over the mesh

    // Number of attributes for vertices and faces
    const int vertex_att = 11;
    const int face_att = 3;

    // Create OpenGL buffers and copy data
    GLuint vbo, ebo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.face.size() * 3 * vertex_att * sizeof(GLuint), 0, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.face.size() * face_att * sizeof(GLuint), 0, GL_STATIC_DRAW);

    unsigned int vertex_index = 0;
    for (unsigned int i = 0; i < mesh.face.size(); i++) {
        // Add three vertices and their attributes
        GLfloat att[3 * vertex_att] = { 0 };
        for (int j = 0; j < 3; j++) {
            // Position
            att[j * vertex_att + 0] = mesh.position[mesh.face[i].i[j]][0];
            att[j * vertex_att + 1] = mesh.position[mesh.face[i].i[j]][1];
            att[j * vertex_att + 2] = mesh.position[mesh.face[i].i[j]][2];
            // Normal
            if (!added_normal) {
                att[j * vertex_att + 3] = mesh.normal[mesh.face[i].i[j]][0];
                att[j * vertex_att + 4] = mesh.normal[mesh.face[i].i[j]][1];
                att[j * vertex_att + 5] = mesh.normal[mesh.face[i].i[j]][2];
            }
            else {
                if (mesh.face[i].n[j] >= 0) {
                    att[j * vertex_att + 3] = mesh.normal[mesh.face[i].n[j]][0];
                    att[j * vertex_att + 4] = mesh.normal[mesh.face[i].n[j]][1];
                    att[j * vertex_att + 5] = mesh.normal[mesh.face[i].n[j]][2];
                }
            }
            // No color in (6, 7, 8)
            // Texture coordinates
            if (mesh.face[i].t[j] >= 0) {
                att[j * vertex_att + 9] = mesh.tex_coord[mesh.face[i].t[j]][0];
                att[j * vertex_att + 10] = mesh.tex_coord[mesh.face[i].t[j]][1];
            }
        }

        // Copy attributes to buffer
        glBufferSubData(GL_ARRAY_BUFFER, i * 3 * vertex_att * sizeof(GLfloat), 3 * vertex_att * sizeof(GLfloat), att);

        // Add triangle
        GLuint findex[face_att] = { 0 };
        findex[0] = vertex_index;
        findex[1] = vertex_index + 1;
        findex[2] = vertex_index + 2;
        vertex_index += 3;

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i * face_att * sizeof(GLuint), face_att * sizeof(GLuint), findex);
    }

    // Create resource
    AddResource(Mesh, name, vbo, ebo, mesh.face.size() * face_att);
}


void ResourceManager::CreateSphereParticles(std::string object_name, int num_particles) {

    // Create a set of points which will be the particles
    // This is similar to drawing a sphere: we will sample points on a sphere, but will allow them to also deviate a bit from the sphere along the normal (change of radius)

    // Data buffer
    GLfloat* particle = NULL;

    // Number of attributes per particle: position (3), normal (3), and color (3), texture coordinates (2)
    const int particle_att = 11;

    // Allocate memory for buffer
    try {
        particle = new GLfloat[num_particles * particle_att];
    }
    catch (std::exception& e) {
        throw e;
    }

    float trad = 0.2f; // Defines the starting point of the particles along the normal
    float maxspray = 1.0f; // This is how much we allow the points to deviate from the sphere
    double u, v, w, theta, phi, spray; // Work variables

    for (int i = 0; i < num_particles; i++) {

        // Get three random numbers
        u = (static_cast<float>(rand()) / (RAND_MAX));
        v = (static_cast<float>(rand()) / (RAND_MAX));
        w = (static_cast<float>(rand()) / (RAND_MAX));

        // Use u to define the angle theta along one direction of the sphere
        theta = u * 2.0 * glm::pi<float>();
        // Use v to define the angle phi along the other direction of the sphere
        phi = acos(2.0 * v - 1.0);
        // Use w to define how much we can deviate from the surface of the sphere (change of radius)
        spray = maxspray * pow((float)w, (float)(1.0 / 3.0)); // Cubic root of w
        //spray = maxspray;

        // Define the normal and point based on theta, phi and the spray
        glm::vec3 normal(spray * cos(theta) * sin(phi), spray * sin(theta) * sin(phi), spray * cos(phi));
        glm::vec3 position(normal.x * trad, normal.y * trad, normal.z * trad);
        glm::vec3 color(i / (float)num_particles, 0.0, 1.0 - (i / (float)num_particles)); // We can use the color for debug, if needed

        // Add vectors to the data buffer
        for (int k = 0; k < 3; k++) {
            particle[i * particle_att + k] = position[k];
            particle[i * particle_att + k + 3] = normal[k];
            particle[i * particle_att + k + 6] = color[k];
        }
    }

    // Create OpenGL buffer and copy data
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_particles * particle_att * sizeof(GLfloat), particle, GL_STATIC_DRAW);

    // Free data buffers
    delete[] particle;

    // Create resource
    AddResource(PointSet, object_name, vbo, 0, num_particles);
}


void string_trim(std::string str, std::string to_trim) {

    // Trim any character in to_trim from the beginning of the string str
    while ((str.size() > 0) &&
        (to_trim.find(str[0]) != std::string::npos)) {
        str.erase(0);
    }

    // Trim any character in to_trim from the end of the string str
    while ((str.size() > 0) &&
        (to_trim.find(str[str.size() - 1]) != std::string::npos)) {
        str.erase(str.size() - 1);
    }
}


std::vector<std::string> string_split(std::string str, std::string separator) {

    // Initialize output
    std::vector<std::string> output;
    output.push_back(std::string(""));
    int string_index = 0;

    // Analyze string
    unsigned int i = 0;
    while (i < str.size()) {
        // Check if character i is a separator
        if (separator.find(str[i]) != std::string::npos) {
            // Split string
            string_index++;
            output.push_back(std::string(""));
            // Skip separators
            while ((i < str.size()) && (separator.find(str[i]) != std::string::npos)) {
                i++;
            }
        }
        else {
            // Otherwise, copy string
            output[string_index] += str[i];
            i++;
        }
    }

    return output;
}


std::vector<std::string> string_split_once(std::string str, std::string separator) {

    // Initialize output
    std::vector<std::string> output;
    output.push_back(std::string(""));
    int string_index = 0;

    // Analyze string
    unsigned int i = 0;
    while (i < str.size()) {
        // Check if character i is a separator
        if (separator.find(str[i]) != std::string::npos) {
            // Split string
            string_index++;
            output.push_back(std::string(""));
            // Skip single separator
            i++;
        }
        else {
            // Otherwise, copy string
            output[string_index] += str[i];
            i++;
        }
    }

    return output;
}


void print_mesh(TriMesh& mesh) {

    for (unsigned int i = 0; i < mesh.position.size(); i++) {
        std::cout << "v " <<
            mesh.position[i].x << " " <<
            mesh.position[i].y << " " <<
            mesh.position[i].z << std::endl;
    }
    for (unsigned int i = 0; i < mesh.normal.size(); i++) {
        std::cout << "vn " <<
            mesh.normal[i].x << " " <<
            mesh.normal[i].y << " " <<
            mesh.normal[i].z << std::endl;
    }
    for (unsigned int i = 0; i < mesh.tex_coord.size(); i++) {
        std::cout << "vt " <<
            mesh.tex_coord[i].x << " " <<
            mesh.tex_coord[i].y << std::endl;
    }
    for (unsigned int i = 0; i < mesh.face.size(); i++) {
        std::cout << "f " <<
            mesh.face[i].i[0] << " " <<
            mesh.face[i].i[1] << " " <<
            mesh.face[i].i[2] << " " << std::endl;
    }
}


template <typename T> std::string num_to_str(T num) {

    std::ostringstream ss;
    ss << num;
    return ss.str();
}


template <typename T> T str_to_num(const std::string& str) {

    std::istringstream ss(str);
    T result;
    ss >> result;
    if (ss.fail()) {
        throw(std::ios_base::failure(std::string("Invalid number: ") + str));
    }
    return result;
}

} // namespace game;
