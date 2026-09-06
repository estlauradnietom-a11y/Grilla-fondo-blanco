#ifndef GRID_CLASS_H
#define GRID_CLASS_H

#include<vector>
#include<glad/glad.h>

// Genera los datos de una cuadricula 2D (tipo hoja cuadriculada) para usar
// como plantilla de un lienzo de paint.
//
// Cada vertice tiene: posicion (x, y, z) + color (r, g, b) -> 6 floats.
// Los indices conectan cada interseccion con su vecino de la derecha y de abajo,
// reutilizando vertices compartidos (mismo principio del Index Buffer del curso).
namespace Grid
{
	void Generate(int cols, int rows, float cellSize,
		std::vector<GLfloat>& vertices, std::vector<GLuint>& indices);
}

#endif
