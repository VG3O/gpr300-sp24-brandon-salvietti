#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 _Model; 
uniform mat4 _ViewProjection;
uniform mat4 _LightSpace;

out Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
	vec4 WorldPosLightSpace; //Vertex position in light space for shadows
}vs_out;

void main(){
	//Transform vertex position to World Space.
	vs_out.WorldPos = vec3(_Model * vec4(vPos,1.0));

	//Transform vertex normal to world space using Normal Matrix
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;

	vs_out.WorldPosLightSpace = _LightSpace * vec4(vs_out.WorldPos, 1.0);

	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
