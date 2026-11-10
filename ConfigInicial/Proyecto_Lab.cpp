// Proyecto 
// Ortiz Valles Joaquin Rafael y Ju?rez Osorio Josue Alexis
// Fecha de entrega: 11/11/2026
// C?digo unificado: Animaci?n de recorrido + Skybox + Gestos

#include <iostream>
#include <cmath>
#include <vector>   // --- SKYBOX ---
#include <string>   // --- SKYBOX ---

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

//Load Models
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
const GLuint WIDTH = 1600, HEIGHT = 900;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;
float movelightPos = 0.0f;

// --- SKYBOX TINT ---
const glm::vec3 SKY_TINT(1.0f, 1.0f, 1.0f);

// Variables globales para el recorrido GRUPAL (Muj1, Muj2, Hom2)
float mujPos_Z = 0.0f;
const float VELOCIDAD_MUJ = 1.5f;
const float DISTANCIA_MUJ = 40.0f; // Tomado del c?digo de tu compa?ero
const float DURACION_GIRO_MUJ = 0.8f;
bool mujRegresando = false;
bool mujGirando = false;
float tiempoGiroMuj = 0.0f;
float anguloMuj = 0.0f;
float anguloInicialMuj = 0.0f;
float anguloFinalMuj = 0.0f;

// Recorrido independiente de la MUJER QUE EST? SOLA (Muj)
float mujSoloPos_Z = 0.0f;
const float DISTANCIA_MUJ_SOLO = 68.0f;
int estadoMujSolo = 0; // 0: camina, 1: gira al agua, 2: se?ala, 3: gira, 4: regresa, 5: gira al inicio
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
int estadoHomDerecho = 0; // 0: avanza, 1: gira, 2: regresa, 3: gira al inicio
float tiempoGiroHomDerecho = 0.0f;
float anguloHomDerecho = 180.0f;
float anguloInicialHomDerecho = 180.0f;
float anguloFinalHomDerecho = 360.0f;

// Recorrido del hombre que camina por el lado izquierdo del puente
float homPuentePosX = -24.0f;
const float HOM_PUENTE_INICIO_X = -24.0f;
const float HOM_PUENTE_FINAL_X = -8.0f;
const float VELOCIDAD_HOM_PUENTE = 1.0f;
int estadoHomPuente = 0; // 0: avanza, 1: gira, 2: regresa, 3: gira al inicio
float tiempoGiroHomPuente = 0.0f;
float anguloHomPuente = 90.0f;
float anguloInicialHomPuente = 90.0f;
float anguloFinalHomPuente = 270.0f;
const float DURACION_GIRO_HOM_PUENTE = 0.8f;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(-15.0f, 2.0f, -6.5f),  // Luz 0: Ciudad 1
    glm::vec3(11.0f, 2.0f, -28.0f),  // Luz 1: Ciudad 2
    glm::vec3(-15.0f, 2.0f, -31.0f), // Luz 2: Ciudad 3
    glm::vec3(11.0f, 2.0f, -6.0f),   // Luz 3: Ciudad 4
    glm::vec3(0.0f, 4.0f, 0.0f)      // Luz 4: Farol blanco
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

// --- SKYBOX VERTICES ---
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

// --- SKYBOX FUNCTION ---
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
// FUNCIONES DE CINEM?TICA Y GESTOS
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
            if (nombre == hueso->mName.C_Str())
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
    2.8f, 0.0f, 0.0f, 0.0f,
    BOTE_CORRIENTE_IZQUIERDA
};

BoteAnimado boteDerecho =
{
    glm::vec3(RADIO_RUTA_BOTES, -10.0f, -90.0f),
    LONGITUD_TRAMO_BOTES + LONGITUD_CURVA_BOTES + Z_SUPERIOR_RUTA_BOTES + 90.0f,
    2.8f, 180.0f, 2.4f, 0.0f,
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
    float oleajeLento = tiempo * 0.92f + fase;
    float oleajeSecundario = tiempo * 1.31f + fase * 0.73f;

    float altura =
        (0.70f * std::sin(oleajeLento) +
            0.30f * std::sin(oleajeSecundario)) * amplitudVertical;

    float giroX = std::sin(oleajeLento * 0.76f + fase * 0.2f) *
        glm::radians(amplitudBalanceo);
    float giroZ = std::cos(oleajeLento * 0.67f + fase) *
        glm::radians(amplitudBalanceo * 0.7f);
    float giroY = std::sin(oleajeSecundario * 0.35f + fase) *
        glm::radians(amplitudBalanceo * 0.16f);

    glm::vec3 pivote = pivoteLocal * escala;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, posicion + glm::vec3(0.0f, altura, 0.0f));
    model = glm::translate(model, pivote);
    model = glm::rotate(model, glm::radians(anguloY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, giroY, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, giroX, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, giroZ, glm::vec3(0.0f, 0.0f, 1.0f));
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
    // Init GLFW
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
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

    // --- SKYBOX VAO/VBO ---
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    // --- CARGA DE TEXTURAS DEL SKYBOX ---
    std::vector<std::string> faces;
    faces.push_back("Skybox/right.jpg");
    faces.push_back("Skybox/left.jpg");
    faces.push_back("Skybox/top.jpg");
    faces.push_back("Skybox/bottom.jpg");
    faces.push_back("Skybox/back.jpg");
    faces.push_back("Skybox/front.jpg");
    GLuint cubemapTexture = loadCubemap(faces);

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

    glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        ActualizarBote(boteIzquierdo, deltaTime, currentFrame);
        ActualizarBote(boteDerecho, deltaTime, currentFrame);

        glm::mat4 modeloBoteIzquierdo = CrearTransformacionFlotante(
            boteIzquierdo.posicion, glm::vec3(4.0f), boteIzquierdo.anguloY,
            currentFrame, boteIzquierdo.faseBalanceo, glm::vec3(0.0f, 0.47f, 0.0f),
            0.10f, 2.5f
        );
        glm::mat4 modeloBoteDerecho = CrearTransformacionFlotante(
            boteDerecho.posicion, glm::vec3(4.0f), boteDerecho.anguloY,
            currentFrame, boteDerecho.faseBalanceo, glm::vec3(0.0f, 0.47f, 0.0f),
            0.10f, 2.5f
        );

        glm::vec3 posicionLuzBoteIzquierdo = glm::vec3(
            modeloBoteIzquierdo * glm::vec4(0.0f, 0.78f, 0.0f, 1.0f)
        );
        glm::vec3 posicionLuzBoteDerecho = glm::vec3(
            modeloBoteDerecho * glm::vec4(0.0f, 0.78f, 0.0f, 1.0f)
        );

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- SKYBOX (Se dibuja al fondo, preservando tu orden original para la transparencia del agua) ---
        glDisable(GL_DEPTH_TEST);
        skyboxShader.Use();
        glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(skyboxShader.Program, "skyTint"), SKY_TINT.x, SKY_TINT.y, SKY_TINT.z);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(skyboxShader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);

        lightingShader.Use();

        // Titileo sincronizado de las luces de los botes de almas
        float intensidadLlama = 0.92f
            + 0.06f * sin(currentFrame * 7.0f)
            + 0.02f * sin(currentFrame * 13.0f);

        glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "brilloLampara"), 0.0f);

        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // --- MATEMATICA Y POSICION DE LA LUNA ---
        float radioLuna = 40.0f;
        float lunaX = radioLuna * cos(movelightPos);
        float lunaZ = radioLuna * sin(movelightPos);
        float lunaY = 26.0f;
        glm::vec3 posicionLuna(lunaX, lunaY, lunaZ);

        // --- LUZ DIRECCIONAL (LUZ DE LUNA) ---
        glm::vec3 dirLuna = glm::normalize(-posicionLuna);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), dirLuna.x, dirLuna.y, dirLuna.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.05f, 0.08f, 0.15f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.15f, 0.2f, 0.35f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.2f, 0.2f, 0.4f);

        glm::vec3 lightColor;
        lightColor.x = abs(sin(glfwGetTime() * Light1.x));
        lightColor.y = abs(sin(glfwGetTime() * Light1.y));
        lightColor.z = sin(glfwGetTime() * Light1.z);

        // Luna Update
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.05f + (lightColor.x * 0.1f), 0.08f + (lightColor.y * 0.1f), 0.15f + (lightColor.z * 0.1f));
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.15f + lightColor.x, 0.2f + lightColor.y, 0.35f + lightColor.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.2f, 0.2f, 0.4f);

        // Luces Point
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.0f, 0.05f, 0.05f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.0f, 0.8f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.0075f);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), 0.05f, 0.02f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), 1.0f, 0.6f, 0.1f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), 1.0f, 0.8f, 0.2f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.0075f);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"), 0.02f, 0.02f, 0.02f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), 0.8f, 0.9f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.0075f);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient"), 0.05f, 0.0f, 0.05f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"), 0.8f, 0.2f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"), 1.0f, 0.5f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.0075f);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].position"), pointLightPositions[4].x, pointLightPositions[4].y, pointLightPositions[4].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].ambient"), 0.1f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].diffuse"), 0.9f, 0.9f, 0.8f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].quadratic"), 0.032f);

        // Luz calida del bote de almas izquierdo
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].position"), posicionLuzBoteIzquierdo.x, posicionLuzBoteIzquierdo.y, posicionLuzBoteIzquierdo.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].ambient"), 0.045f * intensidadLlama, 0.022f * intensidadLlama, 0.008f * intensidadLlama);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].diffuse"), 1.0f * intensidadLlama, 0.58f * intensidadLlama, 0.24f * intensidadLlama);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].specular"), 1.0f * intensidadLlama, 0.72f * intensidadLlama, 0.38f * intensidadLlama);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].linear"), 0.08f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].quadratic"), 0.02f);

        // Luz calida del bote de almas derecho
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[6].position"), posicionLuzBoteDerecho.x, posicionLuzBoteDerecho.y, posicionLuzBoteDerecho.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[6].ambient"), 0.045f * intensidadLlama, 0.022f * intensidadLlama, 0.008f * intensidadLlama);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[6].diffuse"), 1.0f * intensidadLlama, 0.58f * intensidadLlama, 0.24f * intensidadLlama);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[6].specular"), 1.0f * intensidadLlama, 0.72f * intensidadLlama, 0.38f * intensidadLlama);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[6].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[6].linear"), 0.08f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[6].quadratic"), 0.02f);

        // SpotLight
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(0.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(0.0f)));

        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

        glm::mat4 view = camera.GetViewMatrix();

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model = glm::mat4(1.0f);

        // 1. Arbol Izquierdo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-45.0f, -3.5f, -60.0f));
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        arb.Draw(lightingShader);

        // 1.1 Arbol Derecho
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(45.0f, -3.5f, -60.0f));
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        arb.Draw(lightingShader);

        // 2. Muelle Izquierdo 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-40.0f, -10.0f, -22.0f));
        model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Mue.Draw(lightingShader);

        // 2.1 Muelle Izquierdo 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-40.0f, -10.0f, -97.0f));
        model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Mue1.Draw(lightingShader);

        // 3. Muelle Derecho 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(40.0f, -10.0f, -22.0f));
        model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Mue1.Draw(lightingShader);

        // 3.1 Muelle Derecho 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(40.0f, -10.0f, -97.0f));
        model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Mue.Draw(lightingShader);

        // 4. Torii
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -10.0f, -90.0f));
        model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Tor.Draw(lightingShader);

        // 5. Bote izquierdo
        model = modeloBoteIzquierdo;
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Sho.Draw(lightingShader);

        // 5.1 Bote derecho
        model = modeloBoteDerecho;
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Sho.Draw(lightingShader);

        // 6. Puente
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -10.0f, -60.0f));
        model = glm::scale(model, glm::vec3(31.0f, 11.0f, 21.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Pue.Draw(lightingShader);

        // 7. Agua (Transparencia)
        model = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        model = glm::translate(model, glm::vec3(0.0f, -12.5f, -60.0f));
        model = glm::scale(model, glm::vec3(32.0f, 10.0f, 80.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
        Agu.Draw(lightingShader);
        glDisable(GL_BLEND);

        // 8.0 Gorin
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-45.0f, -3.6f, -32.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Gor.Draw(lightingShader);

        // 8.1 Gorin
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(45.0f, -3.6f, -32.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Gor.Draw(lightingShader);

        // 8.2 Gorin
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-45.0f, -3.6f, -86.4f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Gor.Draw(lightingShader);

        // 8.3 Gorin
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(45.0f, -3.6f, -86.4f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Gor.Draw(lightingShader);

        // 9.0 Edificio Izquierdo 1

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-80.0f, 0.95f, 3.0f));
        model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi.Draw(lightingShader);

        // 9.1 Edificio Izquierdo 2

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-86.0f, 0.95f, -44.0f));
        model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi1.Draw(lightingShader);

        // 9.2 Edificio Izquierdo 3

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-78.2f, 0.95f, -71.4f));
        model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi2.Draw(lightingShader);

        // 9.3 Edificio Izquierdo 4

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-84.0f, 0.95f, -110.0f));
        model = glm::scale(model, glm::vec3(26.0f, 26.0f, 26.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi3.Draw(lightingShader);

        // 10.0 Edificio Derecho 1.

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(77.0f, 0.95f, 2.2f));
        model = glm::scale(model, glm::vec3(26.0f, 26.0f, 26.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi2.Draw(lightingShader);

        // 10.1 Edificio Derecho 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(80.0f, 0.95f, -46.2f));
        model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi.Draw(lightingShader);

        // 10.2 Edificio Derecho 3

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(90.0f, 0.95f, -121.0f));
        model = glm::scale(model, glm::vec3(38.0f, 38.0f, 38.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi1.Draw(lightingShader);

        // 10.3 Edificio Derecho 4

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(85.0f, 0.95f, -84.0f));
        model = glm::scale(model, glm::vec3(26.0f, 26.0f, 26.0f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Edi3.Draw(lightingShader);

        // ======================================================================================
        // RECORRIDO DEL GRUPO (Muj1, Muj2 y Hom2)
        // ======================================================================================
        if (!mujGirando)
        {
            if (!mujRegresando)
            {
                mujPos_Z += VELOCIDAD_MUJ * deltaTime;

                if (mujPos_Z >= DISTANCIA_MUJ)
                {
                    mujPos_Z = DISTANCIA_MUJ;
                    mujGirando = true;
                    tiempoGiroMuj = 0.0f;
                    anguloInicialMuj = anguloMuj;
                    anguloFinalMuj = 180.0f;
                }
            }
            else
            {
                mujPos_Z -= VELOCIDAD_MUJ * deltaTime;

                if (mujPos_Z <= 0.0f)
                {
                    mujPos_Z = 0.0f;
                    mujGirando = true;
                    tiempoGiroMuj = 0.0f;
                    anguloInicialMuj = anguloMuj;
                    anguloFinalMuj = 0.0f;
                }
            }
        }
        if (mujGirando)
        {
            tiempoGiroMuj += deltaTime;
            float progresoGiro = glm::clamp(tiempoGiroMuj / DURACION_GIRO_MUJ, 0.0f, 1.0f);
            anguloMuj = glm::mix(anguloInicialMuj, anguloFinalMuj, progresoGiro);

            if (progresoGiro >= 1.0f)
            {
                anguloMuj = anguloFinalMuj;
                mujGirando = false;
                mujRegresando = !mujRegresando;
            }
        }

        // ======================================================================================
        // RECORRIDO INDEPENDIENTE Y GESTOS DE LA MUJER QUE VA SOLA (Muj)
        // ======================================================================================
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

        // ======================================================================================
        // RECORRIDO INDEPENDIENTE DEL HOMBRE DEL LADO DERECHO
        // ======================================================================================
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

        // ======================================================================================
        // RECORRIDO DEL HOMBRE POR EL LADO IZQUIERDO DEL PUENTE
        // ======================================================================================
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
        // DIBUJO DE MODELOS ANIMADOS
        // ======================================================================================

        // 9. Mujer Sola (con gesto cinem?tico)
        if (estadoMujSolo == 0 || estadoMujSolo == 4)
            Muj.UpdateAnimation(deltaTime, Muj.GetScenePointer());
        else
            Muj.UpdateAnimation(0.0f, Muj.GetScenePointer()); // Congelarla

        std::vector<glm::mat4> transformsMujSolo = Muj.GetFinalBoneMatrices();
        AplicarGestoBrazo(Muj, transformsMujSolo, gestoMujSolo); // El c?digo de tu compa?ero
        for (GLuint i = 0; i < transformsMujSolo.size(); i++)
        {
            std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transformsMujSolo[i]));
        }
        glm::mat4 modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, glm::vec3(-30.0f, -7.8f, -100.0f + mujSoloPos_Z));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloMujSolo), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        Muj.Draw(lightingShader);


        // 10. Mujer1
        Muj1.UpdateAnimation(mujGirando ? 0.0f : deltaTime, Muj1.GetScenePointer());
        std::vector<glm::mat4> transforms = Muj1.GetFinalBoneMatrices();
        for (GLuint i = 0; i < transforms.size(); i++)
        {
            std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, glm::vec3(30.0f, -7.8f, -100.0f + mujPos_Z));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloMuj), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        Muj1.Draw(lightingShader);

        // 11. Mujer2
        Muj2.UpdateAnimation(mujGirando ? 0.0f : deltaTime, Muj2.GetScenePointer());
        transforms = Muj2.GetFinalBoneMatrices();
        for (GLuint i = 0; i < transforms.size(); i++)
        {
            std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, glm::vec3(34.0f, -7.8f, -100.0f + mujPos_Z));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloMuj), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        Muj2.Draw(lightingShader);

        //// 12. Hombre
        //Hom2.UpdateAnimation(mujGirando ? 0.0f : deltaTime, Hom2.GetScenePointer());
        //transforms = Hom2.GetFinalBoneMatrices();
        //for (GLuint i = 0; i < transforms.size(); i++) {
        //    std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
        //    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        //}
        //modelAnim = glm::mat4(1.0f);
        //modelAnim = glm::translate(modelAnim, glm::vec3(30.0f, -7.8f, -20.0f + mujPos_Z));
        //modelAnim = glm::rotate(modelAnim, glm::radians(anguloMuj), glm::vec3(0.0f, 1.0f, 0.0f)); // Agregada rotacion para que coincida con las mujeres
        //modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        //glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        //Hom2.Draw(lightingShader);

        //// 13. Hombre1
        //Hom2.UpdateAnimation(mujGirando ? 0.0f : deltaTime, Hom2.GetScenePointer());
        //transforms = Hom2.GetFinalBoneMatrices();
        //for (GLuint i = 0; i < transforms.size(); i++) {
        //    std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
        //    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        //}
        //modelAnim = glm::mat4(1.0f);
        //modelAnim = glm::translate(modelAnim, glm::vec3(30.0f, -7.8f, -20.0f + mujPos_Z));
        //modelAnim = glm::rotate(modelAnim, glm::radians(anguloMuj), glm::vec3(0.0f, 1.0f, 0.0f)); // Agregada rotacion para que coincida con las mujeres
        //modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        //glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        //Hom2.Draw(lightingShader);

        // 14. Hombre2 (recorrido independiente dentro del muelle derecho)
        Hom2.UpdateAnimation((estadoHomDerecho == 0 || estadoHomDerecho == 2) ? deltaTime : 0.0f, Hom2.GetScenePointer());
        transforms = Hom2.GetFinalBoneMatrices();
        for (GLuint i = 0; i < transforms.size(); i++) {
            std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, glm::vec3(30.0f, -7.8f, POSICION_INICIAL_HOM_DERECHO_Z - homDerechoPosZ));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloHomDerecho), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        Hom2.Draw(lightingShader);

        // 15. Hombre Saludando
        HomS.UpdateAnimation(deltaTime, HomS.GetScenePointer());
        transforms = HomS.GetFinalBoneMatrices();
        for (GLuint i = 0; i < transforms.size(); i++)
        {
            std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, glm::vec3(5.0f, -5.0f, -62.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        HomS.Draw(lightingShader);

        // 16. Hombre Borracho
        HomB.UpdateAnimation(deltaTime, HomB.GetScenePointer());
        transforms = HomB.GetFinalBoneMatrices();
        for (GLuint i = 0; i < transforms.size(); i++)
        {
            std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, glm::vec3(homPuentePosX, -5.0f, -62.0f));
        modelAnim = glm::rotate(modelAnim, glm::radians(anguloHomPuente), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        HomB.Draw(lightingShader);

        // 17. Mujer Saludando
        MujS.UpdateAnimation(deltaTime, MujS.GetScenePointer());
        transforms = MujS.GetFinalBoneMatrices();
        for (GLuint i = 0; i < transforms.size(); i++)
        {
            std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, glm::vec3(10.0f, -5.0f, -62.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
        MujS.Draw(lightingShader);

        glBindVertexArray(0);

        // DIBUJAR LAMPARAS
        lampShader.Use();
        modelLoc = glGetUniformLocation(lampShader.Program, "model");
        viewLoc = glGetUniformLocation(lampShader.Program, "view");
        projLoc = glGetUniformLocation(lampShader.Program, "projection");
        GLint lampColorLoc = glGetUniformLocation(lampShader.Program, "lampColor");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3f(lampColorLoc, 1.0f, 1.0f, 1.0f);

        for (GLuint i = 0; i < 5; i++)
        {
            model = glm::mat4(1);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Fuentes visibles de luz calida sobre los dos botes
        glUniform3f(lampColorLoc, intensidadLlama, 0.84f * intensidadLlama, 0.22f * intensidadLlama);

        model = glm::mat4(1.0f);
        model = glm::translate(model, posicionLuzBoteIzquierdo);
        model = glm::scale(model, glm::vec3(0.14f, 0.17f, 0.14f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, posicionLuzBoteDerecho);
        model = glm::scale(model, glm::vec3(0.14f, 0.17f, 0.14f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// Moves/alters the camera positions based on user input
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

    // Luces
    if (keys[GLFW_KEY_T]) pointLightPositions[0].x += 0.01f;
    if (keys[GLFW_KEY_G]) pointLightPositions[0].x -= 0.01f;
    if (keys[GLFW_KEY_Y]) pointLightPositions[0].y += 0.01f;
    if (keys[GLFW_KEY_H]) pointLightPositions[0].y -= 0.01f;
    if (keys[GLFW_KEY_U]) pointLightPositions[0].z -= 0.1f;
    if (keys[GLFW_KEY_J]) pointLightPositions[0].z += 0.01f;

    // Luna
    if (keys[GLFW_KEY_O]) movelightPos += 0.009f;
    if (keys[GLFW_KEY_L]) movelightPos -= 0.009f;
}

// Is called whenever a key is pressed/released via GLFW
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