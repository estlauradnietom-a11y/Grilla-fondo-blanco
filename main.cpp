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

// Variables para control interactivo en ImGui
float offsetFiguraX = 0.0f;
float offsetFiguraY = 0.0f;
float colorLinea[3] = { 0.0f, 0.0f, 0.0f }; // Color RGB de las figuras
bool rellenarForma = false;  // Activador de relleno

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;

const int COLS = 60;
const int ROWS = 60;

const float CELL_SIZE = (float)WIDTH / COLS;

std::vector<GLfloat> puntosLinea;

void guardarPunto(int x, int y, float cellSize)
{
    float pixelX = (x + 0.5f) * cellSize;
    float pixelY = (y + 0.5f) * cellSize;

    puntosLinea.push_back(pixelX);
    puntosLinea.push_back(pixelY);
    puntosLinea.push_back(0.0f);
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

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Grilla Bresenham con ImGui", NULL, NULL);
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

    // VAO para los puntos dibujados
    VAO VAODibujo;
    float puntoInicial[3] = { 0.0f, 0.0f, 0.0f };
    VBO VBODibujo(puntoInicial, sizeof(puntoInicial));

    VAODibujo.Bind();
    VAODibujo.LinkAttrib(VBODibujo, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
    VAODibujo.Unbind();
    VBODibujo.Unbind();

    glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);

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

        // 2. Crear Panel de Control ImGui
        ImGui::Begin("Panel de Control - Bresenham");
        ImGui::Text("Transformaciones y Estilos");
        ImGui::SliderFloat("Mover X", &offsetFiguraX, -25.0f, 25.0f);
        ImGui::SliderFloat("Mover Y", &offsetFiguraY, -25.0f, 25.0f);
        ImGui::ColorEdit3("Color Figuras", colorLinea);
        ImGui::Checkbox("Rellenar Formas", &rellenarForma);
        ImGui::End();

        // 3. Limpiar pantalla
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 4. Recalcular posiciones de figuras por fotograma según ImGui
        puntosLinea.clear();

        int centroX = 30 + (int)offsetFiguraX;
        int centroY = 30 + (int)offsetFiguraY;
        int radio = 8;

        int triX0 = 10 + (int)offsetFiguraX, triY0 = 20 + (int)offsetFiguraY;
        int triX1 = 20 + (int)offsetFiguraX, triY1 = 10 + (int)offsetFiguraY;
        int triX2 = 25 + (int)offsetFiguraX, triY2 = 20 + (int)offsetFiguraY;

        int linX0 = 35 + (int)offsetFiguraX, linY0 = 35 + (int)offsetFiguraY;
        int linX1 = 50 + (int)offsetFiguraX, linY1 = 50 + (int)offsetFiguraY;

        if (rellenarForma)
        {
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
                    bool enCirculo = ((c - centroX) * (c - centroX) + (r - centroY) * (r - centroY)) <= (radio * radio);
                    bool enTriangulo = puntoEnTriangulo(c, r, triX0, triY0, triX1, triY1, triX2, triY2);

                    if (enCirculo || enTriangulo)
                    {
                        guardarPunto(c, r, CELL_SIZE);
                    }
                }
            }
        }
        else
        {
            drawCircle(centroX, centroY, radio, CELL_SIZE);
            drawTriangle(triX0, triY0, triX1, triY1, triX2, triY2, CELL_SIZE);
            drawLine(linX0, linY0, linX1, linY1, CELL_SIZE);
        }

        // Actualizar VBO de dibujo de manera segura con OpenGL
        if (!puntosLinea.empty())
        {
            VBODibujo.Bind();
            glBufferData(GL_ARRAY_BUFFER, puntosLinea.size() * sizeof(GLfloat), puntosLinea.data(), GL_DYNAMIC_DRAW);
            VBODibujo.Unbind();
        }

        // 5. Activar Shader y enviar Uniforms
        shaderProgram.Activate();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "colorPersonalizado"), colorLinea[0], colorLinea[1], colorLinea[2]);

        // 6. Dibujar Grilla
        VAO1.Bind();
        glDrawElements(GL_LINES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        // 7. Dibujar Puntos / Figuras
        if (!puntosLinea.empty())
        {
            VAODibujo.Bind();
            glPointSize(6.0f);
            glDrawArrays(GL_POINTS, 0, (GLsizei)(puntosLinea.size() / 3));
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