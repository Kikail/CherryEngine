#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 iNormal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform float uTime;
uniform int player;


void main()
{
    vec3 lightDir = normalize(lightDirection);

    vec3 color;
    if(player == 1) {
        color = vec3(abs(cos(uTime)), abs(sin(uTime)), 0.0);
    } else {
        color = vec3(0.7);
    }

    float ambient = 0.3;
    float intensity = max(0.0, dot(iNormal, lightDir));
    vec3 diffuse = (ambient * color) + (intensity * color);

    FragColor = vec4(diffuse, 1.0);
}