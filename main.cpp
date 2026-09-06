#include<iostream>
#include<vector>

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"Shader.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Grid.h"

// ---------------------------------------------------------
// Configuracion del lienzo / plantilla del paint
// ---------------------------------------------------------
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;

const int COLS = 20;   // columnas de la cuadricula
const int ROWS = 20;   // filas de la cuadricula
const float CELL_SIZE = (float)WIDTH / COLS; // tamano de cada celda en pixeles

int main()
{
	// ---------------- Inicializar GLFW (igual que en el curso) ----------------
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

	// ---------------- Inicializar GLAD ----------------
	gladLoadGL();
	glViewport(0, 0, WIDTH, HEIGHT);

	// ---------------- Generar los datos de la cuadricula ----------------
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;
	Grid::Generate(COLS, ROWS, CELL_SIZE, vertices, indices);

	// ---------------- Shader ----------------
	Shader shaderProgram("Shaders/default.vert", "Shaders/default.frag");

	// ---------------- VAO, VBO, EBO (mismo patron que el curso) ----------------
	VAO VAO1;
	VAO1.Bind();

	VBO VBO1(vertices.data(), vertices.size() * sizeof(GLfloat));
	EBO EBO1(indices.data(), indices.size() * sizeof(GLuint));

	// posicion (x, y, z) -> location 0
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
	// color (r, g, b) -> location 1
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	// ---------------- Matriz de proyeccion ortografica ----------------
	// Mapea coordenadas de pixel (0,0 arriba-izquierda) directamente a la pantalla,
	// tal como se necesita para una plantilla de dibujo en 2D.
	glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);

	// ---------------- Loop de renderizado ----------------
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // fondo blanco tipo lienzo
		glClear(GL_COLOR_BUFFER_BIT);

		shaderProgram.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

		VAO1.Bind();
		glDrawElements(GL_LINES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ---------------- Limpieza ----------------
	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	shaderProgram.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
