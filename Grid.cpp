#include"Grid.h"

void Grid::Generate(int cols, int rows, float cellSize,
	std::vector<GLfloat>& vertices, std::vector<GLuint>& indices)
{
	vertices.clear();
	indices.clear();

	// Color de las lineas de la cuadricula (gris claro)
	GLfloat r = 0.75f, g = 0.75f, b = 0.75f;

	// 1) Un vertice por cada interseccion de la cuadricula: (cols+1) x (rows+1)
	for (int row = 0; row <= rows; row++)
	{
		for (int col = 0; col <= cols; col++)
		{
			float x = col * cellSize;
			float y = row * cellSize;

			vertices.push_back(x);       // posicion x
			vertices.push_back(y);       // posicion y
			vertices.push_back(0.0f);    // posicion z

			vertices.push_back(r);
			vertices.push_back(g);
			vertices.push_back(b);
		}
	}

	// 2) Indices: conectar cada interseccion con su vecino derecho e inferior.
	//    Esto dibuja todas las lineas de la grilla sin duplicar vertices.
	for (int row = 0; row <= rows; row++)
	{
		for (int col = 0; col <= cols; col++)
		{
			GLuint current = row * (cols + 1) + col;

			if (col < cols)
			{
				GLuint right = current + 1;
				indices.push_back(current);
				indices.push_back(right);
			}

			if (row < rows)
			{
				GLuint below = current + (cols + 1);
				indices.push_back(current);
				indices.push_back(below);
			}
		}
	}
}
