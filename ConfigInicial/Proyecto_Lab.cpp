// Proyecto 
// Ortiz Valles Joaquin Rafael y Juárez Osorio Josue Alexis
// Fecha de entrega: 11/11/2026
// Código unificado: Animación de recorrido + Skybox

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

// Variables globales para el recorrido de las mujeres
float mujPos_Z = 0.0f;
const float VELOCIDAD_MUJ = 1.5f;
const float DISTANCIA_MUJ = 40.0f;
const float DURACION_GIRO_MUJ = 0.8f;
bool mujRegresando = false;
bool mujGirando = false;
float tiempoGiroMuj = 0.0f;
float anguloMuj = 0.0f;
float anguloInicialMuj = 0.0f;
float anguloFinalMuj = 0.0f;

// Recorrido independiente de la mujer que esta sola
float mujSoloPos_Z = 0.0f;
const float DISTANCIA_MUJ_SOLO = 68.0f;
int estadoMujSolo = 0; // 0: camina, 1: gira al agua, 2: señala, 3: gira, 4: regresa, 5: gira al inicio
float tiempoGiroMujSolo = 0.0f;
float tiempoDetenidaMujSolo = 0.0f;
float anguloMujSolo = 0.0f;
float anguloInicialMujSolo = 0.0f;
float anguloFinalMujSolo = 90.0f;
float gestoMujSolo = 0.0f;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(-15.0f, 2.0f, -6.5f),
    glm::vec3(11.0f, 2.0f, -28.0f),
    glm::vec3(-15.0f, 2.0f, -31.0f),
    glm::vec3(11.0f, 2.0f, -6.0f),
    glm::vec3(0.0f, 4.0f, 0.0f)
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
    // Positions          
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

glm::vec3 Light1 = glm::vec3(0);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame


// Gesto de la mujer que observa el agua
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
				glm::mat4 matrizHueso = matrices[it->second] *
					glm::inverse(MatrizAssimp(hueso->mOffsetMatrix));
				posicion = glm::vec3(matrizHueso[3]);
				return true;
			}
		}
	}
	return false;
}

static glm::mat4 OrientarSegmento(const glm::vec3& pivote,
	const glm::vec3& extremo, const glm::vec3& direccion, float intensidad)
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

static void AplicarGestoBrazo(Model& personaje,
	std::vector<glm::mat4>& matrices, float intensidad)
{
	if (intensidad <= 0.0f)
		return;

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
		if (indice < 0 || indice >= (int)matrices.size())
			continue;

		if (nombre == "mixamorig:RightArm")
			matrices[indice] = brazo * matrices[indice];
		else if (nombre == "mixamorig:RightForeArm" ||
			nombre.find("mixamorig:RightHand") == 0)
			matrices[indice] = antebrazo * brazo * matrices[indice];
	}
}

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    //Model Luna((char*)"Models/moon.obj");
    Model Ciu((char*)"Models/Residential Buildings 001.obj");
    //Model Road((char*)"Models/road.obj");
    Model Ciu2((char*)"Models/Residential Buildings 002.obj");
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

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        lightingShader.Use();

        glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// --- MATEMÁTICA Y POSICIÓN DE LA LUNA ---
		float radioLuna = 40.0f;
		float lunaX = radioLuna * cos(movelightPos);
		float lunaZ = radioLuna * sin(movelightPos);
		float lunaY = 26.0f; // Altura en el cielo nocturno
		glm::vec3 posicionLuna(lunaX, lunaY, lunaZ);

		// --- LUZ DIRECCIONAL (LUZ DE LUNA) ---
		glm::vec3 dirLuna = glm::normalize(-posicionLuna);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), dirLuna.x, dirLuna.y, dirLuna.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.05f, 0.08f, 0.15f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.15f, 0.2f, 0.35f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.2f, 0.2f, 0.4f);

        // Point light 0
        glm::vec3 lightColor;
        lightColor.x = abs(sin(glfwGetTime() * Light1.x));
        lightColor.y = abs(sin(glfwGetTime() * Light1.y));
        lightColor.z = sin(glfwGetTime() * Light1.z);

        //Luna
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.05f + (lightColor.x * 0.1f), 0.08f + (lightColor.y * 0.1f), 0.15f + (lightColor.z * 0.1f));
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.15f + lightColor.x, 0.2f + lightColor.y, 0.35f + lightColor.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.2f, 0.2f, 0.4f);

        // Point light 0 (Ciudad 1)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.0f, 0.05f, 0.05f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.0f, 0.8f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.0075f);

        // Point light 1
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), 0.05f, 0.02f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), 1.0f, 0.6f, 0.1f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), 1.0f, 0.8f, 0.2f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.0075f);

        // Point light 2
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"), 0.02f, 0.02f, 0.02f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), 0.8f, 0.9f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.0075f);

        // Point light 3
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient"), 0.05f, 0.0f, 0.05f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"), 0.8f, 0.2f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"), 1.0f, 0.5f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.0075f);

        // Point light 4
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].position"), pointLightPositions[4].x, pointLightPositions[4].y, pointLightPositions[4].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].ambient"), 0.1f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].diffuse"), 0.9f, 0.9f, 0.8f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].quadratic"), 0.032f);

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

        // Set material properties
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

        glm::mat4 view;
        view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        view = camera.GetViewMatrix();

        glm::mat4 model = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        glBindVertexArray(0);

        //// 2. DIBUJAR LUNA
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, posicionLuna);
        //model = glm::scale(model, glm::vec3(0.1f));
        //glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //Luna.Draw(lightingShader);

        // 3. DIBUJAR CIUDAD
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-79.5f, 5.0f, -0.8f));
        model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Ciu.Draw(lightingShader);

        //// 3B. DIBUJAR SEGUNDA CIUDAD
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(11.0f, -0.7f, -28.0f));
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        //model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //Ciu.Draw(lightingShader);

        //// 4. DIBUJAR CARRETERA
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(-1.8f, -0.7f, -22.0f));
        //model = glm::scale(model, glm::vec3(4.0f, 1.0f, 6.0f));
        //glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //Road.Draw(lightingShader);

        // 5. DIBUJAR TERCERA CIUDAD
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-79.5f, 5.0f, -38.0f));
        model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Ciu2.Draw(lightingShader);

        //// 5B. DIBUJAR CUARTA CIUDAD
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(11.0f, -0.7f, -6.0f));
        //model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        //glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //Ciu2.Draw(lightingShader);

        // 7. Arbol Izquierdo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-45.0f, -3.5f, -60.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        arb.Draw(lightingShader);

		// 7.1 Arbol Derecho
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(45.0f, -3.5f, -60.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		arb.Draw(lightingShader);


		// 8. Muelle Izquierdo 1
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-40.0f, -10.0f, -22.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Mue.Draw(lightingShader);

		// 8.1 Muelle Izquierdo 2
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-40.0f, -10.0f, -97.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Mue1.Draw(lightingShader);

		// 9. Muelle Derecho 1
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(40.0f, -10.0f, -22.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Mue1.Draw(lightingShader);

		// 9.1 Muelle Derecho 2
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(40.0f, -10.0f, -97.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Mue.Draw(lightingShader);

		// 10. Torii
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -10.0f, -90.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Tor.Draw(lightingShader);

		// 11. Shorobuni
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-14.0f, -10.0f, -30.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Sho.Draw(lightingShader);

		// 12. Puente
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -10.0f, -60.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(31.0f, 11.0f, 21.0f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Pue.Draw(lightingShader);

        // 12. Agua
        model = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        model = glm::translate(model, glm::vec3(0.0f, -12.5f, -60.0f));
        model = glm::scale(model, glm::vec3(32.0f, 10.0f, 80.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
        Agu.Draw(lightingShader);
        glDisable(GL_BLEND);

		// 12. Gorin
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-40.0f, -10.0f, -30.0f)); // sobre el suelo, a un lado del perro
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0); // ver nota
		Gor.Draw(lightingShader);

        // Recorrido de ida y vuelta de las tres mujeres
        if (mujGirando)
        {
            tiempoGiroMuj += deltaTime;

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

		// Recorrido de la mujer que esta sola: avanza, señala y regresa al otro lado
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

		// La animación de caminar avanza en ambos sentidos y se congela durante los gestos y giros
		if (estadoMujSolo == 0 || estadoMujSolo == 4)
			Muj.UpdateAnimation(deltaTime, Muj.GetScenePointer());
		else
			Muj.UpdateAnimation(0.0f, Muj.GetScenePointer());

		std::vector<glm::mat4> transforms = Muj.GetFinalBoneMatrices();
		AplicarGestoBrazo(Muj, transforms, gestoMujSolo);
		for (GLuint i = 0; i < transforms.size(); i++)
		{
			std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
			glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
		}
		glm::mat4 modelAnim = glm::mat4(1.0f);
		modelAnim = glm::translate(modelAnim, glm::vec3(-30.0f, -7.8f, -100.0f + mujSoloPos_Z));
		modelAnim = glm::rotate(modelAnim, glm::radians(anguloMujSolo), glm::vec3(0.0f, 1.0f, 0.0f));
		modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
		Muj.Draw(lightingShader);



		// 14. Caminar Mujer1
		Muj1.UpdateAnimation(mujGirando ? 0.0f : deltaTime, Muj1.GetScenePointer());
		transforms = Muj1.GetFinalBoneMatrices();
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

		// 15. Caminar Mujer2
		Muj2.UpdateAnimation(mujGirando ? 0.0f : deltaTime, Muj2.GetScenePointer());
		transforms = Muj2.GetFinalBoneMatrices();
		for (GLuint i = 0; i < transforms.size(); i++)
		{
			std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
			glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));
		}
		modelAnim = glm::mat4(1.0f);
		modelAnim = glm::translate(modelAnim, glm::vec3(33.0f, -7.8f, -100.0f + mujPos_Z));
		modelAnim = glm::rotate(modelAnim, glm::radians(anguloMuj), glm::vec3(0.0f, 1.0f, 0.0f));
		modelAnim = glm::scale(modelAnim, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAnim));
		Muj2.Draw(lightingShader);


		glBindVertexArray(0);

        lampShader.Use();

        modelLoc = glGetUniformLocation(lampShader.Program, "model");
        viewLoc = glGetUniformLocation(lampShader.Program, "view");
        projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// Draw the light object (using light's vertex attributes)
		for (GLuint i = 0; i < 5; i++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);



		// Swap the screen buffers
		glfwSwapBuffers(window);
	}


	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
    // Camera controls
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    //Luces
    if (keys[GLFW_KEY_T])
    {
        pointLightPositions[0].x += 0.01f;
    }

    if (keys[GLFW_KEY_G])
    {
        pointLightPositions[0].x -= 0.01f;
    }

    if (keys[GLFW_KEY_Y])
    {
        pointLightPositions[0].y += 0.01f;
    }

    if (keys[GLFW_KEY_H])
    {
        pointLightPositions[0].y -= 0.01f;
    }

    if (keys[GLFW_KEY_U])
    {
        pointLightPositions[0].z -= 0.1f;
    }

    if (keys[GLFW_KEY_J])
    {
        pointLightPositions[0].z += 0.01f;
    }

    //Luna
    if (keys[GLFW_KEY_O])
    {
        movelightPos += 0.009f;
    }

    if (keys[GLFW_KEY_L])
    {
        movelightPos -= 0.009f;
    }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
        }
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

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
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}
