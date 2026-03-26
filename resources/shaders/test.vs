#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 iNormal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // Matrice Projection * View de la lumière

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    iNormal = transpose(inverse(mat3(model))) * aNormal;
    TexCoords = aTexCoords;

    // Position du point par rapport à la lumière
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}