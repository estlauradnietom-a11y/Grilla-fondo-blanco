#include <iostream>
#include <vector>
#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;

// Ancho del panel del menu, que va a la derecha de la grilla (Paso 3)
const unsigned int PANEL_WIDTH = 300;

const int COLS = 60;
const int ROWS = 60;

const float CELL_SIZE = (float)WIDTH / COLS;

// =========================================================
// PASO 2: CADA FIGURA ES UN OBJETO
// =========================================================

// Un punto de referencia de la figura, medido en celdas de la grilla
// (x = columna, y = fila)
struct Punto
{
    int x;
    int y;
};

// Los tipos de figura que sabe dibujar el programa
enum TipoFigura
{
    CIRCULO,
    TRIANGULO,
    LINEA
};

// Toda la informacion que necesita una figura para dibujarse
struct Figura
{
    const char* nombre;          // Nombre que se mostrara en el menu
    TipoFigura tipo;             // Circulo, triangulo o linea
    std::vector<Punto> puntos;   // Puntos de referencia:
                                 //   circulo   -> 1 punto (el centro)
                                 //   triangulo -> 3 puntos (los vertices)
                                 //   linea     -> 2 puntos (inicio y fin)
    int radio;                   // Solo lo usa el circulo
    float colorBorde[3];         // Color RGB del contorno (valores de 0 a 1)
    float colorRelleno[3];       // Color RGB del interior
    bool rellenar;               // true = pintar el interior
    int desplazamientoX;         // Celdas que se movio en X (Paso 3)
    int desplazamientoY;         // Celdas que se movio en Y (Paso 3)
};

// Lista con todas las figuras del dibujo
std::vector<Figura> figuras;

// Posicion en la lista de la figura elegida en el menu (0 = la primera)
int figuraSeleccionada = 0;

// Puntos (celdas) que se pintan en cada fotograma.
// Cada punto ocupa 6 numeros: posicion (x, y, z) + color (r, g, b),
// el mismo formato que usan los vertices de la grilla.
std::vector<GLfloat> puntosDibujo;

// "Pincel": color con el que guardarPunto pinta en este momento
float colorActual[3] = { 0.0f, 0.0f, 0.0f };

void usarColor(const float color[3])
{
    colorActual[0] = color[0];
    colorActual[1] = color[1];
    colorActual[2] = color[2];
}

void guardarPunto(int x, int y, float cellSize)
{
    // Si la celda queda fuera de la grilla no se pinta
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return;

    float pixelX = (x + 0.5f) * cellSize;
    float pixelY = (y + 0.5f) * cellSize;

    // Posicion
    puntosDibujo.push_back(pixelX);
    puntosDibujo.push_back(pixelY);
    puntosDibujo.push_back(0.0f);

    // Color (el del pincel actual)
    puntosDibujo.push_back(colorActual[0]);
    puntosDibujo.push_back(colorActual[1]);
    puntosDibujo.push_back(colorActual[2]);
}

// =========================================================
// BRESENHAM HORIZONTAL (domina X)
// =========================================================
void drawLineH(int x0, int y0, int x1, int y1, float cellSize)
{
    if (x0 > x1)
    {
        int temporal = x0;
        x0 = x1;
        x1 = temporal;

        temporal = y0;
        y0 = y1;
        y1 = temporal;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int dir = (dy < 0) ? -1 : 1;
    dy = abs(dy);

    int y = y0;
    int p = 2 * dy - dx;

    for (int x = x0; x <= x1; x++)
    {
        guardarPunto(x, y, cellSize);

        if (p >= 0)
        {
            y += dir;
            p = p - 2 * dx;
        }

        p = p + 2 * dy;
    }
}

// =========================================================
// BRESENHAM VERTICAL (domina Y)
// =========================================================
void drawLineV(int x0, int y0, int x1, int y1, float cellSize)
{
    if (y0 > y1)
    {
        int temporal = x0;
        x0 = x1;
        x1 = temporal;

        temporal = y0;
        y0 = y1;
        y1 = temporal;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int dir = (dx < 0) ? -1 : 1;
    dx = abs(dx);

    int x = x0;
    int p = 2 * dx - dy;

    for (int y = y0; y <= y1; y++)
    {
        guardarPunto(x, y, cellSize);

        if (p >= 0)
        {
            x += dir;
            p = p - 2 * dy;
        }

        p = p + 2 * dx;
    }
}

// =========================================================
// FUNCIÓN AUXILIAR DE LÍNEA (Selector H/V)
// =========================================================
void drawLine(int x0, int y0, int x1, int y1, float cellSize)
{
    int diferenciaX = abs(x1 - x0);
    int diferenciaY = abs(y1 - y0);

    if (diferenciaX >= diferenciaY)
    {
        drawLineH(x0, y0, x1, y1, cellSize);
    }
    else
    {
        drawLineV(x0, y0, x1, y1, cellSize);
    }
}

// =========================================================
// BRESENHAM PARA TRIÁNGULOS
// =========================================================
void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, float cellSize)
{
    drawLine(x0, y0, x1, y1, cellSize);
    drawLine(x1, y1, x2, y2, cellSize);
    drawLine(x2, y2, x0, y0, cellSize);
}

// =========================================================
// BRESENHAM PARA CÍRCULOS
// =========================================================
void drawCircle(int cx, int cy, int r, float cellSize)
{
    int x = 0;
    int y = -r;
    int p = -r;

    while (x < -y)
    {
        if (p > 0)
        {
            y += 1;
            p += 2 * (x + y) + 1;
        }
        else
        {
            p += 2 * x + 1;
        }

        guardarPunto(cx + x, cy + y, cellSize);
        guardarPunto(cx - x, cy + y, cellSize);
        guardarPunto(cx + x, cy - y, cellSize);
        guardarPunto(cx - x, cy - y, cellSize);
        guardarPunto(cx + y, cy + x, cellSize);
        guardarPunto(cx + y, cy - x, cellSize);
        guardarPunto(cx - y, cy + x, cellSize);
        guardarPunto(cx - y, cy - x, cellSize);

        x += 1;
    }
}

// =========================================================
// PRUEBA DE PUNTO DENTRO DE TRIÁNGULO (Para Relleno)
// =========================================================
bool puntoEnTriangulo(int px, int py, int x0, int y0, int x1, int y1, int x2, int y2)
{
    int denominador = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
    if (denominador == 0) return false;
    float a = ((y1 - y2) * (px - x2) + (x2 - x1) * (py - y2)) / (float)denominador;
    float b = ((y2 - y0) * (px - x2) + (x0 - x2) * (py - y2)) / (float)denominador;
    float c = 1.0f - a - b;
    return (a >= 0.0f && a <= 1.0f && b >= 0.0f && b <= 1.0f && c >= 0.0f && c <= 1.0f);
}

// =========================================================
// PASO 2: DIBUJAR UNA FIGURA (relleno + borde)
// =========================================================

// Pinta el interior recorriendo todas las celdas de la grilla
// y preguntando si cada celda esta dentro de la figura
void rellenarFigura(const Figura& f)
{
    if (f.tipo == LINEA) return;   // una linea no tiene interior

    usarColor(f.colorRelleno);

    for (int fila = 0; fila < ROWS; fila++)
    {
        for (int columna = 0; columna < COLS; columna++)
        {
            bool adentro = false;

            if (f.tipo == CIRCULO)
            {
                int dx = columna - f.puntos[0].x;
                int dy = fila - f.puntos[0].y;
                adentro = (dx * dx + dy * dy) <= (f.radio * f.radio);
            }
            else if (f.tipo == TRIANGULO)
            {
                adentro = puntoEnTriangulo(columna, fila,
                    f.puntos[0].x, f.puntos[0].y,
                    f.puntos[1].x, f.puntos[1].y,
                    f.puntos[2].x, f.puntos[2].y);
            }

            if (adentro)
            {
                guardarPunto(columna, fila, CELL_SIZE);
            }
        }
    }
}

// Dibuja el contorno usando los algoritmos de Bresenham de arriba
void dibujarBorde(const Figura& f)
{
    usarColor(f.colorBorde);

    if (f.tipo == CIRCULO)
    {
        drawCircle(f.puntos[0].x, f.puntos[0].y, f.radio, CELL_SIZE);
    }
    else if (f.tipo == TRIANGULO)
    {
        drawTriangle(f.puntos[0].x, f.puntos[0].y,
                     f.puntos[1].x, f.puntos[1].y,
                     f.puntos[2].x, f.puntos[2].y, CELL_SIZE);
    }
    else if (f.tipo == LINEA)
    {
        drawLine(f.puntos[0].x, f.puntos[0].y,
                 f.puntos[1].x, f.puntos[1].y, CELL_SIZE);
    }
}

// =========================================================
// PASO 3: TRANSFORMACIONES
// =========================================================
// Devuelve una COPIA de la figura con sus puntos ya movidos.
// La figura original no se modifica: solo guarda cuanto se ha movido.
// Asi siempre partimos de los puntos originales y la figura no se deforma.
Figura aplicarTransformaciones(const Figura& f)
{
    Figura resultado = f;   // copia de la figura

    for (int i = 0; i < (int)resultado.puntos.size(); i++)
    {
        resultado.puntos[i].x += f.desplazamientoX;
        resultado.puntos[i].y += f.desplazamientoY;
    }

    return resultado;
}

// Primero se transforma la figura; luego se pinta el relleno
// y despues el borde, para que el borde quede encima
void dibujarFigura(const Figura& f)
{
    Figura g = aplicarTransformaciones(f);

    if (g.rellenar)
    {
        rellenarFigura(g);
    }
    dibujarBorde(g);
}

// Crea las 3 figuras iniciales (mismas coordenadas que ya tenia el proyecto).
// desplazamientoX y desplazamientoY no se escriben: empiezan en 0.
void crearFiguras()
{
    //                  nombre       tipo       puntos de referencia               radio  color borde              color relleno            rellenar
    figuras.push_back({ "Circulo",   CIRCULO,   { {30, 30} },                        8,   { 0.85f, 0.10f, 0.10f }, { 1.00f, 0.65f, 0.65f }, true  });
    figuras.push_back({ "Triangulo", TRIANGULO, { {10, 20}, {20, 10}, {25, 20} },    0,   { 0.10f, 0.25f, 0.85f }, { 0.60f, 0.75f, 1.00f }, false });
    figuras.push_back({ "Linea",     LINEA,     { {35, 35}, {50, 50} },              0,   { 0.05f, 0.55f, 0.20f }, { 0.00f, 0.00f, 0.00f }, false });
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // tamano fijo para que el panel no se descuadre

    // La ventana mide: grilla (800) + panel del menu (300) de ancho
    GLFWwindow* window = glfwCreateWindow(WIDTH + PANEL_WIDTH, HEIGHT, "Grilla Bresenham con ImGui", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Fallo al crear la ventana de GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, WIDTH, HEIGHT);

    // Inicializar ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Generar geometría de la grilla de 60x60
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    for (int fila = 0; fila < ROWS; fila++)
    {
        for (int columna = 0; columna < COLS; columna++)
        {
            float x = columna * CELL_SIZE;
            float y = fila * CELL_SIZE;

            GLuint inicio = (GLuint)(vertices.size() / 6);

            vertices.push_back(x); vertices.push_back(y); vertices.push_back(0.0f);
            vertices.push_back(0.85f); vertices.push_back(0.85f); vertices.push_back(0.85f); // Gris claro

            vertices.push_back(x + CELL_SIZE); vertices.push_back(y); vertices.push_back(0.0f);
            vertices.push_back(0.85f); vertices.push_back(0.85f); vertices.push_back(0.85f);

            vertices.push_back(x + CELL_SIZE); vertices.push_back(y + CELL_SIZE); vertices.push_back(0.0f);
            vertices.push_back(0.85f); vertices.push_back(0.85f); vertices.push_back(0.85f);

            vertices.push_back(x); vertices.push_back(y + CELL_SIZE); vertices.push_back(0.0f);
            vertices.push_back(0.85f); vertices.push_back(0.85f); vertices.push_back(0.85f);

            indices.push_back(inicio + 0); indices.push_back(inicio + 1);
            indices.push_back(inicio + 1); indices.push_back(inicio + 2);
            indices.push_back(inicio + 2); indices.push_back(inicio + 3);
            indices.push_back(inicio + 3); indices.push_back(inicio + 0);
        }
    }

    Shader shaderProgram("Shaders/default.vert", "Shaders/default.frag");

    VAO VAO1;
    VAO1.Bind();
    VBO VBO1(vertices.data(), vertices.size() * sizeof(GLfloat));
    EBO EBO1(indices.data(), indices.size() * sizeof(GLuint));

    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    // VAO para los puntos de las figuras: posicion (location 0) + color (location 1),
    // 6 floats por punto, igual que los vertices de la grilla
    VAO VAODibujo;
    GLfloat puntoInicial[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    VBO VBODibujo(puntoInicial, sizeof(puntoInicial));

    VAODibujo.Bind();
    VAODibujo.LinkAttrib(VBODibujo, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAODibujo.LinkAttrib(VBODibujo, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    VAODibujo.Unbind();
    VBODibujo.Unbind();

    glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);

    // Crear las figuras del dibujo (Paso 2)
    crearFiguras();

    // =========================================================
    // BUCLE PRINCIPAL UNIFICADO
    // =========================================================
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // 1. Iniciar frame de ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 2. Panel de ImGui fijo a la derecha de la grilla (Paso 3)
        ImGui::SetNextWindowPos(ImVec2((float)WIDTH, 0.0f));
        ImGui::SetNextWindowSize(ImVec2((float)PANEL_WIDTH, (float)HEIGHT));
        ImGui::Begin("Menu de figuras", NULL,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // 2a. Menu desplegable para elegir la figura
        if (ImGui::BeginCombo("Figura", figuras[figuraSeleccionada].nombre))
        {
            for (int i = 0; i < (int)figuras.size(); i++)
            {
                if (ImGui::Selectable(figuras[i].nombre, i == figuraSeleccionada))
                {
                    figuraSeleccionada = i;
                }
            }
            ImGui::EndCombo();
        }

        // "f" es la figura elegida. El & hace que editemos la figura real, no una copia
        Figura& f = figuras[figuraSeleccionada];

        // 2b. Color y relleno
        ImGui::SeparatorText("Color");
        ImGui::ColorEdit3("Borde", f.colorBorde);

        if (f.tipo != LINEA)
        {
            ImGui::Checkbox("Rellenar", &f.rellenar);
            if (f.rellenar)
            {
                ImGui::ColorEdit3("Relleno", f.colorRelleno);
            }
        }
        else
        {
            ImGui::TextDisabled("La linea no tiene relleno");
        }

        // 2c. Desplazamiento (en celdas de la grilla)
        ImGui::SeparatorText("Desplazar");
        ImGui::SliderInt("Mover X", &f.desplazamientoX, -COLS, COLS);
        ImGui::SliderInt("Mover Y", &f.desplazamientoY, -ROWS, ROWS);
        ImGui::TextDisabled("X+ = derecha, Y+ = abajo");
        if (ImGui::Button("Reiniciar posicion"))
        {
            f.desplazamientoX = 0;
            f.desplazamientoY = 0;
        }

        // 2d. Coordenadas actuales de los puntos de referencia
        ImGui::SeparatorText("Puntos de referencia");
        Figura actual = aplicarTransformaciones(f);
        for (int i = 0; i < (int)actual.puntos.size(); i++)
        {
            ImGui::Text("P%d: (%d, %d)", i + 1, actual.puntos[i].x, actual.puntos[i].y);
        }
        if (actual.tipo == CIRCULO)
        {
            ImGui::Text("Radio: %d", actual.radio);
        }

        ImGui::End();

        // 3. Limpiar pantalla y usar solo el cuadro de 800x800 de la izquierda para la grilla
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, WIDTH, HEIGHT);

        // 4. Recalcular los puntos de todas las figuras en cada fotograma
        puntosDibujo.clear();

        for (int i = 0; i < (int)figuras.size(); i++)
        {
            dibujarFigura(figuras[i]);
        }

        // Enviar los puntos nuevos a la GPU
        if (!puntosDibujo.empty())
        {
            VBODibujo.UpdateData(puntosDibujo.data(), puntosDibujo.size() * sizeof(GLfloat));
            VBODibujo.Unbind();
        }

        // 5. Activar Shader y enviar Uniforms
        shaderProgram.Activate();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

        // 6. Dibujar Grilla
        VAO1.Bind();
        glDrawElements(GL_LINES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        // 7. Dibujar Puntos / Figuras
        if (!puntosDibujo.empty())
        {
            VAODibujo.Bind();
            glPointSize(6.0f);
            glDrawArrays(GL_POINTS, 0, (GLsizei)(puntosDibujo.size() / 6));  // 6 floats por punto
            VAODibujo.Unbind();
        }

        // 8. Ajustes de OpenGL para renderizar ImGui correctamente encima
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 9. Renderizar Interfaz ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 10. Intercambiar Buffers
        glfwSwapBuffers(window);
    }

    // Limpieza de recursos
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    VAODibujo.Delete();
    VBODibujo.Delete();
    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}