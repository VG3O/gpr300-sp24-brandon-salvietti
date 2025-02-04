#version 450
out vec4 FragColor;
in vec2 UV;

uniform sampler2D _HDRTexture;
void main()
{
    vec3 hdrColor = texture(_HDRTexture, UV).rgb;      

    FragColor = vec4(hdrColor, 1.0);
} 