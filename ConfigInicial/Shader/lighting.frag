// prueba
#version 330 core

#define NUMBER_OF_POINT_LIGHTS 4

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform Material material;
uniform int transparency;
uniform float brilloLampara;

// Agua
uniform int isWater;
uniform float tiempoAgua;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // =====================================================================
    // AGUA ANIMADA OPTIMIZADA (Menos carga para GPUs integradas)
    // =====================================================================
    if (isWater == 1 && transparency == 2)
    {
        vec2 p = FragPos.xz;
        float t = tiempoAgua;

        // Reducción drástica de 5 fases a solo 2 fases
        float fase1 = dot(p, vec2(0.075, 0.18)) + t * 0.48;
        float fase2 = dot(p, vec2(-0.17, 0.065)) - t * 0.36;

        float ola1 = sin(fase1);
        float ola2 = sin(fase2);

        float patron = ola1 * 0.6 + ola2 * 0.4;

        // Pendiente simplificada
        float pendienteX = cos(fase1) * 0.075 * 0.6 + cos(fase2) * -0.17 * 0.4;
        float pendienteZ = cos(fase1) * 0.18 * 0.6  + cos(fase2) * 0.065 * 0.4;

        vec3 normalAgua = normalize(vec3(-pendienteX * 2.0, 1.0, -pendienteZ * 2.0));
        float superficie = smoothstep(0.55, 0.95, norm.y);

        // Colores simplificados
        vec3 azulProfundo = vec3(0.008, 0.028, 0.043);
        vec3 turquesa     = vec3(0.032, 0.142, 0.158);

        float mezclaOlas = smoothstep(-0.5, 0.5, patron);
        vec3 colorAgua = mix(azulProfundo, turquesa, mezclaOlas);

        // =====================================================================
        // LUNA
        // =====================================================================
        vec3 direccionLuna = normalize(-dirLight.direction);
        float luzLuna = max(dot(normalAgua, direccionLuna), 0.0);
        colorAgua *= 0.82 + luzLuna * 0.34;

        vec3 reflejoLuna = reflect(-direccionLuna, normalAgua);
        float brilloLuna = pow(max(dot(viewDir, reflejoLuna), 0.0), 85.0);
        colorAgua += dirLight.specular * brilloLuna * 0.55;

        // Reflejo tenue en angulos rasantes
        float fresnel = pow(1.0 - max(dot(normalAgua, viewDir), 0.0), 3.0);
        colorAgua += vec3(0.010, 0.035, 0.045) * fresnel;

        vec3 laterales = vec3(0.004, 0.014, 0.022);
        colorAgua = mix(laterales, colorAgua, superficie);

        colorAgua = clamp(colorAgua, vec3(0.0), vec3(1.0));
        color = vec4(colorAgua, 1.0);
        return;
    }

    // =====================================================================
    // RESTO DE MODELOS
    // =====================================================================
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++)
    {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    vec4 texColor = texture(material.diffuse, TexCoords);

    color = vec4(
        result + vec3(1.0, 0.57, 0.22) * brilloLampara,
        texColor.a
    );

    if (color.a < 0.1 && transparency == 1)
        discard;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 /
        (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}