#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 iNormal;
in vec3 FragPos;

uniform vec3 lightDirection;
uniform sampler2D texture_diffuse1;

void main()
{
    vec3 norm = normalize(iNormal);
    vec3 lightDir = normalize(lightDirection);

    // Détermination de la couleur de base (Albedo)
    vec4 color = texture(texture_diffuse1, TexCoords);

    float ambientStrength = 0.3;

    // 2. Diffuse
    float diff = max(dot(norm, lightDir), 0.0);

    // Calcul final
    vec3 result = (ambientStrength * color.rgb) + (diff * color.rgb);

    FragColor = vec4(result, color.a);
}