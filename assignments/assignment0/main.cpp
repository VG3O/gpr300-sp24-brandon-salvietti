#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// ew includes
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/transform.h>
#include <ew/camera.h>
#include <ew/procGen.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/Framebuffer.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

void resetCamera(ew::Camera* camera, ew::CameraController* controller)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

struct Material
{
	float Ambient = 1.0;
	float Diffuse = 0.5;
	float Specular = 0.5;
	float Shininess = 128;
} material;

ew::CameraController cameraControl;
ew::Camera camera;

// ImGui variables
bool postProcessEnabled = false;
int postProcessEffect = 1;
glm::vec3 backgroundColor = glm::vec3(0.6f, 0.8f, 0.92f);


int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	vg3o::ScreenBuffer::genScreenQuad();

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader screenShader = ew::Shader("assets/screen.vert", "assets/screen.frag");
	ew::Shader postShader = ew::Shader("assets/screen.vert", "assets/effects.frag");

	ew::Model monkey = ew::Model("assets/Suzanne.obj");
	ew::Transform monkeyTransform;

	ew::Mesh quad = ew::createPlane(10.0f, 10.0f, 1);
	ew::Transform floorTransform;
	floorTransform.position = glm::vec3(0.0f, -2.0f, 0.0f);

	GLuint brickTex = ew::loadTexture("assets/brick_color.jpg");
	GLuint grassTex = ew::loadTexture("assets/grass.jpg");
	
	vg3o::ScreenBuffer framebuffer(screenWidth, screenHeight, 1);
	vg3o::ScreenBuffer depthMap(screenWidth, screenHeight, 1, true);

	// Global settings
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		// RENDER DEPTH MAP
		/*depthMap.useBuffer();
		glClear(GL_DEPTH_BUFFER_BIT);
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, camera.nearPlane, camera.farPlane);
		glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, -1.0f, 0.0f),
										  glm::vec3(0.0f, 0.0f, 0.0f),
										  glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;*/


		// RENDER MAIN SCENE
		framebuffer.useBuffer();
		glEnable(GL_DEPTH_TEST);
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindTextureUnit(0, brickTex);
		glBindTextureUnit(1, grassTex);

		shader.use();
		// lighting & materials
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ambient);
		shader.setFloat("_Material.Kd", material.Diffuse);
		shader.setFloat("_Material.Ks", material.Specular);
		shader.setFloat("_Material.Shininess", material.Shininess);

		// texture
		shader.setInt("_MainTex", 0);
		cameraControl.move(window, &camera, deltaTime);
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
		
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		monkey.draw();

		
		shader.setMat4("_Model", floorTransform.modelMatrix());
		shader.setInt("_MainTex", 1);
		quad.draw();

		// FINISH RENDER

		framebuffer.useDefaultBuffer(); // go back to the framebuffer we want to draw on screen

		if (!postProcessEnabled) screenShader.use();
		else
		{
			postShader.use();
			postShader.setInt("_PostProcessEffect", postProcessEffect);
		}

		glDisable(GL_DEPTH_TEST); // disable depth testing cause we want the framebuffer to be on top of everything else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.getColorBuffers()[0]);
		vg3o::ScreenBuffer::draw();

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	ImGui::ColorPicker3("Background Color", &backgroundColor.x);

	ImGui::Separator();

	if (ImGui::Button("Reset Camera"))
	{
		resetCamera(&camera, &cameraControl);
	}

	if (ImGui::CollapsingHeader("Material Settings")) {
		ImGui::SliderFloat("AmbientK", &material.Ambient, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Diffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Specular, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	ImGui::Separator();
	ImGui::Checkbox("Enable Post Processing", &postProcessEnabled);
	if (postProcessEnabled)
	{
		ImGui::SliderInt("Effect Type", &postProcessEffect, 0, 1);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

