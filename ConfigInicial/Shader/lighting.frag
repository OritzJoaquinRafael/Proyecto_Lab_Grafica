#version 330 core

#define NUMBER_OF_POINT_LIGHTS 7

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

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
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
uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;
uniform float brilloLampara;

// Agua
uniform int isWater;
uniform float tiempoAgua;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // =====================================================================
    // AGUA ANIMADA (compañero). Sale antes de calcular las luces normales.
    // =====================================================================
    if (isWater == 1 && transparency == 2)
    {
        vec2 p = FragPos.xz;
        float t = tiempoAgua;

        // Ondas amplias que avanzan en distintas direcciones
        float fase1 = dot(p, vec2(0.075, 0.18)) + t * 0.48;
        float fase2 = dot(p, vec2(-0.17, 0.065)) - t * 0.36;
        float fase3 = dot(p, vec2(0.24, -0.14)) + t * 0.62;

        // Ondulaciones pequenas para romper los reflejos
        float fase4 = dot(p, vec2(0.72, 0.38)) + t * 1.25;
        float fase5 = dot(p, vec2(-0.58, 0.83)) - t * 0.98;

        float ola1 = sin(fase1);
        float ola2 = sin(fase2);
        float ola3 = sin(fase3);
        float ola4 = sin(fase4);
        float ola5 = sin(fase5);

        float patron = ola1 * 0.42 + ola2 * 0.31 + ola3 * 0.19
                     + ola4 * 0.045 + ola5 * 0.035;

        // Pendiente aproximada de las ondas para normales cambiantes
        float pendienteX =
            cos(fase1) * 0.075 * 0.42 +
            cos(fase2) * -0.17 * 0.31 +
            cos(fase3) * 0.24 * 0.19 +
            cos(fase4) * 0.72 * 0.045 +
            cos(fase5) * -0.58 * 0.035;

        float pendienteZ =
            cos(fase1) * 0.18 * 0.42 +
            cos(fase2) * 0.065 * 0.31 +
            cos(fase3) * -0.14 * 0.19 +
            cos(fase4) * 0.38 * 0.045 +
            cos(fase5) * 0.83 * 0.035;

        vec3 normalAgua = normalize(vec3(
            -pendienteX * 2.0,
            1.0,
            -pendienteZ * 2.0
        ));

        float superficie = smoothstep(0.55, 0.95, norm.y);

        // Azul profundo con variaciones turquesa
        vec3 azulProfundo = vec3(0.008, 0.028, 0.043);
        vec3 azulMedio    = vec3(0.016, 0.073, 0.092);
        vec3 turquesa     = vec3(0.032, 0.142, 0.158);

        float mezclaOlas = smoothstep(-0.78, 0.68, patron);
        vec3 colorAgua = mix(azulProfundo, azulMedio, mezclaOlas);

        float zonasTurquesa = smoothstep(0.12, 0.78, patron + ola4 * ola5 * 0.18);
        colorAgua = mix(colorAgua, turquesa, zonasTurquesa * 0.42);

        float crestasFinas = smoothstep(0.48, 0.92, ola4 * ola5);
        colorAgua += vec3(0.006, 0.018, 0.021) * crestasFinas;

        // Luna
        vec3 direccionLuna = normalize(-dirLight.direction);
        float luzLuna = max(dot(normalAgua, direccionLuna), 0.0);
        colorAgua *= 0.82 + luzLuna * 0.34;

        vec3 reflejoLuna = reflect(-direccionLuna, normalAgua);
        float brilloLuna = pow(max(dot(viewDir, reflejoLuna), 0.0), 85.0);
        colorAgua += dirLight.specular * brilloLuna * 0.55;

        // Reflejos dorados de las luces de los dos botes (luces 5 y 6)
        for (int i = 5; i < 7; i++)
        {
            vec2 diferencia = FragPos.xz - pointLights[i].position.xz;
            float lateral = diferencia.x;
            float longitudinal = diferencia.y;
            float distanciaAgua = length(diferencia);

            float distancia = length(pointLights[i].position - FragPos);
            float atenuacion = 1.0 /
                (pointLights[i].constant +
                 pointLights[i].linear * distancia +
                 pointLights[i].quadratic * distancia * distancia);

            float franjaCentral =
                exp(-(lateral * lateral) * 0.42) *
                exp(-(longitudinal * longitudinal) * 0.020);

            float franjaSuave =
                exp(-(lateral * lateral) * 0.13) *
                exp(-(longitudinal * longitudinal) * 0.008);

            float fragmentos =
                sin(longitudinal * 1.05 + t * 1.9 + lateral * 1.4) * 0.65 +
                sin(longitudinal * 2.10 - t * 1.3) * 0.35;

            float mascaraReflejo =
                0.32 + 0.68 * smoothstep(-0.25, 0.62, fragmentos);

            float movimientoReflejo =
                0.78 + 0.22 * abs(sin(fase4 + lateral * 1.4));

            float caidaDistancia = exp(-distanciaAgua * 0.025);

            float reflejo =
                (franjaCentral * 0.72 + franjaSuave * 0.28) *
                mascaraReflejo * movimientoReflejo * caidaDistancia;

            colorAgua += pointLights[i].diffuse * reflejo * 0.62;

            vec3 direccionLuz = normalize(pointLights[i].position - FragPos);
            vec3 mitad = normalize(direccionLuz + viewDir);
            float brilloPuntual = pow(max(dot(normalAgua, mitad), 0.0), 55.0);

            colorAgua += pointLights[i].specular *
                         brilloPuntual * atenuacion * franjaSuave * 0.85;
        }

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
    // RESTO DE MODELOS (tu código, sin cambios)
    // =====================================================================
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++)
    {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

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

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 /
        (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;

    if (abs(epsilon) < 0.0001)
        return vec3(0.0);

    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}