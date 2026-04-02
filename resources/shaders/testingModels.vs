#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

out vec2 TexCoords;
out vec3 iNormal;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Position dans l'espace "Monde" (utile pour la lumière)
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transfert des coordonnées de texture
    TexCoords = aTexCoords;

    // Dans le Vertex Shader
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(aBiTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    TBN = mat3(T, B, N);

    // On transforme la normale pour qu'elle suive la rotation de l'objet
    // mat3(transpose(inverse(model))) évite les bugs de normales si l'objet subit un scale
    iNormal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Position finale à l'écran
    gl_Position = projection * view * vec4(FragPos, 1.0);
}