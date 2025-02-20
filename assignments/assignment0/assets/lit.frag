#version 450
out vec4 FragColor;
in Surface{
	vec3 WorldPos; 
	vec3 WorldNormal; 
	vec2 TexCoord;
	vec4 WorldPosLightSpace;
}fs_in;

uniform sampler2D _MainTex; 
uniform sampler2D _ShadowMap;

uniform vec3 _EyePos;
uniform vec3 _LightDirection;
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

uniform float _MinBias = 0.005;
uniform float _MaxBias = 0.05;

float CalculateShadow(vec4 lightSpacePosition){
	vec3 projection = lightSpacePosition.xyz / lightSpacePosition.w;
	projection = projection * 0.5 + 0.5;

	float closestDepth = texture(_ShadowMap, projection.xy).r;
	float currentDepth = projection.z;

	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	float bias = max(_MaxBias * (1.0 - dot(fs_in.WorldNormal, _LightDirection)), _MinBias);

	float shadow = 0.0;

	vec2 texelSize = 1.0 / textureSize(_ShadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(_ShadowMap, projection.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	return shadow;
}

void main(){
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight),0.0);
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	vec3 h = normalize(toLight + toEye);

	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);

	float shadow = CalculateShadow(fs_in.WorldPosLightSpace);

	vec3 lightColor = ((_AmbientColor * _Material.Ka) + (1.0 - shadow) * (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor)) * _LightColor;

	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;
	FragColor = vec4(objectColor * lightColor,1.0);
}
