# Rasterización de Figuras en Grilla con Bresenham, Transformaciones 2D e ImGui

## Información del estudiante
- **Nombre:** Laura Daniela Nieto Muñoz
- **Código estudiantil:** 6014538
- **Materia / Curso:** Computación Gráfica MUL A
- **Profesor:** FERNANDO JAVIER NOSSA CHAPARRO

## Descripción general y arquitectura del proyecto

El proyecto es una aplicación interactiva en **C++** que utiliza **OpenGL (GLFW, GLAD, GLM)** y **Dear ImGui** para rasterizar figuras geométricas sobre una cuadrícula discreta de **60 × 60 celdas** (en un lienzo de `800 × 800` píxeles, acompañado de un panel lateral de `300` píxeles para controles).

A nivel de arquitectura, el programa está organizado en los siguientes bloques funcionales:

1. **Modelo de datos (`Punto`, `TipoFigura`, `Figura`):**
   Representa cada figura (`CIRCULO`, `TRIANGULO`, `LINEA`) a partir de sus puntos de referencia en coordenadas de celda, radio (para círculos), colores RGB de borde y relleno, bandera de relleno y parámetros de transformación (`desplazamientoX`, `desplazamientoY` y `angulo`). Mantiene una colección global (`figuras`) y el índice de la figura activa (`figuraSeleccionada`).

2. **Rasterización y relleno sobre grilla:**
   - **Contornos (Algoritmo de Bresenham):** Implementa rasterización discreta para líneas (`drawLineH` con dominio en X, `drawLineV` con dominio en Y y selector `drawLine`), triángulos (`drawTriangle` componiendo tres líneas) y circunferencias (`drawCircle` por simetría de 8 octantes). Cada celda válida se registra mediante `guardarPunto` con posición `(x, y, z)` y color `(r, g, b)`.
   - **Relleno interior (`rellenarFigura`):** Evalúa las celdas de la grilla para determinar si pertenecen al interior de la figura (ecuación de distancia al cuadrado en círculos o coordenadas baricéntricas mediante `puntoEnTriangulo` en triángulos), pintando primero el relleno y luego el borde para que este quede encima.

3. **Transformaciones geométricas 2D (`aplicarTransformaciones`):**
   Calcula el centro de referencia de la figura (`calcularCentro`: centro en círculos, punto medio en líneas y centroide en triángulos) y genera una **copia transformada** en cada ciclo sin alterar los vértices originales (evitando acumulación de errores o deformaciones). Aplica rotación alrededor del centro usando seno y coseno (convertidos a radianes y redondeados a celdas enteras) seguida de traslación en `X` y `Y`.

4. **Pipeline gráfico OpenGL y bucle principal:**
   - Configura el contexto OpenGL 3.3 Core Profile y una proyección ortográfica (`glm::ortho`) alineada con las coordenadas de la grilla.
   - Construye la geometría de la cuadrícula (`VAO1`, `VBO1`, `EBO1` dibujados con `GL_LINES`) y un buffer dinámico (`VAODibujo`, `VBODibujo` dibujado con `GL_POINTS`) que se actualiza en cada fotograma con los puntos generados por los algoritmos de dibujo.
   - Usa un `Shader` (`Shaders/default.vert` y `Shaders/default.frag`) y libera todos los recursos de OpenGL, GLFW e ImGui al cerrar la ventana.

5. **Interfaz de usuario con ImGui:**
   Renderiza un panel fijo (`"Menu de figuras"`) a la derecha de la grilla para seleccionar la figura, cambiar colores de borde y relleno, activar/desactivar el relleno, desplazar en `X`/`Y`, rotar en grados (`-180°` a `180°` en líneas y triángulos) y consultar en tiempo real las coordenadas transformadas y el centro de giro.

## Dependencias y archivos esperados
- **Bibliotecas:** GLFW, GLAD, GLM, Dear ImGui (`imgui`, `imgui_impl_glfw`, `imgui_impl_opengl3`).
- **Headers y shaders locales:** `Shader.h`, `VAO.h`, `VBO.h`, `EBO.h`, `Shaders/default.vert` y `Shaders/default.frag`.
