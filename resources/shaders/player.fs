#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 iNormal;

uniform vec3 lightDirection;

void main()
{
    vec3 playerColor = vec3(0.6,0.6,0.6);
    float lightIntensity = dot(normalize(iNormal), normalize(lightDirection));
    vec3 intensity = playerColor * lightIntensity;
    vec3 ambient = 0.2 * playerColor;
    FragColor = vec4( ambient + intensity, 1.0);
}