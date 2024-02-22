#include <iostream>
#include <fstream>
#include <glad/glad.h> // Manages OpenGL function pointers [Generates platform-specific code for loading OpenGL functions]
#include <GLFW/glfw3.h> // API for OpenGL window, context, input creation/handling

// Helpers/Utils for matrix operations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Image loader (for turbine texture)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Settings.h"
#include "Shader.h"
#include "ShaderSources.h"

int INITIAL_WINDOW_HEIGHT = 800;
int INITIAL_WINDOW_WIDTH = 600;
std::string SETTINGS_PATHNAME = "settings.txt"; // Settings file name

void framebuffer_size_callback(GLFWwindow* window, int width, int height); // Called when window is resized
void processInput(GLFWwindow* window); // Input processor

// Handles settings
void handleSettings(std::vector<Setting> settings);
void handleSetting(std::string settingName, std::string value);

float turbineSpeed = 0.0f; // Turbine rotation Speed
float lastSettingsCheckTime = 0.0f; // Last time we checked for update in the settings files

int main() {
    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create glfw window
    GLFWwindow* window = glfwCreateWindow(INITIAL_WINDOW_HEIGHT, INITIAL_WINDOW_WIDTH, "ITGroup", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Pass window resize handler
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize Glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // --------------------------------------- TURBINE INITIALIZATION BEGIN ---------------------------------------

    // Create shader from shader sources
    Shader turbineShader;
    turbineShader.setFromSource(turbineVertexShaderSource, turbineFragmentShaderSource);

    // Vertex, Colors, Texture Coords for Turbine
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    // Indices to use for Turbine
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // Vertex Buffer Object = Where we store the vertices
    unsigned int VBOs[2];
    glGenBuffers(2, VBOs);

    // Vertex Array Object = We store attribute calls and switch between them
    unsigned int VAOs[2];
    glGenVertexArrays(2, VAOs);

    // Element Buffer Objects = We basically specify the unique vertices and the order which we want them drawn, prevents storing redundant vertices
    unsigned int EBOs[2];
    glGenBuffers(2, EBOs);

    glBindVertexArray(VAOs[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set data to send to position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set data to send to colour attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set data to send to texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Creating the turbine texture
    unsigned int turbineTexture;
    glGenTextures(1, &turbineTexture);
    glBindTexture(GL_TEXTURE_2D, turbineTexture);

    // Set the texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Read image texture, load it and generate Mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // (flip image, because images and OpenGl uses different coordination scheme)
    unsigned char* text1 = stbi_load("turbine.png", &width, &height, &nrChannels, 0);
    if (text1) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, text1);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(text1);

    // Set Texture
    turbineShader.use();
    glUniform1i(glGetUniformLocation(turbineShader.ID, "turbineTexture"), 0);

    // --------------------------------------- TURBINE INITIALIZATION END ---------------------------------------

    // --------------------------------------- BODY INITIALIZATION BEGIN ---------------------------------------
    
    Shader bodyShader;
    bodyShader.setFromSource(bodyVertexShaderSource, bodyFragmentShaderSource);

    // Vertex coords for turbine body
    float bodyVertices[] = {
        // positions
         0.02f,  0.0f, -1.0f,   // top right
         0.05f, -1.0f, -1.0f,   // bottom right
        -0.05f, -1.0f, -1.0f,   // bottom left
        -0.02f,  0.0f, -1.0f    // top left 
    };

    // Indices to use for turbine body
    unsigned int bodyIndices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glBindVertexArray(VAOs[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bodyVertices), bodyVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bodyIndices), bodyIndices, GL_STATIC_DRAW);

    // Set data to send to position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --------------------------------------- BODY INITIALIZATION END ---------------------------------------

    // Load settings
    handleSettings(readSettings(SETTINGS_PATHNAME));

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // --- Input ---
        processInput(window);

        // --- Rendering ---
        glClearColor(0.52f, 0.7f, 0.97f, 1.0f); // Set background colour to light blue
        glClear(GL_COLOR_BUFFER_BIT);

        float elapsedTime = (float)glfwGetTime(); // Get elapsed time since start

        // Render Turbine body
            bodyShader.use();

            // Draw turbine body
            glBindVertexArray(VAOs[1]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Render turbine
            turbineShader.use();

            // Create rotation transformation matrix along Z axis for turbine rotation
            glm::mat4 transform = glm::mat4(1.0f);
            // elapsedTime increases every frame(loop iteration) = new rotation angle every frame
            transform = glm::rotate(transform, elapsedTime * turbineSpeed, glm::vec3(0.0f, 0.0f, 1.0f));

            // Set turbine transformation
            unsigned int transformLoc = glGetUniformLocation(turbineShader.ID, "transform");
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform)); 

            // Draw turbine
            glBindVertexArray(VAOs[0]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Checking settings every ~5 seconds
        if (elapsedTime - lastSettingsCheckTime >= 5.0f) { 
            lastSettingsCheckTime = elapsedTime;
            handleSettings(readSettings(SETTINGS_PATHNAME));
        }

        // --- Event Checks, Buffer swapping ---
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Deallocate OpenGL resources
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteBuffers(2, EBOs);

    // Deallocate Glfw resources
    glfwTerminate();

    return 0;
}

// Window resize handler
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); // set OpenGl viewport to window size
}

// Input handler
void processInput(GLFWwindow* window) {
    // Exit program when ESCAPE key is pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Call setting handler for Settings
void handleSettings(std::vector<Setting> settings) {
    for (Setting setting : settings) {
        handleSetting(setting.name, setting.value);
    }
}

// Set settings
void handleSetting(std::string settingName, std::string value) {
    if (settingName == "speed") {
        try {
            float speedValue = std::stof(value);
            turbineSpeed = std::max(1.0f, std::min(speedValue, 10.0f)); // Clamp between 1 and 10
        } catch (const std::exception&) {
            turbineSpeed = 1.0f;
        }
    }
}