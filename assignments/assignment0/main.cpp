#include <stdio.h>
#include <math.h>
#include <iostream>

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
#include <ew/Animation.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1600;
int screenHeight = 850;
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
glm::vec3 lightDirection = glm::vec3(0.0f, -1.0f, -0.2f);

float maxBias = 0.005, minBias = 0.005;

unsigned int depthTexture;

//vg3o::Animation* scale = new vg3o::Animation();
//
//vg3o::Animation* rotation = new vg3o::Animation();

vg3o::Animator animator;
vg3o::Animation* posAnimation = new vg3o::Animation();
vg3o::Animation* rotAnimation = new vg3o::Animation();
vg3o::Animation* scaleAnimation = new vg3o::Animation();

int main() {
	GLFWwindow* window = initWindow("Assignment 4", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	vg3o::ScreenBuffer::genScreenQuad();

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	
	// shaders

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader depthShader = ew::Shader("assets/depthShader.vert", "assets/empty.frag");
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

	depthTexture = depthMap.getDepthTexture();

	// animations
	posAnimation->AddKeyframe(vg3o::Keyframe(0, glm::vec3(0, 0, 0), vg3o::QUINTIC));
	posAnimation->AddKeyframe(vg3o::Keyframe(1, glm::vec3(0, 2, 1), vg3o::BACK));
	posAnimation->AddKeyframe(vg3o::Keyframe(3, glm::vec3(-2, -0.5f, -1), vg3o::BACK));

	rotAnimation->AddKeyframe(vg3o::Keyframe(0, glm::vec3(0, 0, 0), vg3o::EXPONENTIAL));
	rotAnimation->AddKeyframe(vg3o::Keyframe(2.5, glm::vec3(0, 100, 0), vg3o::CIRCULAR));
	rotAnimation->AddKeyframe(vg3o::Keyframe(5, glm::vec3(180, -60, 0), vg3o::BACK));

	scaleAnimation->AddKeyframe(vg3o::Keyframe(0, glm::vec3(1, 1, 1), vg3o::EXPONENTIAL));
	scaleAnimation->AddKeyframe(vg3o::Keyframe(1, glm::vec3(0.5, 3, 0.5), vg3o::SINE));
	scaleAnimation->AddKeyframe(vg3o::Keyframe(7, glm::vec3(2, 0.5, 3), vg3o::LINEAR));

	// animator
	animator.SetAnimation(posAnimation, 1);
	animator.SetAnimation(rotAnimation, 2);
	animator.SetAnimation(scaleAnimation, 3);
	animator.Loop(true);
	animator.Play();

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

		// UPDATE TRANSFORMS

		cameraControl.move(window, &camera, deltaTime);

		ew::Transform animatorUpdateTransform = animator.UpdateAnimations(deltaTime);
		monkeyTransform = animatorUpdateTransform;

		// RENDER DEPTH MAP
		depthMap.useBuffer();
		depthShader.use();
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_FRONT);

		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.5f, 50.f);
		glm::mat4 lightView = glm::lookAt(lightDirection,
			glm::vec3(0.0f,5.0f,0.0f), // camera
										  
										  glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		
		depthShader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);
		depthShader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkey.draw();

		
		depthShader.setMat4("_Model", floorTransform.modelMatrix());
		quad.draw();


		// RENDER MAIN SCENE
		framebuffer.useBuffer();
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);

		glBindTextureUnit(0, brickTex);
		glBindTextureUnit(1, grassTex);
		glBindTextureUnit(2, depthMap.getDepthTexture());

		shader.use();

		// lighting & materials
		shader.setVec3("_LightDirection", lightDirection);
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ambient);
		shader.setFloat("_Material.Kd", material.Diffuse);
		shader.setFloat("_Material.Ks", material.Specular);
		shader.setFloat("_Material.Ks", material.Specular);
		shader.setFloat("_Material.Shininess", material.Shininess);

		shader.setFloat("_MaxBias", maxBias);
		shader.setFloat("_MinBias", minBias);

		shader.setMat4("_LightSpace", lightSpaceMatrix);

		//texture
		shader.setInt("_MainTex", 0);
		shader.setInt("_ShadowMap", 2);
		
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		monkey.draw();

		
		shader.setMat4("_Model", floorTransform.modelMatrix());
		shader.setInt("_MainTex", 1);
		quad.draw();

		// FINISH RENDER

		framebuffer.useDefaultBuffer(); // go back to the framebuffer we want to draw on screen

		if (!postProcessEnabled) { screenShader.use();/* screenShader.setInt("depthTexture", 0);*/ }
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
	vg3o::Animation::Cleanup();
}

void KeyframeListImGui(std::string name, vg3o::Animation* animation, int& id)
{
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		std::vector<vg3o::Keyframe>& keyframes = animation->GetKeyframes();
		for (int i = 0; i < keyframes.size(); i++)
		{
			id++;
			ImGui::PushID(id);
			std::string name = "Keyframe " + std::to_string(i + 1);
			if (ImGui::CollapsingHeader(name.c_str()))
			{
				vg3o::Keyframe& keyframe = keyframes[i];
				ImGui::DragFloat("Time", &keyframe.time, 0.01f);
				ImGui::DragFloat3("Value", &keyframe.value.x, 0.05f);
				ImGui::Combo("Easing Style", &keyframe.easeInt, vg3o::EasingNames, 10);
				ImGui::Checkbox("Ease In", &keyframe.easeIn);
				keyframe.ease = (vg3o::EasingStyle)keyframe.easeInt;
			}
			ImGui::PopID();
		}
		if (ImGui::Button("Add Keyframe", ImVec2(100, 20))) { animation->AddKeyframe(vg3o::Keyframe(keyframes.size(), glm::vec3(0, 0, 0), vg3o::QUADRATIC)); }
		ImGui::SameLine(115);
		if (ImGui::Button("Remove Keyframe", ImVec2(115, 20))) { animation->PopKeyframe(); }
	}
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	{ // Main Settings
		ImGui::Begin("Settings");
		//ImGui::ColorPicker3("Background Color", &backgroundColor.x);
		ImGui::DragFloat3("Light Direction", &lightDirection.x, 0.001f, -10.f, 10.f);

		ImGui::SliderFloat("Max Bias", &maxBias, 0.f, 1.f);
		ImGui::SliderFloat("Min Bias", &minBias, 0.f, 1.f);

		ImGui::BeginChild("Shadow Map");
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::Image((ImTextureID)depthTexture, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndChild();

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
	}
	{ // Animator Settings
		ImGui::Begin("Animator");

		ImGui::SliderFloat("Playback Time", &animator.playbackTime, 0.f, animator.maximalDuration);
		ImGui::SliderFloat("Playback Speed", &animator.playbackSpeed, -3.f, 4.f);
		ImGui::Checkbox("Loop", &animator.looping);
		
		if (ImGui::Button("Play", ImVec2(80, 25))) animator.Play();
		ImGui::SameLine(100.f);
		if (ImGui::Button("Pause", ImVec2(80, 25))) animator.Pause();
		ImGui::SameLine(192.f);
		if (ImGui::Button("Stop", ImVec2(80, 25))) animator.Stop();

		int id = 0;
		ImGui::Separator();
		KeyframeListImGui("Position Keyframes", posAnimation, id);
		ImGui::Separator();
		KeyframeListImGui("Rotation Keyframes", rotAnimation, id);
		ImGui::Separator();
		KeyframeListImGui("Scale Keyframes", scaleAnimation, id);

		ImGui::End();
	}

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

