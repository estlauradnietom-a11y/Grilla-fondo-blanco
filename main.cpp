#include<iostream>
#include<vector>
#include <cstdlib>

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>


#include<iostream>
#include<vector>
#include <cstdlib>

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"Shader.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"


// 
//eL LIENZO


const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;

const int COLS = 60;
const int ROWS = 60;

const float CELL_SIZE = (float)WIDTH / COLS;


//
// VECTOR PARA GUARDAR LOS PUNTOS DE BRESENHAM
// 

std::vector<GLfloat> puntosLinea;


// 
// FUNCION PARA GUARDAR UNA CELDA COMO PUNTO
//

void guardarPunto(
    int x,
    int y,
    float cellSize
)
{
    // Convertimos la celda en el centro de la celda
    float pixelX = (x + 0.5f) * cellSize;
    float pixelY = (y + 0.5f) * cellSize;

    puntosLinea.push_back(pixelX);
    puntosLinea.push_back(pixelY);
    puntosLinea.push_back(0.0f);
}


// =========================================================
// BRESENHAM HORIZONTAL
//
// Esta es la version para cuando domina X.
//
// Basada en:
//
// dx = x1 - x0
// dy = y1 - y0
// p = 2*dy - dx
//
// for x desde x0 hasta x1
//      plot(x,y)
//
//      if p >= 0
//          y += dir
//          p = p - 2*dx
//
//      p = p + 2*dy
// =========================================================

void drawLineH(
    int x0,
    int y0,
    int x1,
    int y1,
    float cellSize
)
{
    // -----------------------------------------------------
    // Nos aseguramos de recorrer de izquierda a derecha
    // -----------------------------------------------------

    if (x0 > x1)
    {
        int temporal;

        temporal = x0;
        x0 = x1;
        x1 = temporal;

        temporal = y0;
        y0 = y1;
        y1 = temporal;
    }


    // -----------------------------------------------------
    // Calcular dx y dy
    // -----------------------------------------------------

    int dx = x1 - x0;
    int dy = y1 - y0;


    // -----------------------------------------------------
    // Determinar la direccion de Y
    //
    // Si dy es negativo, Y debe disminuir.
    // Si dy es positivo, Y debe aumentar.
    // -----------------------------------------------------

    int dir;

    if (dy < 0)
    {
        dir = -1;
    }
    else
    {
        dir = 1;
    }


    // -----------------------------------------------------
    // Convertimos dy en positivo
    //
    // El signo original queda guardado en dir.
    // -----------------------------------------------------

    dy = abs(dy);


    // -----------------------------------------------------
    // Y comienza en y0
    // -----------------------------------------------------

    int y = y0;


    // -----------------------------------------------------
    // PARAMETRO DE DECISION DE BRESENHAM
    // -----------------------------------------------------

    int p = 2 * dy - dx;


    // -----------------------------------------------------
    // Recorrer X
    // -----------------------------------------------------

    for (int x = x0; x <= x1; x++)
    {

        // ---------------------------------------------
        // Dibujar / guardar el punto
        // ---------------------------------------------

        guardarPunto(
            x,
            y,
            cellSize
        );


        // ---------------------------------------------
        // Decidir si debemos cambiar Y
        // ---------------------------------------------

        if (p >= 0)
        {
            y += dir;

            p = p - 2 * dx;
        }


        // ---------------------------------------------
        // Actualizar parametro de decision
        // ---------------------------------------------

        p = p + 2 * dy;
    }
}


// =========================================================
// BRESENHAM VERTICAL
//
// Esta es la version para cuando domina Y.
//
// Basada en:
//
// dx = x1 - x0
// dy = y1 - y0
// p = 2*dx - dy
//
// for y desde y0 hasta y1
//      plot(x,y)
//
//      if p >= 0
//          x += dir
//          p = p - 2*dy
//
//      p = p + 2*dx
// =========================================================

void drawLineV(
    int x0,
    int y0,
    int x1,
    int y1,
    float cellSize
)
{
    // -----------------------------------------------------
    // Nos aseguramos de recorrer de arriba hacia abajo
    // -----------------------------------------------------

    if (y0 > y1)
    {
        int temporal;

        temporal = x0;
        x0 = x1;
        x1 = temporal;

        temporal = y0;
        y0 = y1;
        y1 = temporal;
    }


    // -----------------------------------------------------
    // Calcular dx y dy
    // -----------------------------------------------------

    int dx = x1 - x0;
    int dy = y1 - y0;


    // -----------------------------------------------------
    // Determinar la direccion de X
    //
    // Si dx es negativo, X debe disminuir.
    // Si dx es positivo, X debe aumentar.
    // -----------------------------------------------------

    int dir;

    if (dx < 0)
    {
        dir = -1;
    }
    else
    {
        dir = 1;
    }


    // -----------------------------------------------------
    // Convertimos dx en positivo
    //
    // El signo original queda guardado en dir.
    // -----------------------------------------------------

    dx = abs(dx);


    // -----------------------------------------------------
    // X comienza en x0
    // -----------------------------------------------------

    int x = x0;


    // -----------------------------------------------------
    // PARAMETRO DE DECISION DE BRESENHAM
    // -----------------------------------------------------

    int p = 2 * dx - dy;


    // -----------------------------------------------------
    // Recorrer Y
    // -----------------------------------------------------

    for (int y = y0; y <= y1; y++)
    {

        // ---------------------------------------------
        // Dibujar / guardar el punto
        // ---------------------------------------------

        guardarPunto(
            x,
            y,
            cellSize
        );


        // ---------------------------------------------
        // Decidir si debemos cambiar X
        // ---------------------------------------------

        if (p >= 0)
        {
            x += dir;

            p = p - 2 * dy;
        }


        // ---------------------------------------------
        // Actualizar parametro de decision
        // ---------------------------------------------

        p = p + 2 * dx;
    }
}



// MAIN
int main()
{

    // PUNTO INICIAL Y PUNTO FINAL

    int inicioX = 1;
    int inicioY = 0;

    int finalX = 10;
    int finalY = 10;


    // =====================================================
    // INICIALIZAR GLFW
    // =====================================================

    glfwInit();

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3
    );

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        3
    );

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );


    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Plantilla de Grilla - Paint",
        NULL,
        NULL
    );


    if (window == NULL)
    {
        std::cout
            << "Fallo al crear la ventana de GLFW"
            << std::endl;

        glfwTerminate();

        return -1;
    }


    glfwMakeContextCurrent(window);


    // =====================================================
    // INICIALIZAR GLAD
    // =====================================================

    gladLoadGL();


    glViewport(
        0,
        0,
        WIDTH,
        HEIGHT
    );


    // =====================================================
    // CREAR LA GRILLA
    //
    // Cada celda se crea como un CUADRADO individual.
    // =====================================================

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;


    for (int fila = 0; fila < ROWS; fila++)
    {
        for (int columna = 0; columna < COLS; columna++)
        {

            // -------------------------------------------------
            // Posicion de la esquina superior izquierda
            // -------------------------------------------------

            float x = columna * CELL_SIZE;
            float y = fila * CELL_SIZE;


            // -------------------------------------------------
            // Primer indice de los vertices de este cuadrado
            // -------------------------------------------------

            GLuint inicio =
                (GLuint)(vertices.size() / 6);


            // =================================================
            // VERTICE 0
            // Superior izquierda
            // =================================================

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);

            // Color
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);


            // =================================================
            // VERTICE 1
            // Superior derecha
            // =================================================

            vertices.push_back(
                x + CELL_SIZE
            );

            vertices.push_back(y);
            vertices.push_back(0.0f);

            // Color
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);


            // =================================================
            // VERTICE 2
            // Inferior derecha
            // =================================================

            vertices.push_back(
                x + CELL_SIZE
            );

            vertices.push_back(
                y + CELL_SIZE
            );

            vertices.push_back(0.0f);

            // Color
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);


            // =================================================
            // VERTICE 3
            // Inferior izquierda
            // =================================================

            vertices.push_back(x);

            vertices.push_back(
                y + CELL_SIZE
            );

            vertices.push_back(0.0f);

            // Color
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);


            // =================================================
            // INDICES DEL CUADRADO
       

            // Arriba
            indices.push_back(inicio + 0);
            indices.push_back(inicio + 1);

            // Derecha
            indices.push_back(inicio + 1);
            indices.push_back(inicio + 2);

            // Abajo
            indices.push_back(inicio + 2);
            indices.push_back(inicio + 3);

            // Izquierda
            indices.push_back(inicio + 3);
            indices.push_back(inicio + 0);
        }
    }


    // =====================================================
    // SHADER
    // =====================================================

    Shader shaderProgram(
        "Shaders/default.vert",
        "Shaders/default.frag"
    );


    // =====================================================
    // VAO, VBO Y EBO DE LA GRILLA
    // =====================================================

    VAO VAO1;

    VAO1.Bind();


    VBO VBO1(
        vertices.data(),
        vertices.size() * sizeof(GLfloat)
    );


    EBO EBO1(
        indices.data(),
        indices.size() * sizeof(GLuint)
    );


    // -----------------------------------------------------
    // POSICION
    // X Y Z
    // location 0
    // -----------------------------------------------------

    VAO1.LinkAttrib(
        VBO1,
        0,
        3,
        GL_FLOAT,
        6 * sizeof(float),
        (void*)0
    );


    // -----------------------------------------------------
    // COLOR
    // R G B
    // location 1
    // -----------------------------------------------------

    VAO1.LinkAttrib(
        VBO1,
        1,
        3,
        GL_FLOAT,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );


    VAO1.Unbind();

    VBO1.Unbind();

    EBO1.Unbind();


    // =====================================================
    // VAO Y VBO PARA LOS PUNTOS DE BRESENHAM
    // =====================================================

    VAO VAODibujo;


    float puntoInicial[3] =
    {
        0.0f,
        0.0f,
        0.0f
    };


    VBO VBODibujo(
        puntoInicial,
        sizeof(puntoInicial)
    );


    VAODibujo.Bind();


    VAODibujo.LinkAttrib(
        VBODibujo,
        0,
        3,
        GL_FLOAT,
        3 * sizeof(float),
        (void*)0
    );


    VAODibujo.Unbind();

    VBODibujo.Unbind();


    // =====================================================
    // SELECCIONAR EL ALGORITMO DE BRESENHAM
    // =====================================================

    puntosLinea.clear();


    int diferenciaX =
        abs(finalX - inicioX);

    int diferenciaY =
        abs(finalY - inicioY);


    // -----------------------------------------------------
    // Si X domina:
    //
    // dx >= dy
    //
    // utilizamos drawLineH
    // -----------------------------------------------------

    if (diferenciaX >= diferenciaY)
    {
        drawLineH(
            inicioX,
            inicioY,
            finalX,
            finalY,
            CELL_SIZE
        );
    }


    // -----------------------------------------------------
    // Si Y domina:
    //
    // dy > dx
    //
    // utilizamos drawLineV
    // -----------------------------------------------------

    else
    {
        drawLineV(
            inicioX,
            inicioY,
            finalX,
            finalY,
            CELL_SIZE
        );
    }


    // =====================================================
    // ENVIAR LOS PUNTOS AL VBO
    // =====================================================

    VBODibujo.UpdateData(
        puntosLinea.data(),
        puntosLinea.size() * sizeof(GLfloat)
    );


    // =====================================================
    // MOUSE
    //
 
    //
    // 1. Primer click  -> punto inicial
    // 2. Movimiento    -> punto final
    // 3. Bresenham     -> calcula la linea
    //
    // =====================================================

    /*

    bool dibujando = false;

    int mouseX = 0;
    int mouseY = 0;


    // Dentro del while:

    double xpos;
    double ypos;

    glfwGetCursorPos(
        window,
        &xpos,
        &ypos
    );


    mouseX = (int)xpos;
    mouseY = (int)ypos;


    if (
        glfwGetMouseButton(
            window,
            GLFW_MOUSE_BUTTON_LEFT
        ) == GLFW_PRESS
    )
    {
        if (!dibujando)
        {
            dibujando = true;

            inicioX =
                (int)(mouseX / CELL_SIZE);

            inicioY =
                (int)(mouseY / CELL_SIZE);
        }


        finalX =
            (int)(mouseX / CELL_SIZE);

        finalY =
            (int)(mouseY / CELL_SIZE);


        puntosLinea.clear();


        int diferenciaX =
            abs(finalX - inicioX);

        int diferenciaY =
            abs(finalY - inicioY);


        if (diferenciaX >= diferenciaY)
        {
            drawLineH(
                inicioX,
                inicioY,
                finalX,
                finalY,
                CELL_SIZE
            );
        }
        else
        {
            drawLineV(
                inicioX,
                inicioY,
                finalX,
                finalY,
                CELL_SIZE
            );
        }


        VBODibujo.UpdateData(
            puntosLinea.data(),
            puntosLinea.size() * sizeof(GLfloat)
        );
    }
    else
    {
        dibujando = false;
    }

    */


    // =====================================================
    // MATRIZ DE PROYECCION
    // =====================================================

    glm::mat4 proj = glm::ortho(
        0.0f,
        (float)WIDTH,
        (float)HEIGHT,
        0.0f,
        -1.0f,
        1.0f
    );


    // =====================================================
    // BUCLE PRINCIPAL
    // =====================================================

    while (!glfwWindowShouldClose(window))
    {

        // -------------------------------------------------
        // FONDO BLANCO
        // -------------------------------------------------

        glClearColor(
            1.0f,
            1.0f,
            1.0f,
            1.0f
        );


        glClear(
            GL_COLOR_BUFFER_BIT
        );


        // -------------------------------------------------
        // ACTIVAR SHADER
        // -------------------------------------------------

        shaderProgram.Activate();


        glUniformMatrix4fv(
            glGetUniformLocation(
                shaderProgram.ID,
                "proj"
            ),
            1,
            GL_FALSE,
            glm::value_ptr(proj)
        );


        // =================================================
        // DIBUJAR GRILLA
        // =================================================

        VAO1.Bind();


        glDrawElements(
            GL_LINES,
            (GLsizei)indices.size(),
            GL_UNSIGNED_INT,
            0
        );


        // =================================================
        // DIBUJAR LOS PUNTOS DE BRESENHAM
        // =================================================

        VAODibujo.Bind();


        glPointSize(5.0f);


        glDrawArrays(
            GL_POINTS,
            0,
            (GLsizei)(puntosLinea.size() / 3)
        );


        VAODibujo.Unbind();


        // -------------------------------------------------
        // MOSTRAR EN PANTALLA
        // -------------------------------------------------

        glfwSwapBuffers(window);

        glfwPollEvents();
    }


    // =====================================================
    // LIMPIEZA
    // =====================================================

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
