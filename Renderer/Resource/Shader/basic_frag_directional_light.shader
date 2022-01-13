#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

in vec4 fragPosLightSpace;

out vec4 fColor;
out vec3 color;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform float constant;
uniform float linear_;
uniform float quadratic;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D depthMapTexture;


//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 1.5f;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    float dot_product = dot(normalEye, lightDirN);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    if (dot_product < 0) {
        color = ambient * texture(diffuseTexture, fTexCoords).rgb;
        return; //If the light hit the surface from behind we wont calculate any further.
    }

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(normalEye - fPosEye.xyz);
    vec3 halfVector = normalize(lightDirN + viewDir);

    //compute diffuse light
    //diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    diffuse = max(dot_product, 0.0f) * lightColor;

    //compute specular light
    //vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 128);
    specular = specularStrength * specCoeff * lightColor;
}

void computePointLight() {
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = normalize(lightPosition - fPosEye.xyz);

    float distance = length(lightPosition - fPosEye.xyz);
    float attenuation = 1.0f / (constant + linear_ * distance + quadratic * (distance * distance));

    float dot_product = dot(normalEye, lightDirN);

    //compute ambient light
    ambient = ambientStrength * lightColor;
    ambient = ambient * attenuation;

    if (dot_product < 0) {
        color = ambient * texture(diffuseTexture, fTexCoords).rgb;
        return; //If the light hit the surface from behind we wont calculate any further.
    }

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(normalEye - fPosEye.xyz);
    vec3 halfVector = normalize(lightDirN + viewDir);

    //compute diffuse light
    //diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    diffuse = max(dot_product, 0.0f) * lightColor;
    diffuse = diffuse * attenuation;
    //compute specular light
    //vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 128);
    specular = specularStrength * specCoeff * lightColor;
    specular = specular * attenuation;
}

void main()
{
    computeDirLight();
    //computePointLight();

    //compute final vertex color
    color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

    fColor = vec4(color, 1.0f);
}
