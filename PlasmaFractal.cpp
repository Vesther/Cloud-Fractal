#include <iostream>
#include <iomanip>
#include <fstream>

#include <Windows.h>
#include <gl/GL.h>
#include <GL/glut.h>

const int size = 513; // This is required to be 2^n + 1
int range = 196; // Degree of randomness / roughness. Larger values generate rougher images.

// The 2D array of integer values into which our fractal will be written
int map[size][size];

// Random helper
int rnd(int min = 0, int max = 255)
{
	return min + (rand() % static_cast<int>(max - min + 1));
}

// Init corner values
void init()
{
	map[0][0] = rnd();
	map[0][size - 1] = rnd();
	map[size - 1][0] = rnd();
	map[size - 1][size - 1] = rnd();
}

// Diamond step
void diamond(int sideLength)
{
	int halfSide = sideLength / 2;

	for (int y = 0; y < size / (sideLength-1); y++)
	{
		for (int x = 0; x < size / (sideLength-1); x++)
		{
			int center_x = x*(sideLength-1) + halfSide;
			int center_y = y*(sideLength-1) + halfSide;

			int avg = (map[x*(sideLength - 1)][y*(sideLength - 1)] +
						map[x*(sideLength - 1)][(y+1) * (sideLength - 1)] +
						map[(x + 1) * (sideLength - 1)][y*(sideLength - 1)] +
						map[(x + 1) * (sideLength - 1)][(y + 1) * (sideLength - 1)])
						/ 4.0f;

			map[center_x][center_y] = avg + rnd(-range, range);
		}
	}

}

// Averaging helper function for square step to ignore out of bounds points
void average(int x, int y, int sideLength)
{
	float counter = 0;
	float accumulator = 0;

	int halfSide = sideLength / 2;

	if (x != 0)
	{
		counter += 1.0f;
		accumulator += map[y][x - halfSide];
	}
	if (y != 0)
	{
		counter += 1.0f;
		accumulator += map[y - halfSide][x];
	}
	if (x != size - 1)
	{
		counter += 1.0f;
		accumulator += map[y][x + halfSide];
	}
	if (y != size - 1)
	{
		counter += 1.0f;
		accumulator += map[y + halfSide][x];
	}

	map[y][x] = (accumulator / counter) + rnd(-range, range);
}

// Square step
void square(int sideLength)
{
	int halfLength = sideLength / 2;

	for (int y = 0; y < size / (sideLength - 1); y++)
	{
		for (int x = 0; x < size / (sideLength - 1); x++)
		{
			// Top
			average(x*(sideLength - 1) + halfLength, y*(sideLength - 1), sideLength);
			// Right
			average((x + 1)*(sideLength - 1), y*(sideLength - 1) + halfLength, sideLength);
			// Bottom
			average(x*(sideLength - 1) + halfLength, (y+1)*(sideLength - 1), sideLength);
			// Left
			average(x*(sideLength - 1), y*(sideLength - 1) + halfLength, sideLength);
		}
	}
}

// Main fractal generating loop
void fractal()
{
	int sideLength = size/2;

	diamond(size);
	square(size);

	range /= 2;

	while (sideLength >= 2)
	{
		diamond(sideLength + 1);
		square(sideLength + 1);
		sideLength /= 2;
		range /= 2;
	}
}

// Integer clamping helper
void clamp(int* val, int min, int max)
{
	if (*val < min) *val = min;
	if (*val > max) *val = max;
}

// Function to clamp all map values
void clamp_map()
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			clamp(&map[i][j], 0, 255);
		}
	}
}

// Debug print of the 2D array
void print_map()
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			std::cout << std::setw(10) << map[i][j];
		}
		std::cout << std::endl;
	}
}

// File saving helper, saves to a PGM file (See https://en.wikipedia.org/wiki/Netpbm_format)
void save_to_file()
{
	std::ofstream file_out("test.pgm", std::ios_base::out
		| std::ios_base::binary
		| std::ios_base::trunc
		);
	file_out << "P2" << "\n" << size << "\n" << size << "\n" << 255 << "\n";

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			file_out << map[i][j] << " ";
		}
		file_out << std::endl;
	}

	file_out.close();
}

// OpenGL rendering function
void render()
{
	int color;
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(3);
	glBegin(GL_POINTS);
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				color = map[j][i];
				glColor3ub(color, color, color);
				glVertex2f(i * 100.0f/size, j * 100.0f/size);
			}
		}

	glEnd();
	glFlush();
}

// Window resize handler with Aspect Ratio correction
void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	GLfloat AspectRatio;
	// Division by zero guard
	if (vertical == 0)
		vertical = 1;

	glViewport(0, 0, horizontal, vertical);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;

	if (horizontal <= vertical)
		glOrtho(0, 100.0, 100.0 / AspectRatio, 0, 1.0, -1.0);
	else
		glOrtho(0, 100.0*AspectRatio, 100.0, 0, 1.0, -1.0);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

}

// Handle key presses
void handle_input(unsigned char key, int x, int y)
{
	switch (key)
	{
	// Quit the program on escape key press
	case 27:
		glutDestroyWindow(glutGetWindow());
		exit(0);
		break;
	// Generate a new fractal on space key press
	case 32:
		range = 196;
		init();
		fractal();
		clamp_map();
		glutPostRedisplay();
		save_to_file();
		break;
	}
}

int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	srand(time(NULL));

	init();
	fractal();
	clamp_map();

	//print_map();
	save_to_file();

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

	glutCreateWindow("Plasma fractal");
	glutFullScreen();

	// Pass function to call on keyboard input
	glutKeyboardFunc(handle_input);

	// Pass function to call on rerender
	glutDisplayFunc(render);

	// Pass function to call on window size change
	glutReshapeFunc(ChangeSize);

	init();
	glutMainLoop();

}
