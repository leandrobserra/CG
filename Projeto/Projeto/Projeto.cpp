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
#include "PlanetRing.h"

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

void setShaderUniforms(GLuint programID, const glm::vec3& lightColor, const glm::vec3& lightPos, const glm::vec3& viewPos,
    float ambientStrength, float specularStrength, float shininess,
    const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) {
    glUniform3f(glGetUniformLocation(programID, "lightColor"), lightColor.r, lightColor.g, lightColor.b);
    glUniform3f(glGetUniformLocation(programID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(programID, "viewPos"), viewPos.x, viewPos.y, viewPos.z);

    glUniform1f(glGetUniformLocation(programID, "ambientStrength"), ambientStrength);
    glUniform1f(glGetUniformLocation(programID, "specularStrength"), specularStrength);
    glUniform1f(glGetUniformLocation(programID, "shininess"), shininess);

    glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(programID, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE, &model[0][0]);
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
    glm::mat4 MVP, Projection, View;

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

    double angle[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    float pos_earth = 50.0;
    double speed_factor = 10;

    auto orbitRadius = [pos_earth](double theta, double e, double rad) -> double { return pos_earth*rad*((1.0 - e * e) / (1.0 + e * std::cos(theta)));};
    auto delta_angle = [](double year_in_days, double seconds_on_day) -> double {return (2.0 * 3.14159 / (year_in_days * seconds_on_day)); };
    auto angular_speed = [speed_factor](double orbit_in_days) -> double {return ((2 * 3.14159) / orbit_in_days)* speed_factor; };

    glm::vec3 lightpos(0.0f, 0.0f, 0.0f);
    glm::vec3 lightcolor(1.0f, 1.0f, 1.0f);

  


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

        angle[2] += angular_speed(365.25);

        double radius = orbitRadius(3.14159 * 2 * angle[2] / 360, 0.017, 1);
       
        float x = radius * sin(3.14159 * 2 * angle[2] / 360);
        float y = radius * cos(3.14159 * 2 * angle[2] / 360);

        // Render Earth
        glm::mat4 earthModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        earthModelMatrix = glm::rotate(earthModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));

        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * earthModelMatrix;
        glm::vec3 viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, earthModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(earthTextureID, programID);
        renderSphere(1.0f, 36, 18);

        // Render Moon
        angle[8] += angular_speed(27.32);

        radius = orbitRadius(3.14159 * 2 * angle[8] / 360, 0.055, 0.1);

        float x_m = radius * sin(3.14159 * 2 * angle[8] / 360);
        float y_m = radius * cos(3.14159 * 2 * angle[8] / 360);

        glm::mat4 moonModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x+x_m, 0.0f, y+y_m));
        moonModelMatrix = glm::rotate(moonModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * moonModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, moonModelMatrix);


        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(moonTextureID, programID);
        renderSphere(0.55f, 36, 18);


        // Render Mars

        angle[3] += angular_speed(687);

        radius = orbitRadius(3.14159 * 2 * angle[3] / 360, 0.093, 1.524);

        x = radius * sin(3.14159 * 2 * angle[3] / 360);
        y = radius * cos(3.14159 * 2 * angle[3] / 360);

        glm::mat4 marsModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        marsModelMatrix = glm::rotate(marsModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * marsModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, marsModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(marsTextureID, programID);
        renderSphere(1.2f, 36, 18);

        // Render Sun
        glm::mat4 sunModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        sunModelMatrix = glm::rotate(sunModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * sunModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 1.f, 0.1f, 0.4f * 128.0f, Projection, View, sunModelMatrix);


        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(sunTextureID, programID);
        renderSphere(10.0f, 36, 18);

    
        // Render Venus

        angle[1] += angular_speed(224.70);

        radius = orbitRadius(3.14159 * 2 * angle[1] / 360, 0.007, 0.723);

        x = radius * sin(3.14159 * 2 * angle[1] / 360);
        y = radius * cos(3.14159 * 2 * angle[1] / 360);

        glm::mat4 venusModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        venusModelMatrix = glm::rotate(venusModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));

        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * venusModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, venusModelMatrix);


        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(venusTextureID, programID);
        renderSphere(0.95f, 36, 18);

        // Render Jupiter

        angle[4] += angular_speed(4328.9);

        radius = orbitRadius(3.14159 * 2 * angle[4] / 360, 0.007, 5.204);

        x = radius * sin(3.14159 * 2 * angle[4] / 360);
        y = radius * cos(3.14159 * 2 * angle[4] / 360);

        glm::mat4 jupiterModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        jupiterModelMatrix = glm::rotate(jupiterModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * jupiterModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, jupiterModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(jupiterTextureID, programID);
        renderSphere(4.2f, 36, 18);

        // Render Uranus
        angle[6] += angular_speed(84.01*365);

        radius = orbitRadius(3.14159 * 2 * angle[6] / 360, 0.046, 19.22);

        x = radius * sin(3.14159 * 2 * angle[6] / 360);
        y = radius * cos(3.14159 * 2 * angle[6] / 360);

        glm::mat4 uranusModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        uranusModelMatrix = glm::rotate(uranusModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * uranusModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, uranusModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(uranusTextureID, programID);
        renderSphere(2.9f, 36, 18);

        // Render Mercury

        angle[0] += angular_speed(87.97);

        radius = orbitRadius(3.14159 * 2 * angle[0] / 360, 0.206, 0.387);

        x = radius * sin(3.14159 * 2 * angle[0] / 360);
        y = radius * cos(3.14159 * 2 * angle[0] / 360);

        glm::mat4 mercuryModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        mercuryModelMatrix = glm::rotate(mercuryModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * mercuryModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, mercuryModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(mercuryTextureID, programID);
        renderSphere(0.383f, 36, 18);

        // Render Neptune

        angle[7] += angular_speed(164.8*365);

        radius = orbitRadius(3.14159 * 2 * angle[7] / 360, 0.01, 30.05);

        x = radius * sin(3.14159 * 2 * angle[7] / 360);
        y = radius * cos(3.14159 * 2 * angle[7] / 360);

        glm::mat4 neptuneModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        neptuneModelMatrix = glm::rotate(neptuneModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * neptuneModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, neptuneModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(neptuneTextureID, programID);
        renderSphere(0.78f, 36, 18);

        // Render Saturn
        angle[5] += angular_speed(29.46 * 365);

        radius = orbitRadius(3.14159 * 2 * angle[5] / 360, 0.056, 9.582);

        x = radius * sin(3.14159 * 2 * angle[5] / 360);
        y = radius * cos(3.14159 * 2 * angle[5] / 360);
        glm::mat4 saturnModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, y));
        saturnModelMatrix = glm::rotate(saturnModelMatrix, velocidade, glm::vec3(0.0f, 1.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * saturnModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, saturnModelMatrix);

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
