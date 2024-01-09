#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs(glm::vec3& position);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::mat4 getObjectModelMatrix();
glm::vec3 getCameraPosition();
glm::vec3 getCameraDirection();

#endif