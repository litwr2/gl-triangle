// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

struct State {
	int win_width, win_height;
	int mouse_button;
	State(int ini_width, int ini_height): win_width(ini_width), win_height(ini_height), mouse_button(0) {}
} state(1024, 768);

GLfloat g_vertex_buffer_data[] = { 
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 0.0f,  1.0f, 0.0f,
};

GLfloat g_vertex_buffer_data2[] = { 
	-.5f, -.5f, 0.0f,
	 1.0f, -.5f, 0.0f,
	 0.0f,  .5f, 0.0f,
};

std::queue<void*> trlist;

void EventDispatcher() {
	state.mouse_button = 0;
}

struct Triangle {
	GLfloat *vertices;
	GLuint vertexbuffer;
        GLuint MatrixID;
	int focus, id;
	static int OC;
	float angle = .0f, scale = .5f, speed = .01f, dx = .0f, dy = .0f, x_velocity = 0.02f, y_velocity = 0.01f;
        Triangle(GLfloat *p, GLuint programID ): vertices(p) {
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
		focus = ((id = OC++) == 0);
		MatrixID = glGetUniformLocation(programID, "MVP"); // Get a handle for our "MVP" uniform
		trlist.push(this);
	}
	~Triangle() {
		glDeleteBuffers(1, &vertexbuffer);	// Cleanup VBO and shader
        }
	void RotationLeft(void) {if (speed < .7) speed += 0.01f;}
	void RotationRight(void) {if (scale > 0.02) speed -= 0.01f;}
	void ZoomIn(void) {if (scale < .99) scale += 0.01f;}
	void ZoomOut(void) {if (scale > 0.02) scale -= 0.01f;}
	int CheckFocus(double, double);
        void turn(void);
};

int Triangle::OC = 0;

void Triangle::turn(void) {
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(scale)); //Scale
	Model = Model * glm::translate(glm::mat4(), glm::vec3(dx, dy, 0.0f)); //Translate
	Model = Model * glm::rotate(angle, glm::vec3(0, 1, 0));  //Rotate

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                // attribute. It must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

	glDisableVertexAttribArray(0);

	glm::vec4 r1 = Model * vec4(make_vec3(vertices), 1.0f);
	glm::vec4 r2 = Model * vec4(make_vec3(vertices + 3), 1.0f);
	glm::vec4 r3 = Model * vec4(make_vec3(vertices + 6), 1.0f);

        angle += speed;

	float x1 = (r1[0] + 1)/2*state.win_width, y1 = (1 - r1[1])/2*state.win_height,
		x2 = (r2[0] + 1)/2*state.win_width, y2 = (1 - r2[1])/2*state.win_height,
		x3 = (r3[0] + 1)/2*state.win_width, y3 = (1 - r3[1])/2*state.win_height;

	dx += x_velocity;
	dy += y_velocity;
	if (y3 <= 0) 
		y_velocity = -fabsf(y_velocity)*(.75f + float(rand())/RAND_MAX/2);
	if (y2 >= state.win_height || y1 >= state.win_height)
		y_velocity = fabsf(y_velocity)*(.75f + float(rand())/RAND_MAX/2);
	if (x2 >= state.win_width  || x1 >= state.win_width)
		x_velocity = -fabsf(x_velocity)*(.75f + float(rand())/RAND_MAX/2);
	if (x1 <= 0|| x2 <= 0)
		x_velocity = fabsf(x_velocity)*(.75f + float(rand())/RAND_MAX/2);
	if (fabsf(x_velocity) > 0.1) x_velocity /= 2;
	if (fabsf(y_velocity) > 0.1) y_velocity /= 2;
	if (fabsf(x_velocity) < 0.01) x_velocity *= 2;
	if (fabsf(y_velocity) < 0.01) y_velocity *= 2;
	if (state.mouse_button) {
		double mx, my, a, b;
		glfwGetCursorPos(window, &mx, &my);
		a = ((mx - x1)*(y3 - y1) - (my - y1)*(x3 - x1))/((x2 - x1)*(y3 - y1) - (y2 - y1)*(x3 - x1));
		b = ((mx - x1)*(y2 - y1) - (my - y1)*(x2 - x1))/((x3 - x1)*(y2 - y1) - (y3 - y1)*(x2 - x1));
		if (a >= 0 && b >= 0 && a + b >=0 && a + b <= 1) {
			printf("An object #%d is hit!\n", id);
			focus = 1;
		}
		else {
			printf("a miss\n");
			focus = 0;
		}
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		state.mouse_button = 1;
}
void window_size_callback(GLFWwindow* window, int width, int height) {
	if (state.win_width != width || state.win_height != height)
		glViewport(0, 0, state.win_width = width, state.win_height = height);
}

GLuint SetUpShaders(void) {
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER); 	// Create the shaders
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Set up the Vertex Shader code
	std::string VertexShaderCode = "#version 330 core\n\
layout(location = 0) in vec3 vertexPosition_modelspace;\n\
uniform mat4 MVP;\n\
void main(){gl_Position =  MVP * vec4(vertexPosition_modelspace,1);}";

	// Set up the Fragment Shader code
	std::string FragmentShaderCode = "#version 330 core\n\
out vec3 color;\n\
void main(){color = vec3(1,0,0);}";

	GLint Result = GL_FALSE;
	int InfoLogLength;

	char const *VertexSourcePointer = VertexShaderCode.c_str();	// Compile Vertex Shader
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%d %s\n", InfoLogLength, &VertexShaderErrorMessage[0]);
	}

	char const *FragmentSourcePointer = FragmentShaderCode.c_str();	// Compile Fragment Shader
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%d %s\n", InfoLogLength, &FragmentShaderErrorMessage[0]);
	}

	GLuint ProgramID = glCreateProgram();	// Link the program
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);	// Check the program
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1){
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%d %s\n", InfoLogLength, &ProgramErrorMessage[0]);
	}
	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

int main(void) {
	if (!glfwInit()) {
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	window = glfwCreateWindow(state.win_width, state.win_height, "A running red triangle", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}

        printf("Use arrow keys to zoom in/out or to change the rotation frequency\n");
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  //normal mouse pointer
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);	// Ensure we can capture the escape key being pressed below
	glClearColor(1, 1, 1, 0);	// White background

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint programID = SetUpShaders(); // Create and compile our GLSL program from the shaders

        Triangle triangle(g_vertex_buffer_data, programID);
	Triangle triangle2(g_vertex_buffer_data2, programID);
	Triangle triangle3(g_vertex_buffer_data2, programID);
	
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	do{
		glClear(GL_COLOR_BUFFER_BIT);		// Clear the screen
		glUseProgram(programID);		// Use our shader

		for (int i = 0; i < trlist.size(); i++) {
			Triangle *p = (Triangle*) trlist.front();
			trlist.pop();
			trlist.push(p);
	                p->turn();
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && p->focus) p->ZoomIn();
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && p->focus) p->ZoomOut();
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && p->focus) p->RotationLeft();  //changes rotation speed
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && p->focus) p->RotationRight();
		}

		EventDispatcher();

		glfwSwapBuffers(window);	// Swap buffers
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	glfwTerminate();	// Close OpenGL window and terminate GLFW
	return 0;
}

