#version 450
out vec4 FragColor;

in vec2 UV;

uniform sampler2D depthMap;
void main() {
	float depth = texture(depthMap, UV).r;
	FragColor = vec4(vec3(depth),1.0);
}