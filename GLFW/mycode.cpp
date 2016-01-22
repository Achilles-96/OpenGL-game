#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define GAME_BIRD 0
#define GAME_WOOD_VERTICAL 1
#define GAME_WOOD_HORIZONTAL 1
#define GAME_PIG 2

using namespace std;

class VAO {
	public:
		GLuint VertexArrayID;
		GLuint VertexBuffer;
		GLuint ColorBuffer;

		GLenum PrimitiveMode;
		GLenum FillMode;
		int NumVertices;
		double centerx;
		double centery;
		double radius;
		double mass;
		int type;
		int dead;

		VAO(){
		}
};
//typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
VAO* create3DObject (GLenum primitive_mode, int numVertices, int type, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, double centerx, double centery, double radius, GLenum fill_mode=GL_FILL )
{
	VAO* vao = new  VAO();
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	vao->centerx = centerx;
	vao->centery = centery;
	vao->radius = radius;
	vao->type = type;
	vao->dead = 0;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, double centerx, double centery, double radius, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, 10, vertex_buffer_data, color_buffer_data, centerx, centery, radius, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

int pressed_state = 0, collision_state=0, zoominstate = 0, zoomoutstate = 0, panright = 0, panleft = 0, panup = 0, pandown = 0;
double curx,cury,initx,inity,speedx,speedy,strength=0.5,prevx,prevy,cannonball_size=18,gravity=0.2;
double fireposx=-400,fireposy=100;
double pivotx=10,pivoty=200,angular_v[6],angle[6],woodspx[6];
VAO  *cannonball, *gameFloor, *woodlogs[6], *pigs[5];
float screenleft = -600, screenright = 600, screentop = -300, screenbotton = 300;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				//rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				zoominstate = 0;
				//triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_M:
				zoomoutstate = 0;
				break;
			case GLFW_KEY_A:
				panleft = 0;
				break;
			case GLFW_KEY_D:
				panright = 0;
				break;
			case GLFW_KEY_W:
				panup = 0;
				break;
			case GLFW_KEY_S:
				pandown = 0;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_P:
				zoominstate=1;
				break;
			case GLFW_KEY_M:
				zoomoutstate = 1;
				break;
			case GLFW_KEY_A:
				panleft = 1;
				break;
			case GLFW_KEY_D:
				panright = 1;
				break;
			case GLFW_KEY_W:
				panup = 1;
				break;
			case GLFW_KEY_S:
				pandown = 1;
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	curx = ((screenright - screenleft)/1200.0f)*xpos + screenleft;
	cury = ((screenbotton - screentop)/600.0f)*ypos + screentop;
}

int is_cannon_clicked(double mousex, double mousey) {
	double x2=cannonball->centerx;
	double y2=cannonball->centery;
	double radius = cannonball->radius;
	if(radius>sqrt((x2-mousex)*(x2-mousex)+(y2-mousey)*(y2-mousey)))
		return true;
	else
		return false;
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS){
				//triangle_rot_dir *= -1;
				if(pressed_state==0&&is_cannon_clicked(curx,cury))
				{	
					initx=curx,inity=cury;
					pressed_state=1;
				}
			}
			if(pressed_state==3){
				pressed_state=0;
				angle[0]=0;
				collision_state=0;
				gravity = 0.2;
			}
			if (action == GLFW_RELEASE){
				//triangle_rot_dir *= -1;
				if(pressed_state==1){
					pressed_state=3;

					if(sqrt((curx-initx)*(curx-initx)+(cury-inity)*(cury-inity)) > 30){
						double angle_present = -M_PI+atan2(inity-cury,initx-curx);
						curx = initx + 30*cos(angle_present);
						cury = inity + 30*sin(angle_present);
					}
					speedx=(initx-curx)*strength;
					speedy=(inity-cury)*strength;

				}
				//printf("pressed at %lf %lf and released at %lf %lf",initx,inity,curx,cury);
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				//rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(screenleft, screenright, screenbotton, screentop, 1.0f, 500.0f);
}



void createPig ()
{
	// GL3 accepts only Triangles. Quads are not supported
	double n=30;
	static GLfloat vertex_buffer_data[3][9*30+2*3];
	static GLfloat color_buffer_data[3][9*30+2*3];
	double sizea[3]={18 + 5, 23 +5 , 20 + 5},sizeb[3]={18, 23, 20};
	float angle=0;
	for(int j=0;j<3;j++)
		for(int i=0;i<n;i++){
			vertex_buffer_data[j][9*i] = vertex_buffer_data[j][9*i+1] = vertex_buffer_data[j][9*i+2] = 0;
			vertex_buffer_data[j][9*i+3]=sizea[j]*cos(angle*M_PI/180.0f);
			vertex_buffer_data[j][9*i+4]=sizeb[j]*sin(angle*M_PI/180.0f);
			vertex_buffer_data[j][9*i+5]=0;
			angle += 360.0f/n;
			vertex_buffer_data[j][9*i+6]=sizea[j]*cos(angle*M_PI/180.0f);
			vertex_buffer_data[j][9*i+7]=sizeb[j]*sin(angle*M_PI/180.0f);
			vertex_buffer_data[j][9*i+8]=0;

			color_buffer_data[j][9*i] = 114.0f/255.0f;
			color_buffer_data[j][9*i+1] = 194.0f/255.0f;
			color_buffer_data[j][9*i+2] = 65.0f/255.0f;
			color_buffer_data[j][9*i+3] = 114.0f/255.0f;
			color_buffer_data[j][9*i+4] = 194.0f/255.0f;
			color_buffer_data[j][9*i+5] = 65.0f/255.0f;
			color_buffer_data[j][9*i+6] = 114.0f/255.0f;
			color_buffer_data[j][9*i+7] = 194.0f/255.0f;
			color_buffer_data[j][9*i+8] = 65.0f/255.0f;
		}
	// create3DObject creates and returns a handle to a VAO that can be used later
	pigs[0] = create3DObject(GL_TRIANGLES, n*3 ,GAME_PIG, vertex_buffer_data[0], color_buffer_data[0], 50, 200-sizeb[0], sizea[0], GL_FILL);
	pigs[1] = create3DObject(GL_TRIANGLES, n*3 ,GAME_PIG, vertex_buffer_data[1], color_buffer_data[1], 300, 200-40-sizeb[1], sizea[1], GL_FILL);
	pigs[2] = create3DObject(GL_TRIANGLES, n*3 ,GAME_PIG, vertex_buffer_data[2], color_buffer_data[2], 400, 200-sizeb[2], sizea[2], GL_FILL);

}
// Creates the rectangle object used in this sample code
void createCannonball ()
{
	// GL3 accepts only Triangles. Quads are not supported
	double n=20;
	static GLfloat vertex_buffer_data[9*20 + 2*3];
	static GLfloat color_buffer_data [9*20 + 2*3];
	float angle=0;
	for(int i=0;i<n;i++){
		vertex_buffer_data[9*i] = vertex_buffer_data[9*i+1] = vertex_buffer_data[9*i+2] = 0;
		vertex_buffer_data[9*i+3]=cannonball_size*cos(angle*M_PI/180.0f);
		vertex_buffer_data[9*i+4]=cannonball_size*sin(angle*M_PI/180.0f);
		vertex_buffer_data[9*i+5]=0;
		angle += 360.0f/n;
		vertex_buffer_data[9*i+6]=cannonball_size*cos(angle*M_PI/180.0f);
		vertex_buffer_data[9*i+7]=cannonball_size*sin(angle*M_PI/180.0f);
		vertex_buffer_data[9*i+8]=0;

		color_buffer_data[9*i] = 214.0f/255.0f;
		color_buffer_data[9*i+1] = 1.0f/255.0f;
		color_buffer_data[9*i+2] = 14.0f/255.0f;
		color_buffer_data[9*i+3] = 214.0f/255.0f;
		color_buffer_data[9*i+4] = 1.0f/255.0f;
		color_buffer_data[9*i+5] = 14.0f/255.0f;
		color_buffer_data[9*i+6] = 214.0f/255.0f;
		color_buffer_data[9*i+7] = 1.0f/255.0f;
		color_buffer_data[9*i+8] = 14.0f/255.0f;
	}


	for(int i=9*n;i<9*(n+2);i+=3){
		color_buffer_data[i] =252.0f/255.0f;
		color_buffer_data[i+1] = 187.0f/255.0f;
		color_buffer_data[i+2] = 35.0f/255.0f;
	}

	vertex_buffer_data[9*20] = cannonball_size*cos((360.0f/n) *M_PI/180.0f);
	vertex_buffer_data[9*20+1] = cannonball_size*sin((360.0f/n) *M_PI/180.0f);
	vertex_buffer_data[9*20+2] = 0;
	vertex_buffer_data[9*20+3] = cannonball_size+10;
	vertex_buffer_data[9*20+4] = -2;
	vertex_buffer_data[9*20+5] = 0;
	vertex_buffer_data[9*20+6] = cannonball_size*cos((360.0f/n) *M_PI/180.0f);
	vertex_buffer_data[9*20+7] = -cannonball_size*sin((360.0f/n) *M_PI/180.0f);
	vertex_buffer_data[9*20+8] = 0;
	/*	vertex_buffer_data[9*20+9] = cannonball_size*cos((360.0f/n) *M_PI/180.0f);
		vertex_buffer_data[9*20+10] = cannonball_size*sin((360.0f/n) *M_PI/180.0f);
		vertex_buffer_data[9*20+11] = 0;
		vertex_buffer_data[9*20+12] = cannonball_size;//cos((360.0f/n) *M_PI/180.0f);
		vertex_buffer_data[9*20+13] = 0;//cannonball_size*sin((360.0f/n) *M_PI/180.0f);
		vertex_buffer_data[9*20+14] = 0;
		vertex_buffer_data[9*20+15] = cannonball_size+10;
		vertex_buffer_data[9*20+16] = 0;
		vertex_buffer_data[9*20+17] = 0;
		*/
	// create3DObject creates and returns a handle to a VAO that can be used later
	cannonball = create3DObject(GL_TRIANGLES, n*3 + 2*3, GAME_BIRD, vertex_buffer_data, color_buffer_data, GL_FILL, 0, 0, cannonball_size);
}

void createGameFloor ()
{
	static GLfloat vertex_buffer_data [20*18];

	double base=200;
	for(int i=0;i<20;i++){
		vertex_buffer_data[18*i]=-600,vertex_buffer_data[18*i+1]=base,vertex_buffer_data[18*i+2]=0;
		vertex_buffer_data[18*i+3]=-600,vertex_buffer_data[18*i+4]=base+10,vertex_buffer_data[18*i+5]=0;
		vertex_buffer_data[18*i+6]=600,vertex_buffer_data[18*i+7]=base,vertex_buffer_data[18*i+8]=0;
		vertex_buffer_data[18*i+9]=600,vertex_buffer_data[18*i+10]=base+10,vertex_buffer_data[18*i+11]=0;
		vertex_buffer_data[18*i+12]=-600,vertex_buffer_data[18*i+13]=base+10,vertex_buffer_data[18*i+14]=0;
		vertex_buffer_data[18*i+15]=600,vertex_buffer_data[18*i+16]=base,vertex_buffer_data[18*i+17]=0;
		base+=5;
	}

	static  GLfloat color_buffer_data [20*18]; 
	double gred=133.0/255.0f,ggreen=183.0/255.0f,gblue=52.0/255.0f;
	for(int i=0;i<20;i++){
		color_buffer_data[18*i]=gred,color_buffer_data[18*i+1]=ggreen,color_buffer_data[18*i+2]=gblue;
		color_buffer_data[18*i+3]=gred,color_buffer_data[18*i+4]=ggreen,color_buffer_data[18*i+5]=gblue;
		color_buffer_data[18*i+6]=gred,color_buffer_data[18*i+7]=ggreen,color_buffer_data[18*i+8]=gblue;
		color_buffer_data[18*i+9]=gred,color_buffer_data[18*i+10]=ggreen,color_buffer_data[18*i+11]=gblue;
		color_buffer_data[18*i+12]=gred,color_buffer_data[18*i+13]=ggreen,color_buffer_data[18*i+14]=gblue;
		color_buffer_data[18*i+15]=gred,color_buffer_data[18*i+16]=ggreen,color_buffer_data[18*i+17]=gblue;
		ggreen+=10.0f/255.0f;
		gred+=2.5f/255.0f;
	}
	gameFloor = create3DObject(GL_TRIANGLES, 20*6, GAME_WOOD_HORIZONTAL, vertex_buffer_data, color_buffer_data, GL_FILL, fireposx, fireposy, 25);
}

void createWoodLogs(){
	static const GLfloat vertex_buffer_data [] = {
		-10, 30, 0,
		-10, -30, 0,
		10, 30, 0,

		10, 30, 0,
		-10, -30, 0,
		10, -30, 0
	};

	static const GLfloat color_buffer_data [] = {
		228.0f/255.0f,142.0f/255.0f,57.0f/255.0f,
		228.0f/255.0f,142.0f/255.0f,57.0f/255.0f,
		228.0f/255.0f,142.0f/255.0f,57.0f/255.0f,
		228.0f/255.0f,142.0f/255.0f,57.0f/255.0f,
		228.0f/255.0f,142.0f/255.0f,57.0f/255.0f,
		228.0f/255.0f,142.0f/255.0f,57.0f/255.0f
	};
	woodlogs[0] = create3DObject(GL_TRIANGLES, 6, GAME_WOOD_VERTICAL, vertex_buffer_data, color_buffer_data, GL_FILL, 0, 0, 25);

	static const GLfloat vertex_buffer_data2 [] = {
		-35, 20, 0,
		-35, -20, 0,
		35, 20, 0,

		35, 20, 0,
		-35, -20, 0,
		35, -20, 0
	};
	woodlogs[1] = create3DObject(GL_TRIANGLES, 6, GAME_WOOD_HORIZONTAL, vertex_buffer_data2, color_buffer_data, GL_FILL, 0, 0, 25);

}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
int lineorfill=GL_FILL;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,1), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	//glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	//glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	//glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	//  Matrices.model *= triangleTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	// draw3DObject draws the VAO given to it using current MVP matrix
	//  draw3DObject(triangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	for(int i=0;i<3;i++){
		if(!pigs[i]->dead){
			double x1 = cannonball->centerx,y1 = cannonball->centery, x2 = pigs[i]->centerx, y2 = pigs[i]->centery;
			if(pigs[i]->radius + cannonball->radius > sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))){
				pigs[i]->dead = 1;
				speedx = 0.1*speedx;
				speedy = 0.1*speedy;
			}

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 translatePig = glm::translate(glm::vec3(pigs[i]->centerx,pigs[i]->centery,0));
			Matrices.model *= translatePig;
			MVP = VP  * Matrices.model; 
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(pigs[i]);

		}
	}

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(gameFloor);


	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateWoodlog,rotateWoodlog;
	translateWoodlog = glm::translate(glm::vec3(0,170,0));
	if(collision_state==1){
		translateWoodlog = glm::translate(glm::vec3(10,200,0));
		glm::mat4 translateWoodlog2 = glm::translate(glm::vec3(-10,-30,0));
		rotateWoodlog = glm::rotate((float)(angle[0]*M_PI/180.0f), glm::vec3(0,0,1));
		angle[0] += angular_v[0];
		angular_v[0] += 0.3;
		angle[0] = min(angle[0],90.0);
		Matrices.model *= (translateWoodlog*rotateWoodlog*translateWoodlog2);
		if(angle[0] >= 45)
			pigs[0]->dead = 1;
	}
	else
		Matrices.model *= translateWoodlog;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(woodlogs[0]);

	Matrices.model = glm::mat4(1.0f);
	translateWoodlog = glm::translate(glm::vec3(300,180,0));
	Matrices.model *= translateWoodlog;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(woodlogs[1]);


	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateRectangle;// = glm::translate (glm::vec3(curx, cury, 0));        // glTranslatef
	glm::mat4 rotateRectangle; // rotate about vector (-1,1,1)
	if(pressed_state==0){
		translateRectangle = glm::translate (glm::vec3(fireposx, fireposy, 0));        // glTranslatef
		cannonball->centerx = fireposx;
		cannonball->centery = fireposy;
		cannonball->radius = cannonball_size;
	}
	else if(pressed_state==1){
		if(sqrt((curx-initx)*(curx-initx)+(cury-inity)*(cury-inity)) > 30){
			double angle_present = -M_PI+atan2(inity-cury,initx-curx);
			curx = initx + 30*cos(angle_present);
			cury = inity + 30*sin(angle_present);
		}
		translateRectangle = glm::translate (glm::vec3(curx, cury, 0));        // glTranslatef
		cannonball->centerx = curx;
		cannonball->centery = cury;
		cannonball->radius = cannonball_size;
	}
	else {
		translateRectangle = glm::translate (glm::vec3(initx, inity, 0));        // glTranslatef
		cannonball->centerx = initx;
		cannonball->centery = inity;
		cannonball->radius = cannonball_size;
	}
	if(cannonball->centerx >= -10 - cannonball_size && cannonball->centerx <= -10 + 20 + cannonball_size && cannonball->centery >= 140 - cannonball_size ){
		collision_state=1;
		angle[0] = 0.1;
		angular_v[0] = speedx*0.2;
		if(cannonball->centery < 140 && cannonball->centerx > -10 - cannonball_size/2 )
			speedy = -speedy;
		else
			speedx = -speedx;
	}
	if(cannonball->centerx >= 300 - 35 - cannonball_size && cannonball->centerx <= 300 - 35 + 60 + cannonball_size && cannonball->centery >= 160 - cannonball_size) {
		if(cannonball->centery < 160 && cannonball->centerx > 300 - 35 - cannonball_size/2 )
			speedy = -speedy;
		else 
			speedx = -speedx;
	}
			
	if(pressed_state==3) 
		rotateRectangle = glm::rotate((float)(atan2(-prevy+inity,-prevx+initx)), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	else
		rotateRectangle = glm::rotate((float)(atan2(-cury+inity,-curx+initx)), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)

	//  Matrices.model *= (translateRectangle * rotateRectangle);
	if(pressed_state==3 || pressed_state==1)  Matrices.model *= (translateRectangle * rotateRectangle);
	else  Matrices.model *= (translateRectangle );
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(cannonball);
	// Increment angles
	float increments = 1;

	if(pressed_state==3){
		prevx=initx,prevy=inity;
		initx=initx+speedx,inity=inity+speedy,speedy+=gravity;
		inity=min(200-cannonball_size,inity);
		if(inity==200-cannonball_size)
			speedy=-0.8*speedy,speedx=0.7*speedx;
		if(fabs(speedx)<=0.05&&fabs(speedy)<=1){
				pressed_state=0;
				angle[0]=0;
				collision_state=0;
				gravity = 0.2;
			
		}

	}
	//camera_rotation_angle++; // Simulating camera rotation
	//triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	// rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetCursorPosCallback(window, cursor_position_callback);
	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	// Generate the VAO, VBOs, vertices data & copy into the array buffer
	createCannonball ();
	createGameFloor ();
	createWoodLogs();
	createPig();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor(156.0/255.0f,205.0f/255.0f,237.0f/255.0f,0.0f);// (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1200;
	int height = 600;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		if(panleft == 1 && screenleft >= -600 + 5){
			screenleft -= 5;
			screenright -= 5;
		}
		if(panright == 1 && screenright <= 600 - 5){
			screenleft += 5;
			screenright += 5;
		}
		if(panup == 1 && screentop >= -300 + 5){
			screentop -= 5;
			screenbotton -= 5;
		}
		if(pandown == 1 && screenbotton <= 300 - 5){
			screentop += 5;
			screenbotton += 5;
		}

		if(zoominstate == 1 && screenright-screenleft > 800) {
				screenleft /= 1.02;
				screenright /= 1.02;
				screentop /= 1.02;
				screenbotton /= 1.02;
		}
		if(zoomoutstate == 1 && screenright - screenleft < 1200) {
			if(screenleft >= -600.0f/1.02f)
				screenleft *= 1.02;
			if(screenright <= 600.0f/1.02f)
				screenright *= 1.02;
			if(screentop >= -300.0f/1.02f)
				screentop *= 1.02;
			if(screenbotton <= 300.0f/1.02f)
				screenbotton *= 1.02;
		}
		if(zoominstate == 1 || zoomoutstate == 1 || panleft == 1 || panright == 1 || panup == 1 || pandown == 1)
			reshapeWindow(window, width, height);
		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
