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
// BRESENHAM HORIZONTAL (Versión para cuando domina X)
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
// BRESENHAM VERTICAL (Versión para cuando domina Y)
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

        // Simetría de 8 octantes usando guardarPunto
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


// ingreso de coordenadas 


int main()
{
    int inicioX = 1;
    int inicioY = 0;

    int finalX = 10;
    int finalY = 10;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Plantilla de Grilla - Paint", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Fallo al crear la ventana de GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, WIDTH, HEIGHT);

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    for (int fila = 0; fila < ROWS; fila++)
    {
        for (int columna = 0; columna < COLS; columna++)
        {
            float x = columna * CELL_SIZE;
            float y = fila * CELL_SIZE;

            GLuint inicio = (GLuint)(vertices.size() / 6);

            // Vértice 0: Superior izquierda
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f);

            // Vértice 1: Superior derecha
            vertices.push_back(x + CELL_SIZE);
            vertices.push_back(y);
            vertices.push_back(0.0f);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f);

            // Vértice 2: Inferior derecha
            vertices.push_back(x + CELL_SIZE);
            vertices.push_back(y + CELL_SIZE);
            vertices.push_back(0.0f);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f);

            // Vértice 3: Inferior izquierda
            vertices.push_back(x);
            vertices.push_back(y + CELL_SIZE);
            vertices.push_back(0.0f);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f);

            // Índices del cuadrado
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

    VAO VAODibujo;
    float puntoInicial[3] = { 0.0f, 0.0f, 0.0f };
    VBO VBODibujo(puntoInicial, sizeof(puntoInicial));

    VAODibujo.Bind();
    VAODibujo.LinkAttrib(VBODibujo, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
    VAODibujo.Unbind();
    VBODibujo.Unbind();

 
    puntosLinea.clear();

    // Dibujar Círculo (Centro X, Centro Y, Radio, Tamaño de Celda)
    int centroX = 30;
    int centroY = 30;
    int radio = 2;

    drawCircle(centroX, centroY, radio, CELL_SIZE);

    // Opcional: Si quieres mantener también la línea en la misma pantalla, 
    // puedes dejar la llamada a drawLineH / drawLineV aquí abajo sin borrar los puntos.

    VBODibujo.UpdateData(puntosLinea.data(), puntosLinea.size() * sizeof(GLfloat));

    int diferenciaX = abs(finalX - inicioX);
    int diferenciaY = abs(finalY - inicioY);

    if (diferenciaX >= diferenciaY)
    {
        drawLineH(inicioX, inicioY, finalX, finalY, CELL_SIZE);
    }
    else
    {
        drawLineV(inicioX, inicioY, finalX, finalY, CELL_SIZE);
    }

    VBODibujo.UpdateData(puntosLinea.data(), puntosLinea.size() * sizeof(GLfloat));

    glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram.ID, "proj"),
            1,
            GL_FALSE,
            glm::value_ptr(proj)
        );

        // Dibujar grilla
        VAO1.Bind();
        glDrawElements(GL_LINES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        // Dibujar los puntos de Bresenham
        VAODibujo.Bind();
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, (GLsizei)(puntosLinea.size() / 3));
        VAODibujo.Unbind();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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