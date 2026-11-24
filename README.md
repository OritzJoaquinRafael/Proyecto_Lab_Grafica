# Proyecto Final — Computación Gráfica e Interacción Humano-Computadora

**Grupo:** 06  
**Integrantes:**
* 319071616
* 320083125

Escena en 3D desarrollada en C++ con OpenGL (GLEW, GLFW, GLM, SOIL2 y Assimp) inspirada en la festividad japonesa del **Obon** y la ceremonia **Tōrō Nagashi** (linternas flotantes sobre el río).

---

## Ejecución rápida (sin instalar nada)

El proyecto incluye un ejecutable independiente compilado en modo **Release (x86)** para Windows con todas sus librerías y recursos listos para correr:

1. Descarga el repositorio completo (**Code → Download ZIP**) y descomprímelo.
2. Abre la carpeta **`Ejecutable/`**.
3. Haz doble clic en **`ProyectoFinal.exe`**.

> **Nota:** Al iniciar la escena, las animaciones comienzan pausadas para permitir explorar el entorno libremente. Usa las teclas numéricas y de función descritas abajo para activarlas.

---

## Controles e interacciones

### Cámara y ventana
|             Tecla            |                        Acción                         |
| :---                         | :---                                                  |
| `W`, `A`, `S`, `D` o Flechas | Mover la cámara (adelante, izquierda, atrás, derecha) |
| Mouse                        | Rotar la vista                                        |
| `N`                          | Capturar el cursor del mouse dentro de la ventana     |
| `M`                          | Liberar el cursor del mouse                           |
| `ESC`                        | Cerrar la aplicación                                  |

### Animaciones de la escena (activar / pausar)
| Tecla |                       Animación                       |
| :---  | :---                                                  |
| `0`   | Órbita de la Luna e iluminación direccional dinámica  |
| `1`   | Recorrido de Mujer 1                                  |
| `2`   | Recorrido de Mujer 2                                  |
| `3`   | Recorrido de Mujer Sola y gesto de brazo hacia el río |
| `4`   | Recorrido de Hombre (lado derecho)                    |
| `5`   | Animación de Hombre saludando                         |
| `6`   | Animación de Mujer saludando                          |
| `7`   | Botes de almas (desplazamiento y flotación en el río) |
| `8`   | Avance de linternas flotantes (Tōrō Nagashi)          |
| `9`   | Balanceo y oleaje de linternas flotantes              |
| `? o '`   | Recorrido de personaje adicional                      |

### Animación jerárquica por Keyframes (Personaje y Linterna 9)
Para ver la secuencia completa pregrabada, presiona **`F4`** y después **`F2`**:

|                        Tecla                        |                               Acción                                  |
| :---                                                | :---                                                                  |
| `F4`                                                | Cargar la secuencia de keyframes desde `animacion_linterna.txt`       |
| `F2`                                                | Reproducir / pausar la animación por keyframes                        |
| `F5`                                                | Reiniciar el personaje al primer keyframe                             |
| `F1` / `F3`                                         | Guardar nuevo keyframe / exportar animación a archivo                 |
| `,` / `.`                                           | Soltar linterna en el agua / restablecer linterna en mano             |
| `T/G`, `Y/H`, `U/J`, `I/K`                          | Mover y rotar personaje articulado manualmente                        |
| `O/L`, `R/F`, `E/Q`, `B/V`, `C/X`, `P/Z`, `[` / `]` | Control manual de torso, cabeza, brazos, piernas y tamaño de linterna |

### Interacción por proximidad
Al acercar la cámara a cualquiera de los cuatro monumentos **Gorin-to** ubicados en los costados del río, la barra de título de la ventana muestra datos culturales sobre el Obon y el Tōrō Nagashi.

---

## Estructura del repositorio

* **`Ejecutable/`** — Versión lista para ejecutar (`ProyectoFinal.exe`), bibliotecas `.dll` y recursos (`Models/`, `Shader/`, `Skybox/`, `animacion_linterna.txt`), sin código fuente.
* **`ConfigInicial/`** — Código fuente en C++ (`Proyecto_Lab.cpp`, encabezados, shaders y recursos) y archivo de proyecto de Visual Studio 2022.
* **`External Libraries/`** — Dependencias externas (Assimp, GLEW, GLFW, GLM y SOIL2) configuradas con rutas relativas.
* **`ConfigInicial.sln`** — Solución de Visual Studio 2022 (compilación en plataforma **x86**).
