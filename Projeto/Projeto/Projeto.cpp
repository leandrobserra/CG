#define STB_IMAGE_IMPLEMENTATION
#include <common/stb_image.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "common/shader.hpp"
#include "common/texture.hpp"
#include <common/controls.hpp>
#include "Sphere.h"

GLFWwindow* window;
unsigned int loadTexture(char const* path);

bool initializeGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    return true;
}

bool createWindow() {
    window = glfwCreateWindow(1024, 740, "Projeto", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open GLFW window.\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    return true;
}

bool initializeGLEW() {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }
    return true;
}

bool initializeOpenGL() {
    if (!initializeGLFW() || !createWindow() || !initializeGLEW()) { return false; }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
}

void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void renderSphere(float r, int sectors, int stacks) {
    Sphere sphere(r, sectors, stacks);
    sphere.Draw();
}

bool rodar = false;
float velocidade = 0.0f;


void setTexture(GLuint textureID, GLuint programID) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);
}




int main() {
    if (!initializeOpenGL()) { return -1; }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    glClearColor(0.0f, 0.0f, 0.3f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    GLuint programID = LoadShaders("shaders/TransformVertexShader.vertexshader", "shaders/TextureFragmentShader.fragmentshader");

    // Get the uniform location for the MVP matrix
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glm::mat4 MVP;

    glUseProgram(programID);
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Load textures for Earth, Mars, Sun, Moon, and Venus
    GLuint earthTextureID = loadTexture("texturas/earth.jpg");
    GLuint marsTextureID = loadTexture("texturas/mars.jpg");
    GLuint sunTextureID = loadTexture("texturas/sun.jpg");
    GLuint moonTextureID = loadTexture("texturas/moon.jpg");
    GLuint venusTextureID = loadTexture("texturas/venus.jpg");
    GLuint jupiterTextureID = loadTexture("texturas/jupiter.jpg");
    GLuint uranusTextureID = loadTexture("texturas/uranus.jpg");
    GLuint mercuryTextureID = loadTexture("texturas/mercury.jpg");
    GLuint neptuneTextureID = loadTexture("texturas/neptune.jpg");
    GLuint saturnTextureID = loadTexture("texturas/saturn.jpg");
    GLuint saturnRingTextureID = loadTexture("texturas/saturn_ring.png");


    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        computeMatricesFromInputs();

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS or rodar == true) {
            velocidade += 0.01f;
            rodar = true;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            rodar = false;
        }

        // Render Earth
        glm::mat4 earthModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-24.0f, 0.0f, 0.0f));
        earthModelMatrix = glm::rotate(earthModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * earthModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(earthTextureID, programID);
        renderSphere(1.0f, 36, 18);

        // Render Mars
        glm::mat4 marsModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-12.0f, 0.0f, 0.0f));
        marsModelMatrix = glm::rotate(marsModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * marsModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(marsTextureID, programID);
        renderSphere(1.2f, 36, 18);

        // Render Sun
        glm::mat4 sunModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        sunModelMatrix = glm::rotate(sunModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * sunModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(sunTextureID, programID);
        renderSphere(10.0f, 36, 18);

        // Render Moon
        glm::mat4 moonModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(12.0f, 0.0f, 0.0f));
        moonModelMatrix = glm::rotate(moonModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * moonModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(moonTextureID, programID);
        renderSphere(0.55f, 36, 18);

        // Render Venus
        glm::mat4 venusModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(24.0f, 0.0f, 0.0f));
        venusModelMatrix = glm::rotate(venusModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * venusModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(venusTextureID, programID);
        renderSphere(0.95f, 36, 18);

        // Render Jupiter
        glm::mat4 jupiterModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-36.0f, 0.0f, 0.0f));
        jupiterModelMatrix = glm::rotate(jupiterModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * jupiterModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(jupiterTextureID, programID);
        renderSphere(4.2f, 36, 18);

        // Render Uranus
        glm::mat4 uranusModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-48.0f, 0.0f, 0.0f));
        uranusModelMatrix = glm::rotate(uranusModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * uranusModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(uranusTextureID, programID);
        renderSphere(2.9f, 36, 18);

        // Render Mercury
        glm::mat4 mercuryModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(18.0f, 0.0f, 0.0f));
        mercuryModelMatrix = glm::rotate(mercuryModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * mercuryModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(mercuryTextureID, programID);
        renderSphere(2.8f, 36, 18);

        // Render Neptune
        glm::mat4 neptuneModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(24.0f, 0.0f, 0.0f));
        neptuneModelMatrix = glm::rotate(neptuneModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * neptuneModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(neptuneTextureID, programID);
        renderSphere(0.78f, 36, 18);

        // Render Saturn
        glm::mat4 saturnModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 0.0f, 0.0f));
        saturnModelMatrix = glm::rotate(saturnModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = getProjectionMatrix() * getViewMatrix() * saturnModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(saturnTextureID, programID);
        renderSphere(3.7f, 36, 18);


        // Render Saturn Ring
        


        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window));

    cleanup();
    return 0;
}