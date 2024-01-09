// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controlsProjeto.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
glm::mat4 ObjectModelMatrix;
glm::vec3 CameraPosition;
glm::vec3 CameraDirection;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}
glm::mat4 getObjectModelMatrix(){
	return ObjectModelMatrix;
}

glm::vec3 getCameraPosition() {
	return CameraPosition;
}

glm::vec3 getCameraDirection() {
	return CameraDirection;
}


// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : -0.3 (câmara ficar um pouco inclinado para baixo)
float verticalAngle = -0.3f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 140.0f; // 3 units / second
float mouseSpeed = 0.005f;

float zoom = 1.0f;


void computeMatricesFromInputs(glm::vec3& position){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024/2 - xpos );
	verticalAngle   += mouseSpeed * float( 768/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}

	// Aumentar Zoom
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		zoom += 0.1;
	}
	// Diminuir Zoom
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		zoom -=0.1;
	}

	zoom = glm::max(zoom, 0.1f);
	float FoV = initialFoV / zoom;

	//float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 4000.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	ObjectModelMatrix = glm::mat4(1.0f); // Identity matrix as a placeholder

	CameraPosition = position;
	
	CameraDirection = direction;

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}