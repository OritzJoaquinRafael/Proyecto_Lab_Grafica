// Proyecto
// Ortiz Valles Joaquin Rafael y Juárez Osorio Josue Alexis
// Fecha de entrega: 11/11/2026
// Código unificado: Animación de recorrido + Skybox + Gestos (LUZ LUNA DINÁMICA) + Optimizaciones (4 Luces) + Control por Teclas

#include <iostream>
#include <cmath>
#include <vector>
#include <string>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Load Models
#include "SOIL2/SOIL2.h"

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Activación individual de animaciones (Compañero)
bool animarMujer1 = false;
bool animarMujer2 = false;
bool animarMujerSola = false;
bool animarHombreDerecho = false;
bool animarHombreSaludo = false;
bool animarMujerSaludo = false;
bool animarBotes = false;
float tiempoAnimacionBotes = 0.0f;

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// --- CONFIGURACIÓN DE LA LUNA ---
float radioLuna = 350.0f;
float alturaLuna = 100.0f;
float movelightPos = 61.0f;
bool animarLuna = false;

// --- SKYBOX TINT ---
const glm::vec3 SKY_TINT(1.0f, 1.0f, 1.0f);

// Recorridos individuales de las dos mujeres (Compañero)
const float VELOCIDAD_MUJ = 1.5f;
const float DISTANCIA_MUJ = 40.0f;
const float DURACION_GIRO_MUJ = 0.8f;

struct RecorridoMujer
{
    float posicionZ = 0.0f;
    bool regresando = false;
    bool girando = false;
    float tiempoGiro = 0.0f;
    float angulo = 0.0f;
    float anguloInicial = 0.0f;
    float anguloFinal = 0.0f;
};

RecorridoMujer recorridoMujer1;
RecorridoMujer recorridoMujer2;

static void ActualizarRecorridoMujer(RecorridoMujer& mujer, float delta)
{
    if (!mujer.girando)
    {
        if (!mujer.regresando)
        {
            mujer.posicionZ += VELOCIDAD_MUJ * delta;
            if (mujer.posicionZ >= DISTANCIA_MUJ)
            {
                mujer.posicionZ = DISTANCIA_MUJ;
                mujer.girando = true;
                mujer.tiempoGiro = 0.0f;
                mujer.anguloInicial = mujer.angulo;
                mujer.anguloFinal = 180.0f;
            }
        }
        else
        {
            mujer.posicionZ -= VELOCIDAD_MUJ * delta;
            if (mujer.posicionZ <= 0.0f)
            {
                mujer.posicionZ = 0.0f;
                mujer.girando = true;
                mujer.tiempoGiro = 0.0f;
                mujer.anguloInicial = mujer.angulo;
                mujer.anguloFinal = 0.0f;
            }
        }
    }
    if (mujer.girando)
    {
        mujer.tiempoGiro += delta;
        float progreso = glm::clamp(mujer.tiempoGiro / DURACION_GIRO_MUJ, 0.0f, 1.0f);
        mujer.angulo = glm::mix(mujer.anguloInicial, mujer.anguloFinal, progreso);
        if (progreso >= 1.0f)
        {
            mujer.angulo = mujer.anguloFinal;
            mujer.girando = false;
            mujer.regresando = !mujer.regresando;
        }
    }
}

// Recorrido independiente de la MUJER QUE ESTÁ SOLA (Muj)
float mujSoloPos_Z = 0.0f;
const float DISTANCIA_MUJ_SOLO = 68.0f;
int estadoMujSolo = 0; // 0: camina, 1: gira al agua, 2: señala, 3: gira, 4: regresa, 5: gira al inicio
float tiempoGiroMujSolo = 0.0f;
float tiempoDetenidaMujSolo = 0.0f;
float anguloMujSolo = 0.0f;
float anguloInicialMujSolo = 0.0f;
float anguloFinalMujSolo = 90.0f;
float gestoMujSolo = 0.0f;

// Recorrido independiente del hombre del lado derecho
float homDerechoPosZ = 0.0f;
const float POSICION_INICIAL_HOM_DERECHO_Z = -26.0f;
const float DISTANCIA_HOM_DERECHO = 22.0f;
const float VELOCIDAD_HOM_DERECHO = 1.3f;
const float DURACION_GIRO_HOM_DERECHO = 0.8f;
int estadoHomDerecho = 0;
float tiempoGiroHomDerecho = 0.0f;
float anguloHomDerecho = 180.0f;
float anguloInicialHomDerecho = 180.0f;
float anguloFinalHomDerecho = 360.0f;

// Recorrido del hombre que camina por el lado izquierdo del puente
float homPuentePosX = -24.0f;
const float HOM_PUENTE_INICIO_X = -24.0f;
const float HOM_PUENTE_FINAL_X = -8.0f;
const float VELOCIDAD_HOM_PUENTE = 1.0f;
int estadoHomPuente = 0;
float tiempoGiroHomPuente = 0.0f;
float anguloHomPuente = 90.0f;
float anguloInicialHomPuente = 90.0f;
float anguloFinalHomPuente = 270.0f;
const float DURACION_GIRO_HOM_PUENTE = 0.8f;

// OPTIMIZACIÓN: Solo 4 luces estáticas en los edificios (Tuyo)
glm::vec3 pointLightPositions[] = {
    glm::vec3(-80.0f, 0.95f, 3.0f),    // Luz 0: Edificio Izquierdo 1
    glm::vec3(-84.0f, 0.95f, -110.0f), // Luz 1: Edificio Izquierdo 4
    glm::vec3(77.0f, 0.95f, 2.2f),     // Luz 2: Edificio Derecho 1
    glm::vec3(90.0f, 0.95f, -121.0f)   // Luz 3: Edificio Derecho 3
};

float vertices[] = {
     -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
      0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

     -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
      0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
      0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
      0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
      0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

GLfloat skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

GLuint loadCubemap(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int width, height, channels;
    for (GLuint i = 0; i < faces.size(); i++)
    {
        unsigned char* image = SOIL_load_image(faces[i].c_str(), &width, &height, &channels, SOIL_LOAD_RGB);
        if (image)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            SOIL_free_image_data(image);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

// =======================================================================
// FUNCIONES DE CINEMÁTICA Y GESTOS
// =======================================================================
static glm::mat4 MatrizAssimp(const aiMatrix4x4& matriz)
{
    glm::mat4 resultado(1.0f);
    for (int fila = 0; fila < 4; fila++)
        for (int columna = 0; columna < 4; columna++)
            resultado[fila][columna] = matriz[columna][fila];
    return resultado;
}

static bool PosicionHueso(Model& personaje, const std::vector<glm::mat4>& matrices,
    const std::string& nombre, glm::vec3& posicion)
{
    auto& mapa = personaje.GetBoneInfoMap();
    auto it = mapa.find(nombre);
    if (it == mapa.end() || it->second < 0 || it->second >= (int)matrices.size())
        return false;

    const aiScene* escena = personaje.GetScenePointer();
    if (!escena)
        return false;

    for (unsigned int i = 0; i < escena->mNumMeshes; i++)
    {
        const aiMesh* malla = escena->mMeshes[i];
        for (unsigned int j = 0; j < malla->mNumBones; j++)
        {
            const aiBone* hueso = malla->mBones[j];
            if (hueso->mName.C_Str() == nombre)
            {
                glm::mat4 matrizHueso = matrices[it->second] * glm::inverse(MatrizAssimp(hueso->mOffsetMatrix));
                posicion = glm::vec3(matrizHueso[3]);
                return true;
            }
        }
    }
    return false;
}
static glm::mat4 OrientarSegmento(const glm::vec3& pivote, const glm::vec3& extremo, const glm::vec3& direccion, float intensidad)
{
    glm::vec3 original = extremo - pivote;
    if (glm::length(original) < 0.0001f)
        return glm::mat4(1.0f);
    original = glm::normalize(original);
    glm::vec3 destino = glm::normalize(direccion);
    float producto = glm::clamp(glm::dot(original, destino), -1.0f, 1.0f);
    glm::vec3 eje = glm::cross(original, destino);
    if (glm::length(eje) < 0.0001f)
    {
        eje = glm::cross(original, glm::vec3(0.0f, 1.0f, 0.0f));
        if (glm::length(eje) < 0.0001f)
            eje = glm::cross(original, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    eje = glm::normalize(eje);
    glm::mat4 matriz(1.0f);
    matriz = glm::translate(matriz, pivote);
    matriz = glm::rotate(matriz, std::acos(producto) * intensidad, eje);
    matriz = glm::translate(matriz, -pivote);
    return matriz;
}
static void AplicarGestoBrazo(Model& personaje, std::vector<glm::mat4>& matrices, float intensidad)
{
    if (intensidad <= 0.0f) return;
    glm::vec3 hombro, codo, mano;
    if (!PosicionHueso(personaje, matrices, "mixamorig:RightArm", hombro) ||
        !PosicionHueso(personaje, matrices, "mixamorig:RightForeArm", codo) ||
        !PosicionHueso(personaje, matrices, "mixamorig:RightHand", mano))
        return;
    glm::vec3 direccion(0.0f, -0.05f, 1.0f);
    glm::mat4 brazo = OrientarSegmento(hombro, codo, direccion, intensidad);
    glm::vec3 codoGirado = glm::vec3(brazo * glm::vec4(codo, 1.0f));
    glm::vec3 manoGirada = glm::vec3(brazo * glm::vec4(mano, 1.0f));
    glm::mat4 antebrazo = OrientarSegmento(codoGirado, manoGirada, direccion, intensidad);
    for (const auto& hueso : personaje.GetBoneInfoMap())
    {
        const std::string& nombre = hueso.first;
        int indice = hueso.second;
        if (indice < 0 || indice >= (int)matrices.size()) continue;
        if (nombre == "mixamorig:RightArm")
            matrices[indice] = brazo * matrices[indice];
        else if (nombre == "mixamorig:RightForeArm" || nombre.find("mixamorig:RightHand") == 0)
            matrices[indice] = antebrazo * brazo * matrices[indice];
    }
}
// =======================================================================
// LÓGICA DE BOTES
// =======================================================================
const float PI_BOTES = 3.14159265359f;
const float RADIO_RUTA_BOTES = 3.0f;
const float Z_SUPERIOR_RUTA_BOTES = 8.0f;
const float Z_INFERIOR_RUTA_BOTES = -128.0f;
const float LONGITUD_TRAMO_BOTES = Z_SUPERIOR_RUTA_BOTES - Z_INFERIOR_RUTA_BOTES;
const float LONGITUD_CURVA_BOTES = PI_BOTES * RADIO_RUTA_BOTES;
const float LONGITUD_RUTA_BOTES = 2.0f * LONGITUD_TRAMO_BOTES + 2.0f * LONGITUD_CURVA_BOTES;
enum EstadoBote
{
    BOTE_CORRIENTE_IZQUIERDA,
    BOTE_CURVA_SUPERIOR,
    BOTE_CORRIENTE_DERECHA,
    BOTE_CURVA_INFERIOR
};
struct BoteAnimado
{
    glm::vec3 posicion;
    float progreso;
    float velocidad;
    float anguloY;
    float faseBalanceo;
    float faseCorriente;
    EstadoBote estado;
};
BoteAnimado boteIzquierdo =
{
    glm::vec3(-RADIO_RUTA_BOTES, -10.0f, -30.0f),
    -30.0f - Z_INFERIOR_RUTA_BOTES,
    1.8f, 0.0f, 0.0f, 0.0f,
    BOTE_CORRIENTE_IZQUIERDA
};
BoteAnimado boteDerecho =
{
    glm::vec3(RADIO_RUTA_BOTES, -10.0f, -90.0f),
    LONGITUD_TRAMO_BOTES + LONGITUD_CURVA_BOTES + Z_SUPERIOR_RUTA_BOTES + 90.0f,
    1.8f, 180.0f, 2.4f, 0.0f,
    BOTE_CORRIENTE_DERECHA
};
static void ActualizarBote(BoteAnimado& bote, float delta, float tiempo)
{
    delta = glm::clamp(delta, 0.0f, 0.05f);
    float variacionCorriente =
        1.0f + 0.045f * std::sin(tiempo * 0.22f + bote.faseCorriente) +
        0.018f * std::sin(tiempo * 0.51f + bote.faseCorriente * 1.7f);
    bote.progreso += bote.velocidad * variacionCorriente * delta;
    if (bote.progreso >= LONGITUD_RUTA_BOTES)
        bote.progreso -= LONGITUD_RUTA_BOTES;
    glm::vec3 tangente(0.0f, 0.0f, 1.0f);
    if (bote.progreso < LONGITUD_TRAMO_BOTES)
    {
        bote.estado = BOTE_CORRIENTE_IZQUIERDA;
        bote.posicion.x = -RADIO_RUTA_BOTES;
        bote.posicion.z = Z_INFERIOR_RUTA_BOTES + bote.progreso;
        tangente = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    else if (bote.progreso < LONGITUD_TRAMO_BOTES + LONGITUD_CURVA_BOTES)
    {
        bote.estado = BOTE_CURVA_SUPERIOR;
        float distanciaCurva = bote.progreso - LONGITUD_TRAMO_BOTES;
        float anguloCurva = PI_BOTES - distanciaCurva / RADIO_RUTA_BOTES;
        bote.posicion.x = RADIO_RUTA_BOTES * std::cos(anguloCurva);
        bote.posicion.z = Z_SUPERIOR_RUTA_BOTES + RADIO_RUTA_BOTES * std::sin(anguloCurva);
        tangente = glm::vec3(std::sin(anguloCurva), 0.0f, -std::cos(anguloCurva));
    }
    else if (bote.progreso < 2.0f * LONGITUD_TRAMO_BOTES + LONGITUD_CURVA_BOTES)
    {
        bote.estado = BOTE_CORRIENTE_DERECHA;
        float distanciaTramo = bote.progreso - LONGITUD_TRAMO_BOTES - LONGITUD_CURVA_BOTES;
        bote.posicion.x = RADIO_RUTA_BOTES;
        bote.posicion.z = Z_SUPERIOR_RUTA_BOTES - distanciaTramo;
        tangente = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    else
    {
        bote.estado = BOTE_CURVA_INFERIOR;
        float distanciaCurva = bote.progreso - 2.0f * LONGITUD_TRAMO_BOTES - LONGITUD_CURVA_BOTES;
        float anguloCurva = -distanciaCurva / RADIO_RUTA_BOTES;
        bote.posicion.x = RADIO_RUTA_BOTES * std::cos(anguloCurva);
        bote.posicion.z = Z_INFERIOR_RUTA_BOTES + RADIO_RUTA_BOTES * std::sin(anguloCurva);
        tangente = glm::vec3(std::sin(anguloCurva), 0.0f, -std::cos(anguloCurva));
    }
    glm::vec3 normal(tangente.z, 0.0f, -tangente.x);
    float derivaLateral =
        0.16f * std::sin(tiempo * 0.19f + bote.faseCorriente) +
        0.06f * std::sin(tiempo * 0.43f + bote.faseCorriente * 1.4f);
    bote.posicion += normal * derivaLateral;
    float orientacion = std::atan2(tangente.x, tangente.z);
    if (orientacion < 0.0f)
        orientacion += 2.0f * PI_BOTES;
    bote.anguloY = glm::degrees(orientacion) +
        1.0f * std::sin(tiempo * 0.31f + bote.faseCorriente);
}
static glm::mat4 CrearTransformacionFlotante(
    const glm::vec3& posicion, const glm::vec3& escala, float anguloY,
    float tiempo, float fase, const glm::vec3& pivoteLocal,
    float amplitudVertical, float amplitudBalanceo)
{
    float oleajeLento = tiempo * 0.62f + fase;
    float oleajeSecundario = tiempo * 0.93f + fase * 0.73f;
    float altura =
        (0.70f * std::sin(oleajeLento) +
            0.30f * std::sin(oleajeSecundario)) * amplitudVertical;
    float balanceoLateral =
        std::sin(oleajeLento * 0.85f + fase * 0.2f) *
        glm::radians(amplitudBalanceo) +
        0.30f * std::sin(oleajeSecundario * 0.61f + fase * 1.1f) *
        glm::radians(amplitudBalanceo * 0.45f);
    float cabeceo = std::sin(oleajeLento * 0.72f + fase * 0.35f) *
        glm::radians(amplitudBalanceo * 0.42f);
    float giroY = std::sin(oleajeSecundario * 0.38f + fase) *
        glm::radians(amplitudBalanceo * 0.12f);
    glm::vec3 pivote = pivoteLocal * escala;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, posicion + glm::vec3(0.0f, altura, 0.0f));
    model = glm::translate(model, pivote);
    model = glm::rotate(model, glm::radians(anguloY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, giroY, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, cabeceo, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, balanceoLateral, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, -pivote);
    model = glm::scale(model, escala);
    return model;
}
glm::vec3 Light1 = glm::vec3(0);
// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto", nullptr, nullptr);
    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync activado
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Shaders
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader skyboxShader("Shader/SkyBox.vs", "Shader/SkyBox.frag");
    // Load Models
    Model arb((char*)"Models/arbol.obj");
    Model Mue((char*)"Models/muelle.obj");
    Model Mue1((char*)"Models/muelle1.obj");
    Model Tor((char*)"Models/torii.obj");
    Model Sho((char*)"Models/shorobuni.obj");
    Model Pue((char*)"Models/puente.obj");
    Model Muj((char*)"Models/mujer.fbx", "mujer.png");
    Model Muj1((char*)"Models/mujer1.fbx", "mujer1.png");
    Model Muj2((char*)"Models/mujer2.fbx", "mujer2.png");
    Model Agu((char*)"Models/agua.obj");
    Model Gor((char*)"Models/gorin.obj");
    Model HomS((char*)"Models/homsal.fbx", "hombre.png");
    Model HomB((char*)"Models/hombor.fbx", "hombre1.png");
    Model MujS((char*)"Models/mujsal.fbx", "mujer1.png");
    Model Hom2((char*)"Models/hombre2.fbx", "hombre2.png");
    Model Edi((char*)"Models/edificio.obj");
    Model Edi1((char*)"Models/edificio1.obj");
    Model Edi2((char*)"Models/edificio2.obj");
    Model Edi3((char*)"Models/edificio3.obj");
    Model Luna((char*)"Models/moon.obj");
    // Model VAO/VBO
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    // Skybox VAO/VBO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
    std::vector<std::string> faces;
    faces.push_back("Skybox/right.jpg");
    faces.push_back("Skybox/left.jpg");
    faces.push_back("Skybox/top.jpg");
    faces.push_back("Skybox/bottom.jpg");
    faces.push_back("Skybox/back.jpg");
    faces.push_back("Skybox/front.jpg");
    GLuint cubemapTexture = loadCubemap(faces);
    // =======================================================================
    // OPTIMIZACIÓN: SOLO 4 LUCES DE EDIFICIOS Y 1 DIRECCIONAL (TUYO)
    // =======================================================================
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
    GLint locModel = glGetUniformLocation(lightingShader.Program, "model");
    GLint locView = glGetUniformLocation(lightingShader.Program, "view");
    GLint locProj = glGetUniformLocation(lightingShader.Program, "projection");
    GLint locViewPos = glGetUniformLocation(lightingShader.Program, "viewPos");
    GLint locTransparency = glGetUniformLocation(lightingShader.Program, "transparency");
    GLint locBrilloLampara = glGetUniformLocation(lightingShader.Program, "brilloLampara");
    GLint locBones = glGetUniformLocation(lightingShader.Program, "finalBoneMatrices[0]");
    GLint locIsWater = glGetUniformLocation(lightingShader.Program, "isWater");
    GLint locTiempoAgua = glGetUniformLocation(lightingShader.Program, "tiempoAgua");
    GLint locDirLightDir = glGetUniformLocation(lightingShader.Program, "dirLight.direction");
    GLint locDirLightAmb = glGetUniformLocation(lightingShader.Program, "dirLight.ambient");
    GLint locDirLightDiff = glGetUniformLocation(lightingShader.Program, "dirLight.diffuse");
    GLint locDirLightSpec = glGetUniformLocation(lightingShader.Program, "dirLight.specular");
    struct PointLightLocs {
        GLint pos, amb, diff, spec, constant, linear, quad;
    } locPointLights[4]; // Reducido estrictamente a 4 luces
    for (int i = 0; i < 4; i++) {
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        locPointLights[i].pos = glGetUniformLocation(lightingShader.Program, (prefix + "position").c_str());
        locPointLights[i].amb = glGetUniformLocation(lightingShader.Program, (prefix + "ambient").c_str());
        locPointLights[i].diff = glGetUniformLocation(lightingShader.Program, (prefix + "diffuse").c_str());
        locPointLights[i].spec = glGetUniformLocation(lightingShader.Program, (prefix + "specular").c_str());
        locPointLights[i].constant = glGetUniformLocation(lightingShader.Program, (prefix + "constant").c_str());
        locPointLights[i].linear = glGetUniformLocation(lightingShader.Program, (prefix + "linear").c_str());
        locPointLights[i].quad = glGetUniformLocation(lightingShader.Program, (prefix + "quadratic").c_str());
    }
    skyboxShader.Use();
    GLint locSkyView = glGetUniformLocation(skyboxShader.Program, "view");
    GLint locSkyProj = glGetUniformLocation(skyboxShader.Program, "projection");
    GLint locSkyTint = glGetUniformLocation(skyboxShader.Program, "skyTint");
    GLint locSkybox = glGetUniformLocation(skyboxShader.Program, "skybox");
    lampShader.Use();
    GLint locLampModel = glGetUniformLocation(lampShader.Program, "model");
    GLint locLampView = glGetUniformLocation(lampShader.Program, "view");
    GLint locLampProj = glGetUniformLocation(lampShader.Program, "projection");
    GLint locLampColor = glGetUniformLocation(lampShader.Program, "lampColor");
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 1.0f, 1000.0f);
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        DoMovement();
        // Actualización de botes controlada por tecla 'B' (Compañero)
        if (animarBotes)
        {
            tiempoAnimacionBotes += glm::clamp(deltaTime, 0.0f, 0.05f);
            ActualizarBote(boteIzquierdo, deltaTime, tiempoAnimacionBotes);
            ActualizarBote(boteDerecho, deltaTime, tiempoAnimacionBotes);
        }
        glm::mat4 modeloBoteIzquierdo = CrearTransformacionFlotante(
            boteIzquierdo.posicion, glm::vec3(4.0f), boteIzquierdo.anguloY,
            tiempoAnimacionBotes, boteIzquierdo.faseBalanceo, glm::vec3(0.0f, 0.47f, 0.0f),
            0.12f, 10.0f
        );
        glm::mat4 modeloBoteDerecho = CrearTransformacionFlotante(
            boteDerecho.posicion, glm::vec3(4.0f), boteDerecho.anguloY,
            tiempoAnimacionBotes, boteDerecho.faseBalanceo, glm::vec3(0.0f, 0.47f, 0.0f),
            0.12f, 10.0f
        );
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // =======================================================================
        // 1. CONFIGURAR LUCES Y SHADER PRINCIPAL DE ILUMINACIÓN
        // =======================================================================
        glEnable(GL_DEPTH_TEST);
        lightingShader.Use();
        glUniform1f(locBrilloLampara, 0.0f);
        glUniform1i(locIsWater, 0);
        glUniform3f(locViewPos, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        // Posición de la Luna
        if (animarLuna) {
            movelightPos += 0.15f * deltaTime;
        }
        float lunaX = radioLuna * cos(movelightPos);
        float lunaZ = radioLuna * sin(movelightPos);
        glm::vec3 posicionLuna(lunaX, alturaLuna, lunaZ);
        // Luz Direccional de Luna
        glm::vec3 dirLuna = glm::normalize(-posicionLuna);
        glUniform3f(locDirLightDir, dirLuna.x, dirLuna.y, dirLuna.z);
        glUniform3f(locDirLightAmb, 0.12f, 0.15f, 0.22f);
        glUniform3f(locDirLightDiff, 0.35f, 0.42f, 0.55f);
        glUniform3f(locDirLightSpec, 0.45f, 0.50f, 0.65f);
        // 4 Luces Puntuales de Edificios (Tuyo)
        for (int i = 0; i < 4; i++) {
            glUniform3f(locPointLights[i].pos, pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
            glUniform3f(locPointLights[i].amb, 0.08f, 0.04f, 0.01f);
            glUniform3f(locPointLights[i].diff, 1.0f, 0.6f, 0.1f);
            glUniform3f(locPointLights[i].spec, 1.0f, 0.8f, 0.2f);
            glUniform1f(locPointLights[i].constant, 1.0f);
            glUniform1f(locPointLights[i].linear, 0.007f);
            glUniform1f(locPointLights[i].quad, 0.0002f);
        }
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));
        glm::mat4 model = glm::mat4(1.0f);
        // =======================================================================
        // 2. DIBUJO DE MODELOS ESTÁTICOS
        // =======================================================================
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-45.0f, -3.5f, -60.0f));
        model = glm::scale(model, glm::vec3(10.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(locTransparency, 0);
        arb.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(45.0f, -3.5f, -60.0f));
        model = glm::scale(model, glm::vec3(10.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        arb.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-40.0f, -10.0f, -22.0f));
        model = glm::scale(model, glm::vec3(40.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Mue.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-40.0f, -10.0f, -97.0f));
        model = glm::scale(model, glm::vec3(40.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Mue1.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, -10.0f, -22.0f));
        model = glm::scale(model, glm::vec3(40.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Mue1.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, -10.0f, -97.0f));
        model = glm::scale(model, glm::vec3(40.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Mue.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, -90.0f));
        model = glm::scale(model, glm::vec3(20.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Tor.Draw(lightingShader);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modeloBoteIzquierdo));
        Sho.Draw(lightingShader);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modeloBoteDerecho));
        Sho.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, -60.0f));
        model = glm::scale(model, glm::vec3(31.0f, 11.0f, 21.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Pue.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-45.0f, -3.6f, -32.0f));
        model = glm::scale(model, glm::vec3(3.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Gor.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(45.0f, -3.6f, -32.0f));
        model = glm::scale(model, glm::vec3(3.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Gor.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-45.0f, -3.6f, -86.4f));
        model = glm::scale(model, glm::vec3(3.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Gor.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(45.0f, -3.6f, -86.4f));
        model = glm::scale(model, glm::vec3(3.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Gor.Draw(lightingShader);
        // Edificios
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-80.0f, 0.95f, 3.0f));
        model = glm::scale(model, glm::vec3(40.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-86.0f, 0.95f, -44.0f));
        model = glm::scale(model, glm::vec3(30.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi1.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-78.2f, 0.95f, -71.4f));
        model = glm::scale(model, glm::vec3(30.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi2.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-84.0f, 0.95f, -110.0f));
        model = glm::scale(model, glm::vec3(26.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi3.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(77.0f, 0.95f, 2.2f));
        model = glm::scale(model, glm::vec3(26.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi2.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(80.0f, 0.95f, -46.2f));
        model = glm::scale(model, glm::vec3(40.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(90.0f, 0.95f, -121.0f));
        model = glm::scale(model, glm::vec3(38.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi1.Draw(lightingShader);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(85.0f, 0.95f, -84.0f));
        model = glm::scale(model, glm::vec3(26.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Edi3.Draw(lightingShader);
        // Luna
        model = glm::translate(glm::mat4(1.0f), posicionLuna);
        float anguloLunaY = atan2(posicionLuna.x, posicionLuna.z);
        model = glm::rotate(model, anguloLunaY, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        Luna.Draw(lightingShader);
        // ======================================================================================
        // LÓGICA DE RECORRIDOS INDIVIDUALES POR TECLA (Compañero)
        // ======================================================================================
        if (animarMujer1)
            ActualizarRecorridoMujer(recorridoMujer1, deltaTime);
        if (animarMujer2)
            ActualizarRecorridoMujer(recorridoMujer2, deltaTime);
        if (animarMujerSola)
        {
            if (estadoMujSolo == 0)
            {
                mujSoloPos_Z += VELOCIDAD_MUJ * deltaTime;
                if (mujSoloPos_Z >= DISTANCIA_MUJ_SOLO)
                {
                    mujSoloPos_Z = DISTANCIA_MUJ_SOLO;
                    estadoMujSolo = 1;
                    tiempoGiroMujSolo = 0.0f;
                    anguloInicialMujSolo = anguloMujSolo;
                    anguloFinalMujSolo = 90.0f;
                }
            }
            else if (estadoMujSolo == 1)
            {
                tiempoGiroMujSolo += deltaTime;
                float progresoGiroSolo = glm::clamp(tiempoGiroMujSolo / DURACION_GIRO_MUJ, 0.0f, 1.0f);
                anguloMujSolo = glm::mix(anguloInicialMujSolo, anguloFinalMujSolo, progresoGiroSolo);
                if (progresoGiroSolo >= 1.0f)
                {
                    anguloMujSolo = anguloFinalMujSolo;
                    estadoMujSolo = 2;
                    tiempoDetenidaMujSolo = 0.0f;
                }
            }
            else if (estadoMujSolo == 2)
            {
                if (gestoMujSolo < 1.0f)
                    gestoMujSolo = glm::min(gestoMujSolo + deltaTime / 0.8f, 1.0f);
                else
                    tiempoDetenidaMujSolo += deltaTime;
                if (tiempoDetenidaMujSolo >= 10.0f)
                {
                    estadoMujSolo = 3;
                    tiempoGiroMujSolo = 0.0f;
                    anguloInicialMujSolo = anguloMujSolo;
                    anguloFinalMujSolo = 180.0f;
                }
            }
            else if (estadoMujSolo == 3)
            {
                tiempoGiroMujSolo += deltaTime;
                gestoMujSolo = glm::max(gestoMujSolo - deltaTime / DURACION_GIRO_MUJ, 0.0f);
                float progresoGiroSolo = glm::clamp(tiempoGiroMujSolo / DURACION_GIRO_MUJ, 0.0f, 1.0f);
                anguloMujSolo = glm::mix(anguloInicialMujSolo, anguloFinalMujSolo, progresoGiroSolo);
                if (progresoGiroSolo >= 1.0f)
                {
                    anguloMujSolo = anguloFinalMujSolo;
                    estadoMujSolo = 4;
                }
            }
            else if (estadoMujSolo == 4)
            {
                mujSoloPos_Z -= VELOCIDAD_MUJ * deltaTime;
                if (mujSoloPos_Z <= 0.0f)
                {
                    mujSoloPos_Z = 0.0f;
                    estadoMujSolo = 5;
                    tiempoGiroMujSolo = 0.0f;
                    anguloInicialMujSolo = anguloMujSolo;
                    anguloFinalMujSolo = 0.0f;
                }
            }
            else if (estadoMujSolo == 5)
            {
                tiempoGiroMujSolo += deltaTime;
                float progresoGiroSolo = glm::clamp(tiempoGiroMujSolo / DURACION_GIRO_MUJ, 0.0f, 1.0f);
                anguloMujSolo = glm::mix(anguloInicialMujSolo, anguloFinalMujSolo, progresoGiroSolo);
                if (progresoGiroSolo >= 1.0f)
                {
                    anguloMujSolo = 0.0f;
                    mujSoloPos_Z = 0.0f;
                    gestoMujSolo = 0.0f;
                    tiempoDetenidaMujSolo = 0.0f;
                    estadoMujSolo = 0;
                }
            }
        }
        // Hombre Derecho
        if (animarHombreDerecho)
        {
            if (estadoHomDerecho == 0)
            {
                homDerechoPosZ += VELOCIDAD_HOM_DERECHO * deltaTime;
                if (homDerechoPosZ >= DISTANCIA_HOM_DERECHO)
                {
                    homDerechoPosZ = DISTANCIA_HOM_DERECHO;
                    estadoHomDerecho = 1;
                    tiempoGiroHomDerecho = 0.0f;
                    anguloInicialHomDerecho = anguloHomDerecho;
                    anguloFinalHomDerecho = 360.0f;
                }
            }
            else if (estadoHomDerecho == 1)
            {
                tiempoGiroHomDerecho += deltaTime;
                float progresoGiroHomDerecho = glm::clamp(tiempoGiroHomDerecho / DURACION_GIRO_HOM_DERECHO, 0.0f, 1.0f);
                anguloHomDerecho = glm::mix(anguloInicialHomDerecho, anguloFinalHomDerecho, progresoGiroHomDerecho);
                if (progresoGiroHomDerecho >= 1.0f)
                {
                    anguloHomDerecho = 360.0f;
                    estadoHomDerecho = 2;
                }
            }
            else if (estadoHomDerecho == 2)
            {
                homDerechoPosZ -= VELOCIDAD_HOM_DERECHO * deltaTime;
                if (homDerechoPosZ <= 0.0f)
                {
                    homDerechoPosZ = 0.0f;
                    estadoHomDerecho = 3;
                    tiempoGiroHomDerecho = 0.0f;
                    anguloInicialHomDerecho = anguloHomDerecho;
                    anguloFinalHomDerecho = 180.0f;
                }
            }
            else if (estadoHomDerecho == 3)
            {
                tiempoGiroHomDerecho += deltaTime;
                float progresoGiroHomDerecho = glm::clamp(tiempoGiroHomDerecho / DURACION_GIRO_HOM_DERECHO, 0.0f, 1.0f);
                anguloHomDerecho = glm::mix(anguloInicialHomDerecho, anguloFinalHomDerecho, progresoGiroHomDerecho);
                if (progresoGiroHomDerecho >= 1.0f)
                {
                    anguloHomDerecho = 180.0f;
                    estadoHomDerecho = 0;
                }
            }
        }
        // Hombre Puente (Automático)
        if (estadoHomPuente == 0)
        {
            homPuentePosX += VELOCIDAD_HOM_PUENTE * deltaTime;
            if (homPuentePosX >= HOM_PUENTE_FINAL_X)
            {
                homPuentePosX = HOM_PUENTE_FINAL_X;
                estadoHomPuente = 1;
                tiempoGiroHomPuente = 0.0f;
                anguloInicialHomPuente = anguloHomPuente;
                anguloFinalHomPuente = 270.0f;
            }
        }
        else if (estadoHomPuente == 1)
        {
            tiempoGiroHomPuente += deltaTime;
            float progresoGiroPuente = glm::clamp(tiempoGiroHomPuente / DURACION_GIRO_HOM_PUENTE, 0.0f, 1.0f);
            anguloHomPuente = glm::mix(anguloInicialHomPuente, anguloFinalHomPuente, progresoGiroPuente);
            if (progresoGiroPuente >= 1.0f)
            {
                anguloHomPuente = 270.0f;
                estadoHomPuente = 2;
            }
        }
        else if (estadoHomPuente == 2)
        {
            homPuentePosX -= VELOCIDAD_HOM_PUENTE * deltaTime;
            if (homPuentePosX <= HOM_PUENTE_INICIO_X)
            {
                homPuentePosX = HOM_PUENTE_INICIO_X;
                estadoHomPuente = 3;
                tiempoGiroHomPuente = 0.0f;
                anguloInicialHomPuente = anguloHomPuente;
                anguloFinalHomPuente = 90.0f;
            }
        }
        else if (estadoHomPuente == 3)
        {
            tiempoGiroHomPuente += deltaTime;
            float progresoGiroPuente = glm::clamp(tiempoGiroHomPuente / DURACION_GIRO_HOM_PUENTE, 0.0f, 1.0f);
            anguloHomPuente = glm::mix(anguloInicialHomPuente, anguloFinalHomPuente, progresoGiroPuente);
            if (progresoGiroPuente >= 1.0f)
            {
                anguloHomPuente = 90.0f;
                estadoHomPuente = 0;
            }
        }
        // ======================================================================================
        // 3. DIBUJO DE MODELOS ANIMADOS
        // ======================================================================================
        // Mujer Sola (con gesto cinemático)
        Muj.UpdateAnimation(
            (animarMujerSola && (estadoMujSolo == 0 || estadoMujSolo == 4)) ? deltaTime : 0.0f,
            Muj.GetScenePointer());
        std::vector<glm::mat4> transformsMujSolo = Muj.GetFinalBoneMatrices();
        AplicarGestoBrazo(Muj, transformsMujSolo, gestoMujSolo);
        if (!transformsMujSolo.empty()) {
            glUniformMatrix4fv(locBones, (GLsizei)transformsMujSolo.size(), GL_FALSE, glm::value_ptr(transformsMujSolo[0]));
        }
        glm::mat4 modelAnim = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, -7.8f, -100.0f + mujSoloPos_Z));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloMujSolo), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelAnim));
        Muj.Draw(lightingShader);
        // Mujer1
        Muj1.UpdateAnimation((animarMujer1 && !recorridoMujer1.girando) ? deltaTime : 0.0f, Muj1.GetScenePointer());
        std::vector<glm::mat4> transforms = Muj1.GetFinalBoneMatrices();
        if (!transforms.empty()) {
            glUniformMatrix4fv(locBones, (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
        }
        modelAnim = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, -7.8f, -100.0f + recorridoMujer1.posicionZ));
        modelAnim = glm::rotate(modelAnim, glm::radians(recorridoMujer1.angulo), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelAnim));
        Muj1.Draw(lightingShader);
        // Mujer2
        Muj2.UpdateAnimation((animarMujer2 && !recorridoMujer2.girando) ? deltaTime : 0.0f, Muj2.GetScenePointer());
        transforms = Muj2.GetFinalBoneMatrices();
        if (!transforms.empty()) {
            glUniformMatrix4fv(locBones, (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
        }
        modelAnim = glm::translate(glm::mat4(1.0f), glm::vec3(34.0f, -7.8f, -100.0f + recorridoMujer2.posicionZ));
        modelAnim = glm::rotate(modelAnim, glm::radians(recorridoMujer2.angulo), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelAnim));
        Muj2.Draw(lightingShader);
        // Hombre2 (Derecho)
        Hom2.UpdateAnimation((animarHombreDerecho && (estadoHomDerecho == 0 || estadoHomDerecho == 2)) ? deltaTime : 0.0f, Hom2.GetScenePointer());
        transforms = Hom2.GetFinalBoneMatrices();
        if (!transforms.empty()) {
            glUniformMatrix4fv(locBones, (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
        }
        modelAnim = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, -7.8f, POSICION_INICIAL_HOM_DERECHO_Z - homDerechoPosZ));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloHomDerecho), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelAnim));
        Hom2.Draw(lightingShader);
        // Hombre Saludando
        HomS.UpdateAnimation(animarHombreSaludo ? deltaTime : 0.0f, HomS.GetScenePointer());
        transforms = HomS.GetFinalBoneMatrices();
        if (!transforms.empty()) {
            glUniformMatrix4fv(locBones, (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
        }
        modelAnim = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -5.0f, -62.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelAnim));
        HomS.Draw(lightingShader);
        // Hombre Borracho (Puente)
        HomB.UpdateAnimation(deltaTime, HomB.GetScenePointer());
        transforms = HomB.GetFinalBoneMatrices();
        if (!transforms.empty()) {
            glUniformMatrix4fv(locBones, (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
        }
        modelAnim = glm::translate(glm::mat4(1.0f), glm::vec3(homPuentePosX, -5.0f, -62.0f));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloHomPuente), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelAnim));
        HomB.Draw(lightingShader);
        // Mujer Saludando
        MujS.UpdateAnimation(animarMujerSaludo ? deltaTime : 0.0f, MujS.GetScenePointer());
        transforms = MujS.GetFinalBoneMatrices();
        if (!transforms.empty()) {
            glUniformMatrix4fv(locBones, (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
        }
        modelAnim = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, -5.0f, -62.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelAnim));
        MujS.Draw(lightingShader);
        glBindVertexArray(0);
        // =======================================================================
        // 4. DIBUJAR SOLO LAS 4 LÁMPARAS DE LOS EDIFICIOS (Tuyo)
        // =======================================================================
        lampShader.Use();
        glUniformMatrix4fv(locLampView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locLampProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(locLampColor, 1.0f, 1.0f, 1.0f);
        for (GLuint i = 0; i < 4; i++)
        {
            model = glm::translate(glm::mat4(1.0f), pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.3f));
            glUniformMatrix4fv(locLampModel, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
        // =======================================================================
        // 5. SKYBOX DIBUJADO AL FINAL (Optimización Overdraw - Tuyo)
        // =======================================================================
        glDepthFunc(GL_LEQUAL);
        skyboxShader.Use();
        glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glUniformMatrix4fv(locSkyView, 1, GL_FALSE, glm::value_ptr(skyView));
        glUniformMatrix4fv(locSkyProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(locSkyTint, SKY_TINT.x, SKY_TINT.y, SKY_TINT.z);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(locSkybox, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        // =======================================================================
        // 6. AGUA ANIMADA
        // =======================================================================
        lightingShader.Use();
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -9.748f, -60.0f));
        model = glm::scale(model, glm::vec3(61.0f, 0.04f, 154.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(locTiempoAgua, currentFrame);
        glUniform1i(locTransparency, 2);
        glUniform1i(locIsWater, 1);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 30, 6);
        glBindVertexArray(0);
        glUniform1i(locIsWater, 0);
        glUniform1i(locTransparency, 0);
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}
void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    // Ajuste en tiempo real de Luz 0
    if (keys[GLFW_KEY_T]) pointLightPositions[0].x += 0.01f;
    if (keys[GLFW_KEY_G]) pointLightPositions[0].x -= 0.01f;
    if (keys[GLFW_KEY_Y]) pointLightPositions[0].y += 0.01f;
    if (keys[GLFW_KEY_H]) pointLightPositions[0].y -= 0.01f;
    if (keys[GLFW_KEY_U]) pointLightPositions[0].z -= 0.1f;
    if (keys[GLFW_KEY_J]) pointLightPositions[0].z += 0.01f;
}
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        active = !active;
        Light1 = active ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(0);
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        animarLuna = !animarLuna;
    }
    // Controles de animaciones individuales (Compañero)
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_Q: animarMujer1 = !animarMujer1; break;
        case GLFW_KEY_E: animarMujer2 = !animarMujer2; break;
        case GLFW_KEY_R: animarMujerSola = !animarMujerSola; break;
        case GLFW_KEY_F: animarHombreDerecho = !animarHombreDerecho; break;
        case GLFW_KEY_Z: animarHombreSaludo = !animarHombreSaludo; break;
        case GLFW_KEY_X: animarMujerSaludo = !animarMujerSaludo; break;
        case GLFW_KEY_B: animarBotes = !animarBotes; break;
        }
    }
}
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}