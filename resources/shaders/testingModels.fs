#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN; // Reçue du Vertex Shader

uniform vec3 lightDirection;
uniform sampler2D diffuse;
uniform sampler2D normal;

void main()
{
    // 1. Extraire la normale de la map (Valeurs entre [0,1])
    vec3 localNormal = texture(normal, TexCoords).rgb;

    // 2. Transformer vers l'espace [-1, 1]
    localNormal = normalize(localNormal * 2.0 - 1.0);

    // 3. Transformer la normale vers l'espace Monde via la matrice TBN
    vec3 worldNormal = normalize(TBN * localNormal);

    // 4. Calcul de l'éclairage avec la nouvelle normale
    vec4 texColor = texture(diffuse, TexCoords);
    vec3 lightDir = normalize(lightDirection);

    float ambientStrength = 0.3;
    float diff = max(dot(worldNormal, lightDir), 0.0);

    vec3 ambient = ambientStrength * texColor.rgb;
    vec3 diffuseRes = diff * texColor.rgb;

    FragColor = vec4(ambient + diffuseRes, texColor.a);
}