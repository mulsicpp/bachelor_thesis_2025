#include <stdio.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main(void) {

	if (!glfwInit())
	{
		printf("Failed to initialize GLFW\n");
		return -1;
	}


	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(640, 480, "Raytracing App", NULL, NULL);
	if (!window)
	{
		printf("Failed to create window\n");

		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);


	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}