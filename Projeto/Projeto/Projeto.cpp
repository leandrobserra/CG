#define STB_IMAGE_IMPLEMENTATION
#include <common/stb_image.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "include/shader.hpp"
#include "include/texture.hpp"
#include "include/controlsProjeto.hpp"
#include "Sphere.h"
#include <map>
#include <glm/gtc/type_ptr.hpp>



GLFWwindow* window;
float SCREEN_WIDTH = 1024, SCREEN_HEIGHT = 740;

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;

struct PlanetInfo {
    std::string Name;
    std::string OrbitSpeed;
    std::string Mass;
    std::string Gravity;
};
PlanetInfo Info;

GLuint textVAO, textVBO;

unsigned int loadTexture(char const* path);
void RenderText(GLuint programID2, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

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
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Projeto", nullptr, nullptr);
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

void RenderText(GLuint programID2, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{   
    std::cout << text << std::endl;
    // Activate corresponding render state
    glUseProgram(programID2);
    glUniform3f(glGetUniformLocation(programID2, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ShowInfo(GLuint programID2)
{
    RenderText(programID2, "Planeta: " + Info.Name, 25.0f, SCREEN_HEIGHT - 30.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
    RenderText(programID2, "Velocidade Orbital Media (km/s): " + Info.OrbitSpeed, 25.0f, SCREEN_HEIGHT - 50.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
    RenderText(programID2, "Massa (kg * 10^24): " + Info.Mass, 25.0f, SCREEN_HEIGHT - 70.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
    RenderText(programID2, "Gravidade (g): " + Info.Gravity, 25.0f, SCREEN_HEIGHT - 90.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
}


int main() {
    if (!initializeOpenGL()) { return -1; }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    GLuint programID = LoadShaders("shaders/TransformVertexShader.vertexshader", "shaders/TextureFragmentShader.fragmentshader");

    // Get the uniform location for the MVP matrix
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glm::mat4 MVP, Projection, View;

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    GLuint programID2 = LoadShaders("shaders/TextShader.vertexshader", "shaders/TextShader.fragmentshader");
    // PROJECTION FOR TEXT RENDER
    glm::mat4 Text_projection = glm::ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(programID2, "projection"), 1, GL_FALSE, glm::value_ptr(Text_projection));

    /* TEXT RENDERING VAO-VBO*/
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    /* TEXT RENDERING VAO-VBO*/


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
    GLuint celestialSkyID = loadTexture("texturas/sky2.png");

    double angle[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 , 0.0 };
    float pos_earth = 50.0;
    double speed_factor = 10;

    auto orbitRadius = [pos_earth](double theta, double e, double rad) -> double { return pos_earth * rad * ((1.0 - e * e) / (1.0 + e * std::cos(theta)));};
    auto delta_angle = [](double year_in_days, double seconds_on_day) -> double {return (2.0 * 3.14159 / (year_in_days * seconds_on_day)); };
    auto angular_speed = [speed_factor](double orbit_in_days) -> double {return ((2 * 3.14159) / orbit_in_days) * speed_factor; };
    bool rodar = true;
    float velocidade[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 , 0.0 };
    float x[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 , 0.0 };
    float y[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 , 0.0 };
    float escala = 0.00005;

    glm::vec3 lightpos(0.0f, 0.0f, 0.0f);
    glm::vec3 lightcolor(1.0f, 1.0f, 1.0f);
    
    //Posição inicial da câmara
    glm::vec3 position = glm::vec3(-5.5, 35.5, 85.5);

    int planetaSelecionado = 0;



    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        computeMatricesFromInputs(position);
        glm::vec3 DirecaoCamera = getCameraDirection();
        double radius = orbitRadius(3.14159 * 2 * angle[2] / 360, 0.017, 1);
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS or rodar == true) {

            //Translação Terra
            angle[2] += angular_speed(365.25);
            x[2] = radius * sin(3.14159 * 2 * angle[2] / 360);
            y[2] = radius * cos(3.14159 * 2 * angle[2] / 360);
            //Rotação
            velocidade[2] += 1574 * escala;


            //Translação Marte
            angle[3] += angular_speed(687);
            radius = orbitRadius(3.14159 * 2 * angle[3] / 360, 0.093, 1.524);
            x[3] = radius * sin(3.14159 * 2 * angle[3] / 360);
            y[3] = radius * cos(3.14159 * 2 * angle[3] / 360);
            //Rotação
            velocidade[3] += 866 * escala;

            //Translação Venus
            angle[1] += angular_speed(224.70);
            radius = orbitRadius(3.14159 * 2 * angle[1] / 360, 0.007, 0.723);
            x[1] = radius * sin(3.14159 * 2 * angle[1] / 360);
            y[1] = radius * cos(3.14159 * 2 * angle[1] / 360);
            //Rotação
            velocidade[1] += 1.52 * escala;


            //Translação Jupiter
            angle[4] += angular_speed(4328.9);
            radius = orbitRadius(3.14159 * 2 * angle[4] / 360, 0.007, 5.204);
            x[4] = radius * sin(3.14159 * 2 * angle[4] / 360);
            y[4] = radius * cos(3.14159 * 2 * angle[4] / 360);
            //Rotação
            velocidade[4] += 45583 * escala;


            //Translação Urano
            angle[6] += angular_speed(84.01 * 365);

            radius = orbitRadius(3.14159 * 2 * angle[6] / 360, 0.046, 19.22);

            x[6] = radius * sin(3.14159 * 2 * angle[6] / 360);
            y[6] = radius * cos(3.14159 * 2 * angle[6] / 360);
            //Rotação
            velocidade[6] += 14794 * escala;


            //Translação Mercurio
            angle[0] += angular_speed(87.97);

            radius = orbitRadius(3.14159 * 2 * angle[0] / 360, 0.206, 0.387);

            x[0] = radius * sin(3.14159 * 2 * angle[0] / 360);
            y[0] = radius * cos(3.14159 * 2 * angle[0] / 360);
            //Rotação
            velocidade[0] += 10.83 * escala;


            //Translação Neptuno
            angle[7] += angular_speed(164.8 * 365);

            radius = orbitRadius(3.14159 * 2 * angle[7] / 360, 0.01, 30.05);

            x[7] = radius * sin(3.14159 * 2 * angle[7] / 360);
            y[7] = radius * cos(3.14159 * 2 * angle[7] / 360);
            //Rotação
            velocidade[7] += 9719 * escala;


            //Translação Saturno
            angle[5] += angular_speed(29.46 * 365);

            radius = orbitRadius(3.14159 * 2 * angle[5] / 360, 0.056, 9.582);

            x[5] = radius * sin(3.14159 * 2 * angle[5] / 360);
            y[5] = radius * cos(3.14159 * 2 * angle[5] / 360);
            //Rotação
            velocidade[5] += 36840 * escala;


            //Rotação do Sol
            velocidade[8] += 1574 / 30 * escala;

            //Translação Lua
            angle[8] += angular_speed(27.32);
            radius = orbitRadius(3.14159 * 2 * angle[8] / 360, 0.055, 0.1);
            x[8] = radius * sin(3.14159 * 2 * angle[8] / 360);
            y[8] = radius * cos(3.14159 * 2 * angle[8] / 360);

            rodar = true;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            rodar = false;
        }


        glUseProgram(programID);
        // Render Earth
        glm::mat4 earthModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[2], 0.0f, y[2]));
        earthModelMatrix = glm::rotate(earthModelMatrix, velocidade[2], glm::vec3(0.0f, 1.0f, 0.0f));

        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * earthModelMatrix;
        glm::vec3 viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, earthModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(earthTextureID, programID);
        renderSphere(1.0f, 36, 18);



        // Render Moon
        glm::mat4 moonModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[2] + x[8], 0.0f, y[2] + y[8]));
        moonModelMatrix = glm::rotate(moonModelMatrix, velocidade[2], glm::vec3(0.0f, 1.0f, 0.0f));

        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * moonModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, moonModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(moonTextureID, programID);
        renderSphere(0.55f, 36, 18);



        // Render Mars
        glm::mat4 marsModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[3], 0.0f, y[3]));
        marsModelMatrix = glm::rotate(marsModelMatrix, velocidade[3], glm::vec3(0.0f, 1.0f, 0.0f));
        
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
        sunModelMatrix = glm::rotate(sunModelMatrix, velocidade[8], glm::vec3(0.0f, 1.0f, 0.0f));
       
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * sunModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 1.f, 0.1f, 0.4f * 128.0f, Projection, View, sunModelMatrix);


        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(sunTextureID, programID);
        renderSphere(10.0f, 36, 18);



        // Render Venus
        glm::mat4 venusModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[1], 0.0f, y[1]));
        venusModelMatrix = glm::rotate(venusModelMatrix, velocidade[1], glm::vec3(0.0f, 1.0f, 0.0f));
       
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * venusModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, venusModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(venusTextureID, programID);
        renderSphere(0.95f, 36, 18);



        // Render Jupiter
        glm::mat4 jupiterModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[4], 0.0f, y[4]));
        jupiterModelMatrix = glm::rotate(jupiterModelMatrix, velocidade[4], glm::vec3(0.0f, 1.0f, 0.0f));
        
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * jupiterModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, jupiterModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(jupiterTextureID, programID);
        renderSphere(4.2f, 36, 18);



        // Render Uranus
        glm::mat4 uranusModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[6], 0.0f, y[6]));
        uranusModelMatrix = glm::rotate(uranusModelMatrix, velocidade[6], glm::vec3(0.0f, 1.0f, 0.0f));
        
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * uranusModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, uranusModelMatrix);


        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(uranusTextureID, programID);
        renderSphere(2.9f, 36, 18);



        // Render Mercury
        glm::mat4 mercuryModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[0], 0.0f, y[0]));
        mercuryModelMatrix = glm::rotate(mercuryModelMatrix, velocidade[0], glm::vec3(0.0f, 1.0f, 0.0f));
        
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * mercuryModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, mercuryModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(mercuryTextureID, programID);
        renderSphere(0.383f, 36, 18);



        // Render Neptune
        glm::mat4 neptuneModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[7], 0.0f, y[7]));
        neptuneModelMatrix = glm::rotate(neptuneModelMatrix, velocidade[7], glm::vec3(0.0f, 1.0f, 0.0f));
        
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * neptuneModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, neptuneModelMatrix);


        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(neptuneTextureID, programID);
        renderSphere(0.78f, 36, 18);



        // Render Saturn
        glm::mat4 saturnModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x[5], 0.0f, y[5]));
        saturnModelMatrix = glm::rotate(saturnModelMatrix, velocidade[5], glm::vec3(0.0f, 1.0f, 0.0f));
        
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * saturnModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, lightpos, viewPos, 0.5f, 0.1f, 0.4f * 128.0f, Projection, View, saturnModelMatrix);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(saturnTextureID, programID);
        renderSphere(3.7f, 36, 18);


        // Render Saturn Ring




        //render sky
        glm::mat4 skyModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        Projection = getProjectionMatrix();
        View = getViewMatrix();
        MVP = Projection * View * skyModelMatrix;
        viewPos = getCameraPosition();

        setShaderUniforms(programID, lightcolor, glm::vec3(), viewPos, 1.0f, 0.f, 0.0f, Projection, View, skyModelMatrix);
        neptuneModelMatrix = glm::rotate(skyModelMatrix, velocidade[7], glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        setTexture(celestialSkyID, programID);
        renderSphere(2000.0f, 36, 18);




        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS or planetaSelecionado == 1) {
            position = glm::vec3(x[0]+4.4, 0, y[0]+4.4);
            planetaSelecionado = 1;
            Info.Name = "Mercurio";
            Info.OrbitSpeed = "47,87";
            Info.Mass = "0.32868";
            Info.Gravity = "0.38"; //Em Mercúrio, a gravidade é cerca de 0,38 vezes a gravidade na Terra

            ShowInfo(programID2);


        }if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS or planetaSelecionado == 2) {
            position = glm::vec3(x[1] + 6.4, 0, y[1] + 6.4);
            planetaSelecionado = 2;

            Info.Name = "Venus";
            Info.OrbitSpeed = "35,02";
            Info.Mass = "0.32868";
            Info.Gravity = "0.90";

            ShowInfo(programID2);

        }if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS or planetaSelecionado == 3) {
            position = glm::vec3(x[2] + 6.4, 0, y[2] + 6.4);
            planetaSelecionado = 3;

            Info.Name = "Terra";
            Info.OrbitSpeed = "29,76";
            Info.Mass = "5.97600";
            Info.Gravity = "1";

            ShowInfo(programID2);

            }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS or planetaSelecionado == 4) {
            position = glm::vec3(x[3] + 6.4, 0, y[3] + 6.4);
            planetaSelecionado = 4;

            Info.Name = "Marte";
            Info.OrbitSpeed = "24,13";
            Info.Mass = "0.63345";
            Info.Gravity = "0.38";

            ShowInfo(programID2);

            }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS or planetaSelecionado == 5) {
            position = glm::vec3(x[4] + 15.4, 0, y[4] + 20.4);
            planetaSelecionado = 5;

            Info.Name = "Jupiter";
            Info.OrbitSpeed = "13,07";
            Info.Mass = "1876.64328";
            Info.Gravity = "2.55";

            ShowInfo(programID2);

        }if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS or planetaSelecionado == 6) {

            position = glm::vec3(x[5] + 20.4, 0, y[5] + 20.4);
            planetaSelecionado = 6;

            Info.Name = "Saturno";
            Info.OrbitSpeed = "9,67";
            Info.Mass = "561.80376";
            Info.Gravity = "1.12";

            ShowInfo(programID2);

        }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS or planetaSelecionado == 7) {

            position = glm::vec3(x[6] + 15.4, 0, y[6] + 15.4);
            planetaSelecionado = 7;

            Info.Name = "Urano";
            Info.OrbitSpeed = "6,84";
            Info.Mass = "86.05440";
            Info.Gravity = "0.97";

            ShowInfo(programID2);

        }
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS or planetaSelecionado == 8) {
            position = glm::vec3(x[7] + 4.4, 0, y[7] + 4.4);
            planetaSelecionado = 8;

            Info.Name = "Neptuno";
            Info.OrbitSpeed = "5,48";
            Info.Mass = "101.59200";
            Info.Gravity = "1.17";

            ShowInfo(programID2);

        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            planetaSelecionado = 0;
        }



        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window));

    cleanup();
    return 0;
}