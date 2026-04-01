#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 iNormal;
in vec3 FragPos;

uniform vec3 lightDirection;

void main()
{
    // On s'assure que les vecteurs sont bien unitaires
    vec3 norm = normalize(iNormal);
    vec3 lightDir = normalize(lightDirection);

    // Détermination de la couleur de base (Albedo)
    vec4 color = vec4(1.0,1.0,0.0,1.0);

    // --- Éclairage Directionnel ---

    // 1. Ambiant (Lumière minimum pour ne pas avoir de noir pur)
    float ambientStrength = 0.3;

    // 2. Diffuse (Lumière selon l'angle de la normale)
    // Note : On utilise -lightDir car lightDirection pointe souvent VERS l'objet
    float diff = max(dot(norm, lightDir), 0.0);

    // Calcul final selon ta formule : (Ambiant + Diffuse) * Couleur
    vec3 result = (ambientStrength * color.rgb) + (diff * color.rgb);

    FragColor = vec4(result, color.a);
}