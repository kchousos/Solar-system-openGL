#include "../lib/glad/glad.h"
#include <GLFW/glfw3.h>

#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"

#include "../include/camera.h"
#include "../include/model.h"
#include "../include/shader.h"

#include <iostream>

// Animation flag
bool animation = true;

// Timing
float lastFrame = 0.0f;
float frameToggled = 0.0f;
float timeSinceLastToggle = 1.0f;

// Resolution settings
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

float earthOrbitRadius = 100.0f;
float moonOrbitRadius = 20.0f;
glm::vec3 sunPos = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 earthPos =
    sunPos + glm::vec3(sin(frameToggled) * earthOrbitRadius, 0.0f,
                       cos(frameToggled) * earthOrbitRadius);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float cameraOrbitRadius = 30.0f;
float rotateAngle = 1.0f;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyboardInput(GLFWwindow *window, float deltaTime);

int main() {
  // Initialize and configure GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
  if (window == NULL) {
    std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  // Load glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glEnable(GL_DEPTH_TEST);

  Shader planetShader("./src/planet.vs", "./src/planet.fs");
  Shader sunShader("./src/sun.vs", "./src/sun.fs");

  // Load the models
  Model Sun("./models/planet/planet.obj");
  Model Earth("./models/earth/Globe.obj");
  Model Moon("./models/rock/rock.obj");

  // Render Loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    timeSinceLastToggle += deltaTime;
    if (animation)
      frameToggled += deltaTime;

    keyboardInput(window, deltaTime);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // view / projection
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    // Render the sun object
    sunShader.use();
    sunShader.setMat4("projection", projection);
    sunShader.setMat4("view", view);

    model = glm::translate(model, sunPos); // Center it
    sunShader.setMat4("model", model);
    Sun.Draw(sunShader);

    planetShader.use();

    // Set the lighting
    planetShader.setVec3("viewPos", camera.Position);
    planetShader.setFloat("material.shininess", 32.0f);

    planetShader.setVec3("light.position", sunPos);
    planetShader.setVec3("light.ambient", 0.25f, 0.25f, 0.25f);
    planetShader.setVec3("light.diffuse", 1.8f, 1.8f, 1.8f);
    planetShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    planetShader.setFloat("pointLights[0].constant", 1.0f);
    planetShader.setFloat("pointLights[0].linear", 0.045);
    planetShader.setFloat("pointLights[0].quadratic", 0.0075);

    // Render the Earth object
    planetShader.setMat4("projection", projection);
    planetShader.setMat4("view", view);
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

    // Orbit around the sun
    earthPos = sunPos + glm::vec3(sin(frameToggled) * earthOrbitRadius, 0.0f,
                                  cos(frameToggled) * earthOrbitRadius);
    model = glm::translate(model, earthPos);
    // Rotate around itself
    model = glm::rotate(model, frameToggled * 1.5f * glm::radians(-50.0f),
                        glm::vec3(0.1f, 1.0f, 0.0f));

    planetShader.setMat4("model", model);
    Earth.Draw(planetShader);

    // Render the Moon object
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    // Orbit around the earth
    glm::vec3 moonPos =
        earthPos + glm::vec3(0.0f, sin(frameToggled) * moonOrbitRadius,
                             cos(frameToggled) * moonOrbitRadius);
    model = glm::translate(model, moonPos);
    planetShader.use();
    planetShader.setMat4("model", model);
    Moon.Draw(planetShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

// Handles user keyboard input. Supposed to be used every frame, so deltaTime
// can be calculated appropriately.
void keyboardInput(GLFWwindow *window, float deltaTime) {
  // Exit
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Upwards Rotation
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.Orbit(UP, cameraOrbitRadius, rotateAngle);

  // Downwards Rotation
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.Orbit(DOWN, cameraOrbitRadius, rotateAngle);

  // Rightwards Rotation
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.Orbit(RIGHT, cameraOrbitRadius, rotateAngle);

  // Leftwards Rotation
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.Orbit(LEFT, cameraOrbitRadius, rotateAngle);

  // Pause / Start
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    if (timeSinceLastToggle > 0.2) {
      animation = !animation;
      timeSinceLastToggle = 0.0f;
    }
  }
}
