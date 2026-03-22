#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 iNormal;

uniform vec3 lightDirection;
uniform int player;

void main()
{
    if(player == 1){
        vec3 color = vec3(0.6,0.6,0.6);
        float lightIntensity = dot(normalize(iNormal), normalize(lightDirection));
        vec3 intensity = color * lightIntensity;
        vec3 ambient = 0.2 * color;
        FragColor = vec4( ambient + intensity, 1.0);
    }
    else{
        float lightIntensity = dot(normalize(iNormal), normalize(lightDirection));
        vec3 intensity = iNormal * lightIntensity;
        vec3 ambient = 0.2 * iNormal;
        FragColor = vec4( ambient + intensity, 1.0);
    }
}