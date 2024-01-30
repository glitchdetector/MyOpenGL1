// source 
//https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/1.getting_started/2.1.hello_triangle/hello_triangle.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderLoader.h"

// Number of properties per vertex in the vertex table
constexpr int VERTEX_DEF_LENGTH = 8;
struct Vertex
{
    float x, y, z, r, g, b, u, v;
};

// If anything happens to your frame, this is called
// resizing etc
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;

std::string vertexShaderSourceStr = ShaderLoader::LoadShaderFromFile("svert.glsl");
const char* vertexShaderSource = vertexShaderSourceStr.c_str();
std::string fragmentShaderSourceStr = ShaderLoader::LoadShaderFromFile("sfrag.glsl");
const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();

//vertex shader source 

//const char* vertexShaderSource = R"RRERR(
//#version 330 core
//
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aCol;
//layout (location = 2) in vec2 aUV;
//
//out vec3 color;
//
//void main()
//{
//	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
//	color = aCol;
//}
//)RRERR";

//const char* fragmentShaderSource = R"RRERR(
//#version 330 core
//
//in vec3 color;
//
//void main()
//{
//	gl_FragColor = vec4(color, 1.0);
//}
//)RRERR";

void forcePrecision(std::ostream& stream, int decimals) {
    stream << std::fixed << std::setprecision(decimals);
}

std::vector<float> readVerticesFromFile(std::string fileName)
{
    std::ifstream in;
    in.open(fileName);

    int size;
    in >> size;

    std::cout << "Reading " << size << " vertices from file " << fileName << std::endl;

    int dataPointsToRead = VERTEX_DEF_LENGTH * size;

    std::vector<float> vertices;
    vertices.resize(dataPointsToRead);

    forcePrecision(std::cout, 4);

    for (int i = 0; i < dataPointsToRead; i++)
    {
	    in >> vertices[i];
	}

    return vertices;
}

void ReadVertexDataFile(std::string fileName, std::vector<Vertex>& vertices)
{
    std::ifstream in;
    in.open(fileName);

    int size;
    in >> size;

    for (int i = 0; i < size; i++)
    {
        Vertex v;
        in
            >> v.x
            >> v.y
            >> v.z
            >> v.r
            >> v.g
            >> v.b
            >> v.u
            >> v.v;
        vertices.push_back(v);
    }
}

void ReadLineFile(std::string fileName, std::vector<Vertex>& vertices, std::vector<int>& indices)
{
	ReadVertexDataFile(fileName, vertices);

	for (int i = 1; i < vertices.size(); i += 1)
	{
		indices.push_back(i - 1);
		indices.push_back(i - 0);
	}
}

void ReadTriangleStripFile(std::string fileName, std::vector<Vertex>& vertices, std::vector<int>& indices)
{
    ReadVertexDataFile(fileName, vertices);

	for (int i = 2; i < vertices.size(); i += 1)
	{
		indices.push_back(i - 2);
		indices.push_back(i - 1);
		indices.push_back(i - 0);
	}
}

void ConcatMeshes(std::vector<Vertex>& vertices, std::vector<int>& indices, std::vector<Vertex>& newVerts, std::vector<int>& newIndices)
{
    int vertOffset = vertices.size();
    // Add all new verts to vertices
	for (int i = 0; i < newVerts.size(); i++)
	{
		vertices.push_back(newVerts[i]);
	}

	// Add all new indices to indices
    for (int i = 0; i < newIndices.size(); i++)
	{
		indices.push_back(newIndices[i] + vertOffset);
	}
}

void ReadObjectFile(std::string fileName, std::vector<Vertex>& vertices, std::vector<int>& indices)
{
    std::ifstream in;
    in.open(fileName);

    //vertices.push_back(Vertex{ 0,0,0,0,0,0,0,0 });

    if (!in.is_open())
    {
        std::cout << "Could not open file " << fileName << std::endl;
        return;
    }
    std::cout << "Reading file " << fileName << std::endl;

    std::string line;

    char prefix;
    in >> prefix;
    while (!in.eof())
    {
        switch (prefix)
        {
        case '#':
            std::getline(in, line);
            std::cout << line << std::endl;
            break;
        case 'v':
            // Vertex
            Vertex v;
            in
        		>> v.x
        		>> v.y
        		>> v.z
        		>> v.r
        		>> v.g
        		>> v.b;
            v.u = 0.f;
            v.v = 0.f;
            vertices.push_back(v);
            break;
        case 'f':
            int v1, v2, v3;
            in
                >> v1
                >> v2
                >> v3;
            indices.push_back(v1 - 1);
            indices.push_back(v2 - 1);
            indices.push_back(v3 - 1);
            break;
        case 'o':
            std::getline(in, line);
            std::cout << "Object " << line << std::endl;
            break;
        case 's':
            int shadeSmooth;
            in >> shadeSmooth;
            if (shadeSmooth == 1) {
                std::cout << "Set smooth shading" << std::endl;
            } else {
	            std::cout << "Set flat shading" << std::endl;
            }
            break;
        default:
            std::getline(in, line);
            std::cout << "Unknown line " << line << std::endl;
            break;
        }
        in >> prefix;
    }

    std::cout << "Loaded " << fileName << " with " << vertices.size() << " verts and " << (indices.size() / 3) << " tris." << std::endl;
}

int CurrentRenderMode = GL_TRIANGLES;

struct CameraOffset
{
    float x, y, z;
    float pitch, yaw, roll;
};

CameraOffset cameraOffset = { 0, 0, 0, 0, 0, 0 };

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // to make this portable for other Devices/ operating system MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangle window", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }




    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up shader variables
    GLint timePassedLocation = glGetUniformLocation(shaderProgram, "timePassed");
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
  //  float vertices[] = {
        ////  X      Y      Z    R    G    B    U    V
      //      -0.5f, -0.5f,  0.0f, 1.0, 0.0, 0.0, 0.0, 0.0, // top left  
      //      -0.5f,  0.5f,  0.0f, 0.0, 1.0, 0.0, 0.0, 1.0, // bottom left
      //       0.5f,  0.5f,  0.0f, 0.0, 0.0, 1.0, 1.0, 1.0, // bottom right
      //       0.5f, -0.5f,  0.0f, 1.0, 1.0, 0.0, 1.0, 0.0, // top right
  //  };

    //std::vector<float> vertices = {
    //    //  X      Y      Z     R     G     B     U     V
    //    -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top left  
    //    -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
    //     0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // top right
    //     0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // bottom right
    //     0.7f,  0.7f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // bottom right
    //     0.8f, -0.2f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // bottom right
    //};

    //std::vector<int> indices = {
    //	1, 2, 0, // first triangle
    //	2, 3, 1,  // second triangle
    //};
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    ReadObjectFile("legion.obj", vertices, indices);

    //ReadTriangleStripFile("plane.txt", vertices, indices);
    CurrentRenderMode = GL_TRIANGLES;

    std::vector<Vertex> tempVerts;
    std::vector<int> tempIndices;

    //ReadTriangleStripFile("trianglestrip.txt", tempVerts, tempIndices);

    ConcatMeshes(vertices, indices, tempVerts, tempIndices);

 //   std::vector<float> vertices = readVerticesFromFile("vertices.txt");

 //   const int vertexCount = vertices.size() / VERTEX_DEF_LENGTH;

 //   std::vector<int> indices;
 //   // treat each sequential vertex a new triangle
 //   for (int i = 2; i < vertexCount; i += 1)
 //   {
 //       indices.push_back(i - 2);
 //       indices.push_back(i - 1);
 //       indices.push_back(i - 0);
 //   }

 //   for (int i = 0; i < vertices.size(); i++) {
 //       if (i % VERTEX_DEF_LENGTH == 0) {
 //           std::cout << std::endl;
 //       }
	//    std::cout << " " << vertices[i];
	//}

    //int indices[] = {
    //    0, 1, 2, // first triangle
    //    2, 3, 0,  // second triangle
    //};

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glEnable(GL_DEPTH_TEST);

    glDepthRange(0.0, 10000.0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glLineWidth(0.1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    double previousFrameTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
		double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - previousFrameTime;
		previousFrameTime = currentFrameTime;
		
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        
		// Update shader variables
		glUniform1f(timePassedLocation, (float) currentFrameTime);

        // create transformations
        glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        transform = glm::translate(transform, glm::vec3(cameraOffset.x, cameraOffset.y, cameraOffset.z));
        transform = glm::rotate(transform, cameraOffset.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, cameraOffset.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, cameraOffset.roll, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        // create perspective projection
    	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawElements(CurrentRenderMode, indices.size(), GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time 

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// Convert input to directional vector


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

bool pressedKeyLookup[GLFW_KEY_LAST];
bool hasKeyJustBeenPressed(GLFWwindow* window, int key) {
    bool pressed = glfwGetKey(window, key) == GLFW_PRESS;
    bool wasPressed = pressedKeyLookup[key];
    pressedKeyLookup[key] = pressed;

    if (pressed && !wasPressed) return true;
    return false;
}


struct MovementInput {
    float x, y, z, pitch, yaw, roll;
};

MovementInput processMovementInput(GLFWwindow* window)
{
    MovementInput movement = { 0, 0, 0, 0 };
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		movement.z += 1;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		movement.z -= 1;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        movement.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        movement.x += 1;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        movement.y += 1;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        movement.y -= 1;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        movement.yaw -= 1;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        movement.yaw += 1;
    if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
        movement.pitch -= 1;
    if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
        movement.pitch += 1;
    if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
        movement.roll -= 1;
    if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
        movement.roll += 1;
    return movement;
}

bool isWireframeModeEnabled = false;
void processInput(GLFWwindow* window)
{
    if (hasKeyJustBeenPressed(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    MovementInput movement = processMovementInput(window);
    float movementModifier = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) movementModifier += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) movementModifier += 5.0f;
    cameraOffset.x += movement.x * 0.01f * movementModifier;
    cameraOffset.y += movement.y * 0.01f * movementModifier;
    cameraOffset.z += movement.z * 0.01f * movementModifier;
    cameraOffset.pitch += movement.pitch * 0.01f * movementModifier;
    cameraOffset.yaw += movement.yaw * 0.01f * movementModifier;
    cameraOffset.roll += movement.roll * 0.01f * movementModifier;

    // Wireframe mode toggle
    if (hasKeyJustBeenPressed(window, GLFW_KEY_W)) {
        if (isWireframeModeEnabled) {
            std::cout << "Wireframe mode disabled" << std::endl;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            std::cout << "Wireframe mode enabled" << std::endl;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        isWireframeModeEnabled = !isWireframeModeEnabled;
    }

    // Render mode toggle
    if (hasKeyJustBeenPressed(window, GLFW_KEY_R)) {
	    if (CurrentRenderMode == GL_LINE_STRIP) {
            std::cout << "Switching to GL_TRIANGLES" << std::endl;
	    	CurrentRenderMode = GL_TRIANGLES;
		} else {
            std::cout << "Switching to GL_LINE_STRIP" << std::endl;
			CurrentRenderMode = GL_LINE_STRIP;
		}
	}   
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    std::cout << " windows resized with " << width << " Height " << height << std::endl;
}