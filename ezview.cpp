

// INCLUDE THESE headers
#define GLES_VERSION 2
#include "GL/glew.h"
#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "Shader.h"
#include "ppmrw_io.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// link these libs
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3dll.lib")

// macros
#define WINDOW_W 800
#define WINDOW_H 600
#define MY_PI 3.14159265359
#define RADIANS(x) ((x) * MY_PI / 180.0)
#define DEF_angleSensitivity    RADIANS(8)
#define DEF_moveSensitivity    RADIANS(1.5)

// global vars about directory and assets
char resourceDirectory[240] = "assets/";
char vertexShaderFilename[240] = "ezview.vert";
char fragmentShaderFilename[240] = "ezview.frag";
char textureFilename[240] = "assets/cool_scene6.ppm";

// Shader variables
Shader *shader;
GLint iLocPosition;
GLint iLocTexCoord; 
GLint iLocMVP;
GLint iLocDecal;

// buffers
GLfloat *drawvertexBuffer, *texcbuffer;

// texture ID
GLuint decal = 0;

// viewport stuff
int windowWidth = -1;
int windowHeight = -1;
double lastX=0, lastY=0;

// matrix
glm::mat4 modelView;
glm::mat4 rotations;
float Shmatrix[16] = { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };

// global vars for model view
float rx = 0, ry = 0;
float fScale = 1.0f;
float ilmx = 0.0f, ilmy = 0.0f;
float SHx = 0.f, SHy = 0.f;

// texure pixels
char *pixels = NULL;

// to control mouse  and keyboard state
bool leftClick = false;
bool rightClick = false;
bool lctrl = false;
bool ShX = false;
bool ShY = false;


// for each vertex, 2 values
typedef struct 
{
    float position[3];
    float color[2];
} Vertex;


// QUAD vertexes
const Vertex Verteces[] = 
{
    { { 1, -1, 0 },{ 0, 0 } },
    { { 1, 1, 0 },{ 1, 0 } },
    { { -1, 1, 0 },{ 1, 1 } },
    { { -1, -1, 0 },{ 0, 1 } }
};

// indices of the QUAD
const GLubyte Indices[] = 
{
    0, 1, 2,
    2, 3, 0
};


// load a texture into GPU memory
GLuint createTexture(int w, int h)
{
	// generating a texture identifier
	GLuint textureID;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, (void *)pixels);

	// linear texture filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// when the texture is outside [0,1], we just repeat the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	return textureID;
}

// load a texture into main memory amd then into GPU memory
GLuint loadTexture(const char *filename, int w, int h)
{
    unsigned char *IR, *IG, *IB;
    if (readPPM(filename, &IR, &IG, &IB, &h, &w) == -1)
    {
		printf("Error loading file %s\n", filename);
		return -1;
    }

    // allocate memory to contain file data
    pixels = (char *)malloc(w*h*3);

	for (int index = 0; index < w*h; index++)
	{
		pixels[index * 3 + 0] = IR[index];
		pixels[index * 3 + 1] = IG[index];
		pixels[index * 3 + 2] = IB[index];
	}

	GLuint ret = createTexture(w, h);
	return ret;
}

// set up opengl stuff (buffers, texture, ...)
bool setupGraphics(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    drawvertexBuffer = (GLfloat *)malloc(1 * 4 * 3 * sizeof(GLfloat));
    texcbuffer = (GLfloat *)malloc(1 * 4 * 2 * sizeof(GLfloat));

    for (int vert = 0; vert < 4; vert++)
    {
        drawvertexBuffer[vert * 3 + 0] = Verteces[vert].position[0];
        drawvertexBuffer[vert * 3 + 1] = Verteces[vert].position[1];
        drawvertexBuffer[vert * 3 + 2] = Verteces[vert].position[2];
        texcbuffer[vert * 2 + 0] = Verteces[vert].color[0];
        texcbuffer[vert * 2 + 1] = Verteces[vert].color[1];
    }

    /* Full paths to the shader and texture files */
	char vertexShaderPath[240];
	strcpy(vertexShaderPath, resourceDirectory);
	strcat(vertexShaderPath, vertexShaderFilename);

	char fragmentShaderPath[240];
	strcpy(fragmentShaderPath, resourceDirectory);
	strcat(fragmentShaderPath, fragmentShaderFilename);
	
	decal = loadTexture(textureFilename, 500, 500);
	if (decal == 0)
	{
		printf("Error loading %s\n", textureFilename);
		return false;
	}
	printf("%s has been loaded\n", textureFilename);

    /* Process shaders. */
    shader = new Shader(vertexShaderPath, fragmentShaderPath);
 
    shader->Use();

	/* Get attribute locations of non-fixed attributes like colour and texture coordinates. */
    iLocPosition = glGetAttribLocation(shader->Program, "position");
    iLocTexCoord = glGetAttribLocation(shader->Program, "texCoord");
	glEnableVertexAttribArray(iLocPosition);
	glEnableVertexAttribArray(iLocTexCoord);
	
	glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, drawvertexBuffer);
	glVertexAttribPointer(iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, texcbuffer);
	
    /* Get uniform locations */
    iLocMVP = glGetUniformLocation(shader->Program, "mvp");
	iLocDecal = glGetUniformLocation(shader->Program, "decal");

    /* Set clear screen color. */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    return true;
}

// draw function
void renderFrame(void)
{
    shader->Use();
	glm::mat4 ortho = glm::ortho(-windowWidth/(float)windowHeight, windowWidth / (float)windowHeight, -1.0f,1.0f);
	glEnableVertexAttribArray(iLocPosition);
	glEnableVertexAttribArray(iLocTexCoord);
    glm::mat4 modelViewPerspective = ortho * modelView;
    glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, glm::value_ptr(modelViewPerspective));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, decal);
	glUniform1i(iLocDecal, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, Indices);
}

// update shear factors on Mt
void makeShM(float *Mt)
{
    if (ShX) 
		Mt[1] = SHx;
    else if(ShY)
		Mt[4] = SHy;
}

// updater the model view with the current status of global vars
void updateModelView()
{
    if (ShX || ShY)
        makeShM(Shmatrix);
    glm::mat4 ShearM(Shmatrix[0], Shmatrix[4], Shmatrix[8], Shmatrix[12], Shmatrix[1], Shmatrix[5], Shmatrix[9], Shmatrix[13], Shmatrix[2], Shmatrix[7], Shmatrix[10], Shmatrix[14], Shmatrix[3], Shmatrix[7], Shmatrix[11], Shmatrix[15]);
    glm::mat4 translate = glm::translate(glm::vec3(ilmx, ilmy, 0.1));
	glm::mat4 rotationX = glm::rotate(glm::mat4(), glm::radians(rx), glm::vec3(0, 0, 1));
	glm::mat4 scale = glm::scale(glm::vec3(fScale, fScale, fScale));
	glm::mat4 rotationY = glm::rotate(glm::mat4(), glm::radians(ry), glm::vec3(0, 0, 1));
    modelView = translate * scale * ShearM * rotationY * rotationX;
}


void onMouseMove(GLFWwindow *window, double x, double y)
{
	double dx = x - lastX;
	double dy = lastY - y;
	lastX = x;
	lastY = y;

	if (leftClick)
	{
        if (lctrl) 
		{
            ilmx += dx * 0.1f*DEF_moveSensitivity;
            ilmy += dy * 0.1f*DEF_moveSensitivity;
        }
        else if (ShX) 
			SHx += dx * 0.1f*DEF_moveSensitivity;
		else if (ShY) 
			SHy += dy * 0.1f*DEF_moveSensitivity;
		else
		{
			ry += (float)dy * ((lastX <= windowWidth / 2) ? -1.f : 1.f)*DEF_angleSensitivity;
			if (ry > 360.0f) ry -= 360.0f;
			if (ry < -360.0f) ry += 360.0f;

			rx += (float)dx * ((lastY <= windowHeight / 2) ? -1.f : 1.f)*DEF_angleSensitivity;
			if (rx > 360.0f) rx -= 360.0f;
			if (rx < -360.0f) rx += 360.0f;
		}
		updateModelView();
	}

    if (rightClick)
    {
        if (dy != 0)
            fScale += dy < 0 ? 0.1f * DEF_angleSensitivity : -0.1f * DEF_angleSensitivity;
        fScale = fScale > 0 ? fScale : 0.1f *DEF_angleSensitivity;
        updateModelView();
    }
}

void onMouseClick(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		leftClick = (action == GLFW_PRESS);
		lastX = x;
		lastY = y;
	}

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        rightClick = (action == GLFW_PRESS);
        lastX = x;
        lastY = y;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_LEFT_CONTROL && (action == GLFW_PRESS)) 
	{
        lctrl = !lctrl;
        ShY = false;
        ShX = false;
    }

    if (key == GLFW_KEY_X && (action == GLFW_PRESS)) 
	{
        ShX = !ShX;
        if(ShY)
            ShY = !ShY;
        lctrl = false;
    }

    if (key == GLFW_KEY_Y && (action == GLFW_PRESS)) 
	{
        ShY = !ShY;
        if (ShX)
            ShX = !ShX;
        lctrl = false;
    }

}

// resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0,0,width, height);
}

GLFWwindow* InitOpenGL()
{
	printf("Starting GLFW context, OpenGLES 2.0\n");
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Eye Fish Viewer", nullptr, nullptr);
	if (window == nullptr)
	{
		printf("OpenGL ES not supported... trying with default opengl API\n");
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Eye Fish Viewer", nullptr, nullptr);
		if (window == nullptr)
		{
			printf("Failed to create GLFW window\n");
			glfwTerminate();
			return NULL;
		}
	}
	int result = glfwGetWindowAttrib(window, GLFW_CLIENT_API);
	glfwMakeContextCurrent(window);
	printf("%s\n", glGetString(GL_VERSION));

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, onMouseClick);
	glfwSetCursorPosCallback(window, onMouseMove);
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		printf("Failed to initialize GLEW\n");
		return NULL;
	}
	printf("%s\n", glGetString(GL_RENDERER));
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	setupGraphics(WINDOW_W, WINDOW_H);
	return window;
}

// destroy call back
void destroy()
{
	glfwTerminate();
	if (pixels)
		free(pixels);
}

// main function
int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Use ezview < file.ppm>\n");
		return 0;
	}

	// first argument should be a ppm file name
	strcpy(textureFilename, argv[1]);
	GLFWwindow* window = InitOpenGL();
	if (!window)
	{
		printf("Error initializing glfw window\n");
		return 1;
	}
	updateModelView();
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderFrame();
		glfwSwapBuffers(window);
	}
	destroy();
    return 0;
}
