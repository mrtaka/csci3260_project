/*
Student Information
Student ID:	1155111158
Student Name:	Chung Yiu Ting Timothy
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "Dependencies/stb_image/stb_image.h"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <random>

#include <vector>
#include <map>

//haha

using namespace std;
using glm::vec3;
using glm::mat4;

GLuint programID;
GLuint vao[100];

bool play_scene = true;

double updateTime;

int waveNum;
bool mouseInScreen = false;
bool holding = false;
bool autoMouse = false;			//P to toggle
bool movementDetected = false;
bool snapCamera = true;			//O to toggle
bool wasdMovements = false;		//Q to toggle
bool paused = false;
double pauseTime = 0;
int button0_state = 0;
int button1_state = 3;
double pauseX = 450;
double pauseY = 450;

float tigerEnergyTime = -0.5f;
bool tigerDash = false;

glm::vec4 skyColor;

double mouseSensitivity = 1.0;
float camX;
float camY;
float camZ;
float sun_distance = 50.0f;
float moon_distance = 50.0f;

float yaw = -90.0f;
float pitch = 17.0f;

float brightness = 0.8f;
float seconds = 0.0f;

float tigerPosX = 0.0f;
float tigerPosY = 0.0f;
float tigerPosZ = 0.0f;
const float initialDir = 226.5f;	//Facing Z direction, later maybe change to something else
float tigerDir;
int tigerHP = 100;
float invincibleTime;

int heartNum;
float heartPosX[1000];
float heartPosY[1000];
float heartPosZ[1000];
bool heartDestroyed[1000];

int theme_ground = 0;
int theme_tiger = 0;

const int grassNum = 15;
float grassPosX[grassNum];
float grassPosY[grassNum];
float grassPosZ[grassNum];
float grassSize[grassNum];

const int bushNum = 4;
float bushPosX[bushNum];
float bushPosY[bushNum];
float bushPosZ[bushNum];

const int mudNum = 2;
float mudPosX[bushNum];
float mudPosY[bushNum];
float mudPosZ[bushNum];
float mudSize[bushNum];

int wolfDeath = 0;
int wolfIndex = 0;

const int spawnQueueMax = 1000;
int spawnQueue[spawnQueueMax];

const int wolfNum = 300;
float wolfPosX[wolfNum];
float wolfPosY[wolfNum];
float wolfPosZ[wolfNum];
int wolfAgressiveness[wolfNum];
bool wolfAlive[wolfNum];
float wolfRot[wolfNum];
float wolfSize[wolfNum];
bool wolfAttacked[wolfNum];
int wolfHP[wolfNum];
double wolfEnergyTime[wolfNum];
glm::vec2 wolfDashVector[wolfNum];
float wolfDashDistance[wolfNum];
float wolfDeathTime[wolfNum];

int wolfSpawned;
int typeSpawned;

// screen setting
const int SCR_WIDTH = 900;
const int SCR_HEIGHT = 900;

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		std::cout << buffer << std::endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID) {
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
	std::ifstream meInput(fileName);
	if (!meInput.good()) {
		std::cout << "File failed to load ... " << fileName << std::endl;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	//adapter[0] = vertexShaderCode;
	std::string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	//adapter[0] = fragmentShaderCode;
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;
	glUseProgram(programID);
}

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

GLuint loadTexture(const char* texturePath)
{
	// tell stb_image.h to flip loaded texture's on the y-axis.
	stbi_set_flip_vertically_on_load(true);
	// load the texture data into "data"
	int Width, Height, BPP;
	unsigned char* data = stbi_load(texturePath, &Width, &Height, &BPP, 0);
	// Please pay attention to the format when sending the data to GPU
	GLenum format = 3;
	switch (BPP) {
	case 1: format = GL_RED; break;
	case 3: format = GL_RGB; break;
	case 4: format = GL_RGBA; break;
	}
	if (!data) {
		std::cout << "Failed to load texture: " << texturePath << std::endl;
		exit(1);
	}

	//GLuint textureID = 0;
	//TODO: Create one OpenGL texture and set the texture parameter 
	GLuint textureID;
	glGenTextures(1, &textureID);
	// "Bind" the newly created texture :
	// to indicate all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, data);
	// OpenGL has now copied the data. Free our own version


	stbi_image_free(data);

	std::cout << "Load " << texturePath << " successfully!" << std::endl;


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

GLuint Texture0;
GLuint Texture1;

GLuint health_barVAO, health_barVBO, health_barEBO;
Model health_barobj;
GLuint health_barTexture0;

GLuint energy_barVAO, energy_barVBO, energy_barEBO;
Model energy_barobj;
GLuint energy_barTexture0;

GLuint bar_backgroundVAO, bar_backgroundVBO, bar_backgroundEBO;
Model bar_backgroundobj;
GLuint bar_backgroundTexture0;

GLuint heartVAO, heartVBO, heartEBO;
Model heartobj;
GLuint heartTexture0;

GLuint buttonVAO, buttonVBO, buttonEBO;
Model buttonobj;
GLuint buttonTexture0, buttonTexture1, buttonTexture2, buttonTexture3, buttonTexture4, buttonTexture5, buttonTexture6, buttonTexture7, buttonTexture8;

GLuint wolfVAO, wolfVBO, wolfEBO;
Model wolfobj;
GLuint wolfTexture0, wolfTexture1;

GLuint mudVAO, mudVBO, mudEBO;
Model mudobj;
GLuint mudTexture0;

GLuint bushVAO, bushVBO, bushEBO;
Model bushobj;
GLuint bushTexture0;

GLuint grassVAO, grassVBO, grassEBO;
Model grassobj;
GLuint grassTexture0;

GLuint moonVAO, moonVBO, moonEBO;
Model moonobj;
GLuint moonTexture0;

GLuint sunVAO, sunVBO, sunEBO;
Model sunobj;
GLuint sunTexture0;

GLuint groundVAO, groundVBO, groundEBO;
Model groundobj;
GLuint groundTexture0, groundTexture1;

GLuint tigerVAO, tigerVBO, tigerEBO;
Model tigerobj;
GLuint tigerTexture0, tigerTexture1;

void object_textured_health_bar() {
	health_barobj = loadOBJ("resources/health_bar/health_bar.obj");
	glGenVertexArrays(1, &health_barVAO);
	glBindVertexArray(health_barVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &health_barVBO);
	glBindBuffer(GL_ARRAY_BUFFER, health_barVBO);
	glBufferData(GL_ARRAY_BUFFER, health_barobj.vertices.size() * sizeof(Vertex), &health_barobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &health_barEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, health_barEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, health_barobj.indices.size() * sizeof(unsigned int), &health_barobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	health_barTexture0 = loadTexture("resources/health_bar/health_bar_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_energy_bar() {
	energy_barobj = loadOBJ("resources/energy_bar/energy_bar.obj");
	glGenVertexArrays(1, &energy_barVAO);
	glBindVertexArray(energy_barVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &energy_barVBO);
	glBindBuffer(GL_ARRAY_BUFFER, energy_barVBO);
	glBufferData(GL_ARRAY_BUFFER, energy_barobj.vertices.size() * sizeof(Vertex), &energy_barobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &energy_barEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, energy_barEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, energy_barobj.indices.size() * sizeof(unsigned int), &energy_barobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	energy_barTexture0 = loadTexture("resources/energy_bar/energy_bar_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_bar_background() {
	bar_backgroundobj = loadOBJ("resources/bar_background/bar_background.obj");
	glGenVertexArrays(1, &bar_backgroundVAO);
	glBindVertexArray(bar_backgroundVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &bar_backgroundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, bar_backgroundVBO);
	glBufferData(GL_ARRAY_BUFFER, bar_backgroundobj.vertices.size() * sizeof(Vertex), &bar_backgroundobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &bar_backgroundEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bar_backgroundEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bar_backgroundobj.indices.size() * sizeof(unsigned int), &bar_backgroundobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	bar_backgroundTexture0 = loadTexture("resources/bar_background/bar_background_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_heart() {
	heartobj = loadOBJ("resources/heart/heart.obj");
	glGenVertexArrays(1, &heartVAO);
	glBindVertexArray(heartVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &heartVBO);
	glBindBuffer(GL_ARRAY_BUFFER, heartVBO);
	glBufferData(GL_ARRAY_BUFFER, heartobj.vertices.size() * sizeof(Vertex), &heartobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &heartEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heartEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, heartobj.indices.size() * sizeof(unsigned int), &heartobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	heartTexture0 = loadTexture("resources/heart/heart_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_button() {
	buttonobj = loadOBJ("resources/button/button.obj");
	glGenVertexArrays(1, &buttonVAO);
	glBindVertexArray(buttonVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &buttonVBO);
	glBindBuffer(GL_ARRAY_BUFFER, buttonVBO);
	glBufferData(GL_ARRAY_BUFFER, buttonobj.vertices.size() * sizeof(Vertex), &buttonobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &buttonEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buttonEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buttonobj.indices.size() * sizeof(unsigned int), &buttonobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	buttonTexture0 = loadTexture("resources/button/button_continue.jpg");
	buttonTexture1 = loadTexture("resources/button/button_continue_hovered.jpg");
	buttonTexture2 = loadTexture("resources/button/button_continue_clicked.jpg");
	buttonTexture3 = loadTexture("resources/button/button_quit.jpg");
	buttonTexture4 = loadTexture("resources/button/button_quit_hovered.jpg");
	buttonTexture5 = loadTexture("resources/button/button_quit_clicked.jpg");
	buttonTexture6 = loadTexture("resources/button/button_restart.jpg");
	buttonTexture7 = loadTexture("resources/button/button_restart_hovered.jpg");
	buttonTexture8 = loadTexture("resources/button/button_restart_clicked.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_wolf() {
	wolfobj = loadOBJ("resources/wolf/wolf.obj");
	glGenVertexArrays(1, &wolfVAO);
	glBindVertexArray(wolfVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &wolfVBO);
	glBindBuffer(GL_ARRAY_BUFFER, wolfVBO);
	glBufferData(GL_ARRAY_BUFFER, wolfobj.vertices.size() * sizeof(Vertex), &wolfobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &wolfEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wolfEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, wolfobj.indices.size() * sizeof(unsigned int), &wolfobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	wolfTexture0 = loadTexture("resources/wolf/wolf_01.jpg");
	wolfTexture1 = loadTexture("resources/wolf/wolf_02.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_mud() {
	mudobj = loadOBJ("resources/mud/mud.obj");
	glGenVertexArrays(1, &mudVAO);
	glBindVertexArray(mudVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &mudVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mudVBO);
	glBufferData(GL_ARRAY_BUFFER, mudobj.vertices.size() * sizeof(Vertex), &mudobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &mudEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mudEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mudobj.indices.size() * sizeof(unsigned int), &mudobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	mudTexture0 = loadTexture("resources/mud/mud_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_bush() {
	bushobj = loadOBJ("resources/bush/bush.obj");
	glGenVertexArrays(1, &bushVAO);
	glBindVertexArray(bushVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &bushVBO);
	glBindBuffer(GL_ARRAY_BUFFER, bushVBO);
	glBufferData(GL_ARRAY_BUFFER, bushobj.vertices.size() * sizeof(Vertex), &bushobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &bushEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bushEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bushobj.indices.size() * sizeof(unsigned int), &bushobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	bushTexture0 = loadTexture("resources/bush/bush_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_grass() {
	grassobj = loadOBJ("resources/grass/grass.obj");
	glGenVertexArrays(1, &grassVAO);
	glBindVertexArray(grassVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &grassVBO);
	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
	glBufferData(GL_ARRAY_BUFFER, grassobj.vertices.size() * sizeof(Vertex), &grassobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &grassEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, grassobj.indices.size() * sizeof(unsigned int), &grassobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	grassTexture0 = loadTexture("resources/grass/grass_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_moon() {
	moonobj = loadOBJ("resources/moon/moon.obj");
	glGenVertexArrays(1, &moonVAO);
	glBindVertexArray(moonVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &moonVBO);
	glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
	glBufferData(GL_ARRAY_BUFFER, moonobj.vertices.size() * sizeof(Vertex), &moonobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &moonEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moonEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, moonobj.indices.size() * sizeof(unsigned int), &moonobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	moonTexture0 = loadTexture("resources/moon/moon_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_sun() {
	sunobj = loadOBJ("resources/sun/sun.obj");
	glGenVertexArrays(1, &sunVAO);
	glBindVertexArray(sunVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &sunVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
	glBufferData(GL_ARRAY_BUFFER, sunobj.vertices.size() * sizeof(Vertex), &sunobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &sunEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sunobj.indices.size() * sizeof(unsigned int), &sunobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	sunTexture0 = loadTexture("resources/sun/sun_01.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_tiger() {
	tigerobj = loadOBJ("resources/tiger/tiger.obj");
	glGenVertexArrays(1, &tigerVAO);
	glBindVertexArray(tigerVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &tigerVBO);
	glBindBuffer(GL_ARRAY_BUFFER, tigerVBO);
	glBufferData(GL_ARRAY_BUFFER, tigerobj.vertices.size() * sizeof(Vertex), &tigerobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &tigerEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tigerEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tigerobj.indices.size() * sizeof(unsigned int), &tigerobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	tigerTexture0 = loadTexture("resources/tiger/tiger_01.jpg");
	tigerTexture1 = loadTexture("resources/tiger/tiger_02.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void object_textured_ground() {
	groundobj = loadOBJ("resources/ground/ground.obj");
	glGenVertexArrays(1, &groundVAO);
	glBindVertexArray(groundVAO);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &groundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, groundobj.vertices.size() * sizeof(Vertex), &groundobj.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &groundEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, groundobj.indices.size() * sizeof(unsigned int), &groundobj.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	groundTexture0 = loadTexture("resources/ground/ground_01.jpg");
	groundTexture1 = loadTexture("resources/ground/ground_02.jpg");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

void sendDataToOpenGL()
{
	//Load textures
	object_textured_ground();
	object_textured_tiger();
	object_textured_sun();
	object_textured_moon();
	object_textured_grass();
	object_textured_bush();
	object_textured_mud();
	object_textured_wolf();
	object_textured_button();
	object_textured_heart();
	object_textured_bar_background();
	object_textured_energy_bar();
	object_textured_health_bar();
}

void create_object(string obj, float x /*offset*/, float y /*offset*/, float z /*offset*/, int index, int aggression, float rotation, float wolf_size) {
	float shinyLevel = 0.15f;
	int realColor = 0;
	glm::vec4 emissionLight(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	unsigned int slot = 0;
	bool anchor = false;
	bool screen_anchor = false;

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));


	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//Camera stuff
	//translate into camera angle
	if (play_scene) {
		projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 180.0f);

		camX = 20.0 * sin(cos(glm::radians(yaw * mouseSensitivity)) * cos(glm::radians(pitch * mouseSensitivity)));
		camY = 20.0 * sin(glm::radians(pitch * mouseSensitivity));
		camZ = -20.0 * cos(glm::radians(pitch * mouseSensitivity)) * sin(glm::radians(yaw * mouseSensitivity));

		if (snapCamera and movementDetected and !autoMouse) {
			camX = -20.0 * cos(glm::radians(tigerDir - 136.5f));
			camZ = 20.0 * sin(glm::radians(tigerDir - 136.5f));
		}
		/*
		const float radius = 10.0f;
		camX = camera_pos_x * sin(glm::radians(mouseY * 0.01f * mouseSensitivity + 90.0f));
		camY = 14.0f + 10.0f * sin(glm::radians(mouseY * 0.01f * mouseSensitivity));
		camZ = camera_pos_z * sin(glm::radians(mouseY * 0.01f * mouseSensitivity + 90.0f));
		*/
		glm::mat4 view;
		viewMatrix = glm::lookAt(glm::vec3(camX + tigerPosX, camY + tigerPosY, camZ + tigerPosZ), glm::vec3(+tigerPosX, +tigerPosY, +tigerPosZ), glm::vec3(0.0, 1.0, 0.0));
	}

	if (obj == "ground") {
		shinyLevel = 0.5f;
		trans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
		trans = glm::scale(trans, glm::vec3(3.0f, 1.0f, 3.0f));

		//Hide ground plane when looking upwards
		if (pitch < 0.0f) {
			trans = glm::scale(trans, glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}
	if (obj == "tiger") {
		trans = glm::translate(glm::mat4(1.0f), glm::vec3(x + 0.8f * cos(glm::radians(tigerDir - 136.5f)), y + 0.2f, z + 0.8f * -sin(glm::radians(tigerDir - 136.5f))));
		trans = glm::scale(trans, glm::vec3(4.0f, 4.0f, 4.0f));
		trans = glm::rotate(trans, glm::radians(tigerDir), glm::vec3(0.0f, 1.0f, 0.0f));

		//	tigerPosX += 0.9f * cos(glm::radians(tigerDir - 136.5f));
		//	tigerPosZ += -0.9f * sin(glm::radians(tigerDir - 136.5f));
	}
	if (obj == "sun") {
		anchor = true;
		sun_distance = 72.0f;
		trans = glm::scale(trans, glm::vec3(0.000175f, 0.000175f, 0.000175f));

		trans = glm::translate(trans, sun_distance * glm::vec3(1.0f, 2.0f, 0.0f));

		glm::mat4 origin_offset = glm::mat4(1.0f);
		origin_offset = glm::translate(origin_offset, sun_distance * glm::vec3(-1.0f, -2.0f, 0.0f));

		glm::mat4 rotation_at_origin = glm::mat4(1.0f);
		rotation_at_origin = glm::rotate(rotation_at_origin, glm::radians(seconds * 15.0f + 180.0f), glm::vec3(-2.0f, 1.0f, 0.0f));

		trans = rotation_at_origin * origin_offset * trans;		//Loop every 24 seconds

		trans = glm::translate(trans, sun_distance * glm::vec3(1.5f, 0.0f, 0.0f));
		//trans = glm::translate(trans, glm::vec3(camX + tigerPosX, camY + tigerPosY, camZ + tigerPosZ));

		float sunBrightness = cos(glm::radians(seconds * 15.0f)) * 8.0f - 0.4f;
		//Dynamic brightness of light throughout the day!
		sunBrightness = 0.28 * glm::clamp(sunBrightness, 0.0f, 1.0f);
		//Dynamic color of light throughout the day!
		emissionLight = glm::vec4(1.0f, glm::clamp(cos(glm::radians(seconds * 15.0f)) * 1.7f + 0.4f, 0.0f, 1.0f), glm::clamp(cos(glm::radians(seconds * 15.0f)) * 0.7f + 0.8f, 0.0f, 1.0f), 1.0f);
		emissionLight = sunBrightness * emissionLight;
	}
	if (obj == "moon") {
		anchor = true;
		moon_distance = 72.0f;
		trans = glm::scale(trans, glm::vec3(0.035f, 0.035f, 0.035f));

		trans = glm::translate(trans, moon_distance * glm::vec3(1.0f, 2.0f, 0.0f));

		glm::mat4 origin_offset = glm::mat4(1.0f);
		origin_offset = glm::translate(origin_offset, moon_distance * glm::vec3(-1.0f, -2.0f, 0.0f));

		glm::mat4 rotation_at_origin = glm::mat4(1.0f);
		rotation_at_origin = glm::rotate(rotation_at_origin, glm::radians(seconds * 15.0f + 30.0f), glm::vec3(-2.0f, 1.0f, 0.0f));

		trans = rotation_at_origin * origin_offset * trans;		//Loop every 24 seconds

		trans = glm::translate(trans, moon_distance * glm::vec3(2.0f, 0.0f, 0.0f));
		//trans = glm::translate(trans, glm::vec3(camX + tigerPosX, camY + tigerPosY, camZ + tigerPosZ));

		float moonBrightness = cos(glm::radians(seconds * 15.0f + 210.0f)) * 3.0f + 0.5f;
		//Dynamic brightness of light throughout the night!
		moonBrightness = 0.27 * glm::clamp(moonBrightness, 0.0f, 1.0f);
		emissionLight = glm::vec4(1.0f, 1.0f, 0.8f, 1.0f);
		emissionLight = moonBrightness * emissionLight;
	}
	if (obj == "grass") {
		trans = glm::translate(trans, glm::vec3(x, -0.75f, z));
		trans = glm::scale(trans, glm::vec3(0.0015f * y, 0.002f, 0.0015f * y));	//y is grassSize here
	}
	if (obj == "grass2") {
		trans = glm::translate(trans, glm::vec3(x, -1.0f, z));
		trans = glm::scale(trans, glm::vec3(0.0025f * y, 0.0015f, 0.0025f * y));	//y is grassSize here
	}
	if (obj == "grass3") {
		trans = glm::translate(trans, glm::vec3(x, -0.75f, z));
		trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::scale(trans, glm::vec3(0.0015f * y, 0.002f, 0.0015f * y));	//y is grassSize here
	}
	if (obj == "grass4") {
		trans = glm::translate(trans, glm::vec3(x, -1.0f, z));
		trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::scale(trans, glm::vec3(0.0025f * y, 0.0015f, 0.0025f * y));	//y is grassSize here
	}

	if (obj == "bush") {
		trans = glm::translate(trans, glm::vec3(x, -1.0f + y, z));
		//trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::scale(trans, glm::vec3(2.0f, 2.0f, 2.0f));
	}
	if (obj == "mud") {
		trans = glm::translate(trans, glm::vec3(x, -0.9f, z));
		//trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::scale(trans, glm::vec3(y, 0.25f, y));	//y is mudSize here
	}
	if (obj == "wolf") {
		trans = glm::translate(trans, glm::vec3(x, y - 1.0f, z));
		if (aggression == -1) {
			trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			trans = glm::translate(trans, glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else
			trans = glm::rotate(trans, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::scale(trans, glm::vec3(wolf_size, wolf_size, wolf_size));	//y is wolfSize here
	}
	if (obj == "heart") {
		trans = glm::translate(trans, glm::vec3(x, y - 0.6f, z));
		trans = glm::rotate(trans, glm::radians(seconds * 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::scale(trans, glm::vec3(0.075f, 0.075f, 0.075f));
	}
	if (obj == "button") {
		screen_anchor = true;
		realColor = 1;
		trans = glm::translate(trans, glm::vec3(x, y, 0.0f));
		trans = glm::scale(trans, glm::vec3(0.08f, 0.08f, 1.0f));
	}
	if (obj == "health_bar") {
		screen_anchor = true;
		realColor = 2;
		trans = glm::translate(trans, glm::vec3(x + rotation * 0.5f - 0.5f, y, 0.0f));
		trans = glm::scale(trans, glm::vec3(rotation, 0.5f, 1.0f));
	}
	if (obj == "energy_bar") {
		screen_anchor = true;
		realColor = 2;
		trans = glm::translate(trans, glm::vec3(x + rotation * 0.5f - 0.5f, y, 0.0f));
		trans = glm::scale(trans, glm::vec3(rotation, 0.5f, 1.0f));
	}
	if (obj == "bar_background") {
		screen_anchor = true;
		realColor = 2;
		trans = glm::translate(trans, glm::vec3(x, y, 0.1f));
		trans = glm::scale(trans, glm::vec3(1.0f, 0.5f, 1.0f));
	}

	GLint modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "model");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &trans[0][0]);

	if (anchor)
		viewMatrix = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	if (screen_anchor) {
		//viewMatrix = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
		viewMatrix = glm::mat4(1.0f);
		projectionMatrix = glm::mat4(1.0f);
	}

	GLuint viewMatrixUniformLocation = glGetUniformLocation(programID, "view");
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);

	GLuint projectionMatrixUniformLocation = glGetUniformLocation(programID, "projection");
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler0");
	glActiveTexture(GL_TEXTURE0 + slot);

	//ppt
	glBindTexture(GL_TEXTURE_2D, Texture0);
	glUniform1i(TextureID, slot);

	//Real Color
	GLint realColorUniformLocation = glGetUniformLocation(programID, "realColor");
	glUniform1i(realColorUniformLocation, realColor);


	//Map Shininess!!!
	GLint shininessUniformLocation = glGetUniformLocation(programID, "shininess");
	glm::vec4 shininess(shinyLevel, shinyLevel, shinyLevel, 1.0f);
	glUniform4fv(shininessUniformLocation, 1, &shininess[0]);

	//Emission Light Calculation
	GLint emissionLightUniformLocation =
		glGetUniformLocation(programID, "emissionLight");
	glUniform4fv(emissionLightUniformLocation, 1, &emissionLight[0]);
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_MULTISAMPLE);
	installShaders();
	void glfwSetTime(double time);

	//TODO: set up the camera parameters

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void create_grass_pile(float x, float y, float z) {
	create_object("grass", x, y, z, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(grassVAO);
	glBindTexture(GL_TEXTURE_2D, grassTexture0);
	glDrawElements(GL_TRIANGLES, grassobj.indices.size(), GL_UNSIGNED_INT, 0);
	create_object("grass2", x, y, z, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(grassVAO);
	glBindTexture(GL_TEXTURE_2D, grassTexture0);
	glDrawElements(GL_TRIANGLES, grassobj.indices.size(), GL_UNSIGNED_INT, 0);
	create_object("grass3", x, y, z, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(grassVAO);
	glBindTexture(GL_TEXTURE_2D, grassTexture0);
	glDrawElements(GL_TRIANGLES, grassobj.indices.size(), GL_UNSIGNED_INT, 0);
	create_object("grass4", x, y, z, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(grassVAO);
	glBindTexture(GL_TEXTURE_2D, grassTexture0);
	glDrawElements(GL_TRIANGLES, grassobj.indices.size(), GL_UNSIGNED_INT, 0);
}

void create_set_of_random_grass(float x, float y, float z, int invert) {
	if (invert == 0)
		for (int i = 0; i < grassNum; i++) {
			create_grass_pile(grassPosX[i] + x, grassSize[i], grassPosZ[i] + z);
		}
	else
		for (int i = 0; i < bushNum; i++) {
			create_object("bush", -bushPosX[i] + x, grassSize[i], -bushPosZ[i] + z, 0, 0, 0.0f, 0.0f);
			glBindVertexArray(bushVAO);
			glBindTexture(GL_TEXTURE_2D, bushTexture0);
			glDrawElements(GL_TRIANGLES, bushobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
}

void create_set_of_random_bushes(float x, float y, float z, int invert) {
	if (invert == 0)
		for (int i = 0; i < bushNum; i++) {
			create_object("bush", bushPosX[i] + x, bushPosY[i] + y, bushPosZ[i] + z, 0, 0, 0.0f, 0.0f);
			glBindVertexArray(bushVAO);
			glBindTexture(GL_TEXTURE_2D, bushTexture0);
			glDrawElements(GL_TRIANGLES, bushobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
	else
		for (int i = 0; i < bushNum; i++) {
			create_object("bush", -bushPosX[i] + x, -bushPosY[i] + y, -bushPosZ[i] + z, 0, 0, 0.0f, 0.0f);
			glBindVertexArray(bushVAO);
			glBindTexture(GL_TEXTURE_2D, bushTexture0);
			glDrawElements(GL_TRIANGLES, bushobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
}

void create_set_of_random_mud(float x, float y, float z, int invert) {
	if (invert == 0)
		for (int i = 0; i < mudNum; i++) {
			create_object("mud", mudPosX[i] + x, mudSize[i], mudPosZ[i] + z, 0, 0, 0.0f, 0.0f);
			glBindVertexArray(mudVAO);
			glBindTexture(GL_TEXTURE_2D, mudTexture0);
			glDrawElements(GL_TRIANGLES, mudobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
	else
		for (int i = 0; i < mudNum; i++) {
			create_object("mud", -mudPosX[i] + x, mudSize[i], -mudPosZ[i] + z, 0, 0, 0.0f, 0.0f);
			glBindVertexArray(mudVAO);
			glBindTexture(GL_TEXTURE_2D, mudTexture0);
			glDrawElements(GL_TRIANGLES, mudobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
}

void paintGL(void)
{
	if (!paused && tigerHP <= 0) {
		paused = true;
		button0_state = 6;
		pauseTime = glfwGetTime();
		GLint darkenSceneUniformLocation =
			glGetUniformLocation(programID, "darken_scene");
		float darken_scene = 0.35f;
		glUniform1f(darkenSceneUniformLocation, darken_scene);
	}
	if (!paused)
		glClearColor(skyColor.x, skyColor.y, skyColor.z, skyColor.w); //specify the background color, this is just an example
	else
		glClearColor(skyColor.x / 4, skyColor.y / 4, skyColor.z / 4, skyColor.w);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (paused)
		glfwSetTime(pauseTime);
	seconds = (float)glfwGetTime();
	waveNum = 0 + (int)(glfwGetTime() / 30.0f);	//1 Wave per 30 Seconds
	//glViewport(0, 0, width(), height());

	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures


	//Creating Pseudo Infinitely Looping Random Positions of Grass Piles, Bushes and Dirt Piles
	int invert = 0;
	for (int i = -4; i < 5; i++) {
		for (int j = -4; j < 5; j++) {
			if ((((int)tigerPosX / 60) + ((int)tigerPosZ / 48) + i + j) % 2 == 0)
				invert = 0;
			else
				invert = 1;
			create_set_of_random_grass(((float)i + ((int)tigerPosX / 60)) * 60.0f, 0.0f, ((float)j + ((int)tigerPosZ / 48)) * 48.0f, invert);
			create_set_of_random_bushes(((float)i + ((int)tigerPosX / 60)) * 60.0f, 0.0f, ((float)j + ((int)tigerPosZ / 48)) * 48.0f, invert);
			create_set_of_random_mud(((float)i + ((int)tigerPosX / 60)) * 60.0f, 0.0f, ((float)j + ((int)tigerPosZ / 48)) * 48.0f, invert);
		}
	}


	//Render Existing Hearts

	for (int i = 0; i < heartNum; i++) {
		//Check if Collecting Hearts
		if (!heartDestroyed[i]) {
			float distance = sqrt(pow((heartPosZ[i] - tigerPosZ), 2) + pow((heartPosX[i] - tigerPosX), 2));
			if (distance < 4.0f) {
				heartDestroyed[i] = true;
				tigerHP += 10;
			}
		}

		if (!heartDestroyed[i]) {
			create_object("heart", heartPosX[i], heartPosY[i], heartPosZ[i], 0, 0, 0.0f, 0.0f);
			glBindVertexArray(heartVAO);
			glBindTexture(GL_TEXTURE_2D, heartTexture0);
			glDrawElements(GL_TRIANGLES, heartobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
	}

	//Spawn Wolves

	if ((wolfSpawned - wolfDeath) < (3 + waveNum) * 7) {
		random_device dev;
		mt19937 rng(dev());
		uniform_int_distribution<int> rndDistance1(75, 85);
		uniform_int_distribution<int> rndDistance2(85, 120);
		uniform_int_distribution<int> rndDistance(120, 180);
		uniform_int_distribution<int> rndRotation(-180, 180);

		float spawnDistanceOffset = (float)rndDistance(rng);

		//First wolf packs should be spawned nearer
		if (wolfSpawned == 0)
			spawnDistanceOffset = (float)rndDistance1(rng);
		else if (wolfSpawned < 6)
			spawnDistanceOffset = (float)rndDistance1(rng);
		else if (wolfSpawned < 13)
			spawnDistanceOffset = (float)rndDistance2(rng);

		float spawnAngleOffset = (float)rndRotation(rng);

		float spawnXOffset = spawnDistanceOffset * cos(glm::radians(spawnAngleOffset));
		float spawnZOffset = spawnDistanceOffset * sin(glm::radians(spawnAngleOffset));

		if (spawnQueue[typeSpawned] == 0) {
			for (int i = wolfSpawned; i < (wolfSpawned + 5); i++) {
				wolfPosX[i] += spawnXOffset;
				wolfPosZ[i] += spawnZOffset;
				wolfAlive[i] = true;
			}

			wolfSpawned += 5;
			typeSpawned++;
		}
		else if (spawnQueue[typeSpawned] == 1) {
			for (int i = wolfSpawned; i < (wolfSpawned + 2); i++) {
				wolfPosX[i] += spawnXOffset;
				wolfPosZ[i] += spawnZOffset;
				wolfAlive[i] = true;
			}

			wolfSpawned += 2;
			typeSpawned++;
		}
	}

	//Repeating Player Movements
	if (!paused) {
		//Tiger Dash Motion

		if ((seconds - tigerEnergyTime) < 0.2) {
			tigerPosX += 2.7f * cos(glm::radians(tigerDir - 136.5f));
			tigerPosY = sin(glm::radians((seconds - tigerEnergyTime) * 900)) * 2.3f;
			tigerPosZ += -2.7f * sin(glm::radians(tigerDir - 136.5f));
		}
		else {
			tigerPosY = 0.0f;
			tigerDash = false;
		}
	}

	//Repeating AI Movements
	if (!paused && (seconds - updateTime) > 0.025f) {
		updateTime = seconds;
		//cout << tigerPosX << " : " << tigerPosZ << " : " << wolfPosX[0] << " : " << wolfPosZ[0] << endl;
		for (int i = 0; i < wolfNum; i++) {
			if (wolfAlive[i]) {
				if (wolfDeathTime[i] != -1.0f && (seconds - wolfDeathTime[i]) > 4.0f) {
					wolfAlive[i] = false;
					wolfDeath++;

					//Drop heart
					random_device dev;
					mt19937 rng(dev());
					uniform_int_distribution<int> rndHeartChance(0, 4);
					int heart_chance = rndHeartChance(rng);
					if (heart_chance == 4) {
						heartPosX[heartNum] = wolfPosX[i];
						heartPosY[heartNum] = 0.0f;
						heartPosZ[heartNum] = wolfPosZ[i];
						heartDestroyed[heartNum] = false;
						heartNum++;
					}
				}

				if (wolfDeathTime[i] == -1.0f) {
					glm::vec2 vector_towards_tiger;
					float distance;
					float angle;
					distance = sqrt(pow((wolfPosZ[i] - tigerPosZ), 2) + pow((wolfPosX[i] - tigerPosX), 2));
					//distance = glm::distance(glm::vec2(wolfPosX[i], wolfPosZ[i]), glm::vec2(tigerPosX, tigerPosZ));
					vector_towards_tiger = glm::normalize(glm::vec2(tigerPosX, tigerPosZ) - glm::vec2(wolfPosX[i], wolfPosZ[i]));
					angle = atan2(-vector_towards_tiger.y, vector_towards_tiger.x) + glm::radians(90.0f);

					//Check if damaging tiger
					if (!wolfAttacked[i] && distance < 3.5f) {
						wolfAttacked[i] = true;
						if ((seconds - invincibleTime) > 0.2f) {
							//Hurt();
							tigerHP -= 5;
							cout << "tigerHP:" << tigerHP << endl;
						}
					}
					//Check if damaged by tiger
					if (tigerDash && distance < 5.0f) {
						wolfHP[i] -= 1;
						if (wolfHP[i] <= 0) {
							wolfDeathTime[i] = seconds;
							wolfAgressiveness[i] = -1;
						}
					}

					//Move back to scene if too far away
					if (distance > 200.0f) {

						wolfPosX[i] += 15.0f * vector_towards_tiger.x;
						wolfPosZ[i] += 15.0f * vector_towards_tiger.y;
					}
					//If close, react according to aggressiveness
					//During Wolf Dash Motion
					else if ((seconds - wolfEnergyTime[i]) < 0.3f) {
						if (wolfAgressiveness[i] == 0) {
							wolfPosX[i] -= 1.0f * (wolfSize[i] / 2.8f) * wolfDashVector[i].x;
							wolfPosY[i] = sin(glm::radians((seconds - wolfEnergyTime[i]) * 600)) * 1.3f;
							wolfPosZ[i] -= 1.0f * wolfDashVector[i].y;
							wolfRot[i] = atan2(-wolfDashVector[i].y, wolfDashVector[i].x) + glm::radians(270.0f);
						}
						if (wolfAgressiveness[i] == 1) {
							wolfPosX[i] += wolfDashDistance[i] * 1.6f * wolfDashVector[i].x;
							wolfPosY[i] = sin(glm::radians((seconds - wolfEnergyTime[i]) * 600)) * 1.3f;
							wolfPosZ[i] += wolfDashDistance[i] * 1.6f * wolfDashVector[i].y;
							wolfRot[i] = atan2(-wolfDashVector[i].y, wolfDashVector[i].x) + glm::radians(90.0f);
						}
						else if (wolfAgressiveness[i] == 2) {
							wolfPosX[i] += wolfDashDistance[i] * 1.6f * wolfDashVector[i].x;
							wolfPosY[i] = sin(glm::radians((seconds - wolfEnergyTime[i]) * 600)) * 1.3f;
							wolfPosZ[i] += wolfDashDistance[i] * 1.6f * wolfDashVector[i].y;
							wolfRot[i] = atan2(-wolfDashVector[i].y, wolfDashVector[i].x) + glm::radians(90.0f);
						}
					}
					else {
						wolfAttacked[i] = true;
						//Start Wolf Dash Motion
						if ((seconds - wolfEnergyTime[i]) > 2.5f && distance < 10.0f) {
							wolfEnergyTime[i] = seconds;
							wolfDashVector[i] = vector_towards_tiger;
							wolfDashDistance[i] = distance / 10.0f;
							if (wolfAgressiveness[i] == 1) {
								wolfAttacked[i] = false;
							}
						}

						//Scared wolf, Run away if detects player
						else if (wolfAgressiveness[i] == 0 && distance < 60.0f) {
							wolfPosX[i] -= 0.4f * (wolfSize[i] / 2.8f) * vector_towards_tiger.x;
							wolfPosZ[i] -= 0.4f * (wolfSize[i] / 2.8f) * vector_towards_tiger.y;
							wolfRot[i] = angle + glm::radians(180.0f);
						}

						//Aggressive wolf, Run close if far but close enough to detect player
						else if ((seconds - wolfEnergyTime[i]) > 1.0f && distance > 10.0f && distance < 70.0f) {
							if (wolfAgressiveness[i] == 1) {
								wolfPosX[i] += 0.58f * (wolfSize[i] / 2.8f) * vector_towards_tiger.x;
								wolfPosZ[i] += 0.58f * (wolfSize[i] / 2.8f) * vector_towards_tiger.y;
								wolfRot[i] = angle;
							}
							else if (wolfAgressiveness[i] == 2) {
								wolfPosX[i] += 0.65f * (wolfSize[i] / 2.8f) * vector_towards_tiger.x;
								wolfPosZ[i] += 0.65f * (wolfSize[i] / 2.8f) * vector_towards_tiger.y;
								wolfRot[i] = angle;
							}
						}

						//Aggressive wolf, backups if way too close to player
						else if ((seconds - wolfEnergyTime[i]) > 0.7f && distance < 8.0f) {
							if (wolfAgressiveness[i] == 1) {
								wolfPosX[i] -= 0.25f * vector_towards_tiger.x;
								wolfPosZ[i] -= 0.25f * vector_towards_tiger.y;
								wolfRot[i] = angle;
							}
							else if (wolfAgressiveness[i] == 2) {
								wolfPosX[i] -= 0.4f * vector_towards_tiger.x;
								wolfPosZ[i] -= 0.4f * vector_towards_tiger.y;
								wolfRot[i] = angle;
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < wolfNum; i++) {
		if (wolfAlive[i]) {
			create_object("wolf", wolfPosX[i], wolfPosY[i], wolfPosZ[i], i, wolfAgressiveness[i], wolfRot[i], wolfSize[i]);
			glBindVertexArray(wolfVAO);
			glBindTexture(GL_TEXTURE_2D, wolfTexture0);
			glDrawElements(GL_TRIANGLES, wolfobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
	}

	create_object("moon", 0.0f, 0.0f, 0.0f, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(moonVAO);
	glBindTexture(GL_TEXTURE_2D, moonTexture0);
	glDrawElements(GL_TRIANGLES, moonobj.indices.size(), GL_UNSIGNED_INT, 0);

	create_object("sun", 0.0f, 0.0f, 0.0f, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(sunVAO);
	glBindTexture(GL_TEXTURE_2D, sunTexture0);
	glDrawElements(GL_TRIANGLES, sunobj.indices.size(), GL_UNSIGNED_INT, 0);


	//Display Health and Energy

	/*
	create_object("bar_background", -0.4f, 0.9f, 0.0f, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(bar_backgroundVAO);
	glBindTexture(GL_TEXTURE_2D, bar_backgroundTexture0);
	glDrawElements(GL_TRIANGLES, bar_backgroundobj.indices.size(), GL_UNSIGNED_INT, 0);
	*/

	create_object("health_bar", -0.4f, 0.9f, 0.0f, 0, 0, (float)(tigerHP) / 100.0f, 0.0f);
	glBindVertexArray(health_barVAO);
	glBindTexture(GL_TEXTURE_2D, health_barTexture0);
	glDrawElements(GL_TRIANGLES, health_barobj.indices.size(), GL_UNSIGNED_INT, 0);

	/*
	create_object("bar_background", -0.4f, 0.8f, 0.0f, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(bar_backgroundVAO);
	glBindTexture(GL_TEXTURE_2D, bar_backgroundTexture0);
	glDrawElements(GL_TRIANGLES, bar_backgroundobj.indices.size(), GL_UNSIGNED_INT, 0);
	*/

	create_object("energy_bar", -0.4f, 0.8f, 0.0f, 0, 0, glm::clamp(seconds - tigerEnergyTime, 0.0f, 1.0f), 0.0f);
	glBindVertexArray(energy_barVAO);
	glBindTexture(GL_TEXTURE_2D, energy_barTexture0);
	glDrawElements(GL_TRIANGLES, energy_barobj.indices.size(), GL_UNSIGNED_INT, 0);

	if (paused) {
		create_object("button", 0.0f, 0.15f, 0.0f, 0, 0, 0.0f, 0.0f);
		glBindVertexArray(buttonVAO);
		if (button0_state == 0)
			glBindTexture(GL_TEXTURE_2D, buttonTexture0);
		else if (button0_state == 1)
			glBindTexture(GL_TEXTURE_2D, buttonTexture1);
		else if (button0_state == 2)
			glBindTexture(GL_TEXTURE_2D, buttonTexture2);
		else if (button0_state == 6)
			glBindTexture(GL_TEXTURE_2D, buttonTexture6);
		else if (button0_state == 7)
			glBindTexture(GL_TEXTURE_2D, buttonTexture7);
		else if (button0_state == 8)
			glBindTexture(GL_TEXTURE_2D, buttonTexture8);



		glDrawElements(GL_TRIANGLES, buttonobj.indices.size(), GL_UNSIGNED_INT, 0);

		create_object("button", 0.0f, -0.15f, 0.0f, 0, 0, 0.0f, 0.0f);
		glBindVertexArray(buttonVAO);
		if (button1_state == 3)
			glBindTexture(GL_TEXTURE_2D, buttonTexture3);
		else if (button1_state == 4)
			glBindTexture(GL_TEXTURE_2D, buttonTexture4);
		else if (button1_state == 5)
			glBindTexture(GL_TEXTURE_2D, buttonTexture5);
		glDrawElements(GL_TRIANGLES, buttonobj.indices.size(), GL_UNSIGNED_INT, 0);
	}

	//Creating Pseudo Infinitely Looping Ground
	for (int i = -4; i < 5; i++) {
		for (int j = -4; j < 5; j++) {
			create_object("ground", (float)i * 60 + ((int)tigerPosX / 60) * 60, 0.0f, (float)j * 48 + ((int)tigerPosZ / 48) * 48, 0, 0, 0.0f, 0.0f);
			glBindVertexArray(groundVAO);
			if (theme_ground == 0)
				glBindTexture(GL_TEXTURE_2D, groundTexture0);
			if (theme_ground == 1)
				glBindTexture(GL_TEXTURE_2D, groundTexture1);
			glDrawElements(GL_TRIANGLES, groundobj.indices.size(), GL_UNSIGNED_INT, 0);
		}
	}

	//1 tile of ground
	/*
	create_object("ground", 0.0f, 0.0f, 0.0f, 0, 0);
	glBindVertexArray(groundVAO);
	if (theme_ground == 0)
		glBindTexture(GL_TEXTURE_2D, groundTexture0);
	if (theme_ground == 1)
		glBindTexture(GL_TEXTURE_2D, groundTexture1);
	glDrawElements(GL_TRIANGLES, groundobj.indices.size(), GL_UNSIGNED_INT, 0);
	*/

	glBindVertexArray(groundVAO);
	if (theme_ground == 0)
		glBindTexture(GL_TEXTURE_2D, groundTexture0);
	if (theme_ground == 1)
		glBindTexture(GL_TEXTURE_2D, groundTexture1);
	glDrawElements(GL_TRIANGLES, groundobj.indices.size(), GL_UNSIGNED_INT, 0);

	/////////////

	create_object("tiger", tigerPosX, tigerPosY, tigerPosZ, 0, 0, 0.0f, 0.0f);
	glBindVertexArray(tigerVAO);
	if (theme_tiger == 0)
		glBindTexture(GL_TEXTURE_2D, tigerTexture0);
	if (theme_tiger == 1)
		glBindTexture(GL_TEXTURE_2D, tigerTexture1);
	glDrawElements(GL_TRIANGLES, tigerobj.indices.size(), GL_UNSIGNED_INT, 0);


	//Lights
	glm::mat4 transLight;

	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	glm::vec4 ambientLight(0.8f * brightness, 0.8f * brightness, 0.9f * brightness, 1.0f);
	glUniform4fv(ambientLightUniformLocation, 1, &ambientLight[0]);

	GLint eyePositionUniformLocation =
		glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(camX + tigerPosX, camY + tigerPosY, camZ + tigerPosZ);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

	//	SUN

	transLight = glm::mat4(1.0f);
	GLint lightPositionUniformLocation =
		glGetUniformLocation(programID, "lightPositionWorld");
	glm::vec4 SunPosition(1.0f, 2.0f, 0.0f, 1.0f);
	transLight = glm::rotate(transLight, glm::radians(seconds * 15.0f), glm::vec3(-2.0f, 1.0f, 0.0f));;		//Loop every 24 seconds
	transLight = glm::translate(transLight, glm::vec3(1.5f, 0.0f, 0.0f));
	//transLight = glm::translate(transLight, glm::vec3(tigerPosX, tigerPosY, tigerPosZ));
	vec3 lightPosition(transLight * SunPosition);
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);

	GLint sunColorUniformLocation =
		glGetUniformLocation(programID, "sunColor");
	float sunBrightness = cos(glm::radians(seconds * 15.0f)) * 3.0f + 1.0f;
	//Dynamic brightness of light throughout the day!
	sunBrightness = 0.85 * glm::clamp(sunBrightness, 0.0f, 1.0f);
	//Dynamic color of light throughout the day!
	glm::vec4 sunColor(1.0f, glm::clamp(cos(glm::radians(seconds * 15.0f)) * 1.2f + 0.32f, 0.0f, 1.0f), glm::clamp(cos(glm::radians(seconds * 15.0f)) * 0.7f + 0.7f, 0.0f, 1.0f), 1.0f);
	sunColor = sunBrightness * sunColor;

	float skyBrightness = cos(glm::radians(seconds * 15.0f)) * 3.0f + 1.0f;
	skyBrightness = glm::clamp(sunBrightness, 0.05f, 1.0f);
	skyColor = glm::vec4(0.85f, glm::clamp(cos(glm::radians(seconds * 15.0f)) * 1.7f + 0.4f, 0.2f, 0.85f), glm::clamp(cos(glm::radians(seconds * 15.0f)) * 0.7f + 0.9f, 0.0f, 1.0f), 1.0f);
	skyColor = skyBrightness * skyColor;
	glUniform4fv(sunColorUniformLocation, 1, &sunColor[0]);

	//	MOON

	transLight = glm::mat4(1.0f);
	GLint lightPositionUniformLocationMoon =
		glGetUniformLocation(programID, "lightPositionWorldMoon");
	glm::vec4 MoonPosition(1.0f, 2.0f, 0.0f, 1.0f);
	transLight = glm::rotate(transLight, glm::radians(seconds * 15.0f + 210.0f), glm::vec3(-2.0f, 1.0f, 0.0f));;		//Loop every 24 seconds
	transLight = glm::translate(transLight, glm::vec3(2.0f, 0.0f, 0.0f));
	//transLight = glm::translate(transLight, glm::vec3(tigerPosX, tigerPosY, tigerPosZ));
	vec3 lightPositionMoon(transLight * MoonPosition);
	glUniform3fv(lightPositionUniformLocationMoon, 1, &lightPositionMoon[0]);

	GLint moonColorUniformLocation =
		glGetUniformLocation(programID, "moonColor");
	float moonBrightness = cos(glm::radians(seconds * 15.0f + 210.0f)) * 3.0f + 1.0f;
	//Dynamic brightness of light throughout the night!
	moonBrightness = 0.25 * glm::clamp(moonBrightness, 0.0f, 1.0f);
	glm::vec4 moonColor(1.0f, 1.0f, 0.8f, 1.0f);
	moonColor = moonBrightness * moonColor;
	glUniform4fv(moonColorUniformLocation, 1, &moonColor[0]);
	glFlush();

	//	SPOTLIGHT
	/*
	transLight = glm::mat4(1.0f);
	GLint lightPositionUniformLocationSpotLight =
		glGetUniformLocation(programID, "lightPositionSpotLight");
	glm::vec4 SpotLightPosition(tigerPosX, tigerPosY + 4.0f, tigerPosZ, 1.0f);
	transLight = glm::translate(transLight, glm::vec3(0.0f, sin(glm::radians(seconds * 180.0f)), 0.0f));
	vec3 lightPositionSpotLight(transLight* SpotLightPosition);
	glUniform3fv(lightPositionUniformLocationSpotLight, 1, &lightPositionSpotLight[0]);

	GLint spotLightColorUniformLocation =
		glGetUniformLocation(programID, "spotLightColor");
	float spotLightBrightness = 0.8f;
	glm::vec4 spotLightColor(0.0f, 0.0f, 1.0f, 1.0f);
	spotLightColor = spotLightBrightness * spotLightColor;
	glUniform4fv(spotLightColorUniformLocation, 1, &spotLightColor[0]);
	*/
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


void initialize_game() {
	glfwSetTime(0.0f);
	updateTime = 0.0f;
	tigerPosX = 0.0f;
	tigerPosY = 0.0f;
	tigerPosZ = 0.0f;
	tigerDir = initialDir;
	tigerEnergyTime = -0.5f;
	tigerDash = false;
	invincibleTime = 5.0f;
	tigerHP = 100;
	heartNum = 0;

	waveNum = 1;
	typeSpawned = 0;
	wolfSpawned = 0;
	wolfDeath = 0;
	wolfIndex = 0;

	random_device dev;
	mt19937 rng(dev());

	uniform_int_distribution<int> rnd1(-50, 50);
	uniform_int_distribution<int> rnd2(-40, 40);
	uniform_int_distribution<int> rnd3(3, 6);
	uniform_int_distribution<int> rnd4(2, 4);
	uniform_int_distribution<int> rndSpawnType(0, 1);
	uniform_int_distribution<int> rndRotation(-180, 180);

	for (int i = 0; i < wolfNum; i++) {
		wolfAlive[i] = false;
		wolfHP[i] = 1;
		wolfEnergyTime[i] = -1.0f;
		wolfDashVector[i] = glm::vec2(0.0f, -1.0f);
		wolfDashDistance[i] = 0.0f;
		wolfAttacked[i] = true;
		wolfDeathTime[i] = -1.0f;
	}

	for (int i = 0; i < spawnQueueMax; i++) {
		spawnQueue[i] = rndSpawnType(rng);
	}

	for (int i = 0; i < spawnQueueMax; i++) {
		if (wolfIndex < wolfNum - 5) {
			if (spawnQueue[i] == 0) {
				wolfPosX[wolfIndex] = 0.0f;
				wolfPosY[wolfIndex] = 0.0f;
				wolfPosZ[wolfIndex] = 0.0f;
				wolfRot[wolfIndex] = (float)rndRotation(rng);
				wolfSize[wolfIndex] = 3.8;
				wolfAgressiveness[wolfIndex] = 1;
				wolfHP[wolfIndex] = 1;

				wolfPosX[wolfIndex + 1] = 2.5f;
				wolfPosY[wolfIndex + 1] = 0.0f;
				wolfPosZ[wolfIndex + 1] = -1.0f;
				wolfRot[wolfIndex + 1] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 1] = 3.5;
				wolfAgressiveness[wolfIndex + 1] = 1;
				wolfHP[wolfIndex + 1] = 1;

				wolfPosX[wolfIndex + 2] = -2.5f;
				wolfPosY[wolfIndex + 2] = 0.0f;
				wolfPosZ[wolfIndex + 2] = -1.0f;
				wolfRot[wolfIndex + 2] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 2] = 2.8;
				wolfAgressiveness[wolfIndex + 2] = 0;
				wolfHP[wolfIndex + 2] = 1;

				wolfPosX[wolfIndex + 3] = 1.0f;
				wolfPosY[wolfIndex + 3] = 0.0f;
				wolfPosZ[wolfIndex + 3] = -2.5f;
				wolfRot[wolfIndex + 3] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 3] = 2.8;
				wolfAgressiveness[wolfIndex + 3] = 0;
				wolfHP[wolfIndex + 3] = 1;

				wolfPosX[wolfIndex + 4] = -1.0f;
				wolfPosY[wolfIndex + 4] = 0.0f;
				wolfPosZ[wolfIndex + 4] = -2.5f;
				wolfRot[wolfIndex + 4] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 4] = 2.8;
				wolfAgressiveness[wolfIndex + 4] = 0;
				wolfHP[wolfIndex + 4] = 1;

				wolfIndex += 5;
			}
			else if (spawnQueue[i] == 1) {
				wolfPosX[wolfIndex] = 0.0f;
				wolfPosY[wolfIndex] = 0.0f;
				wolfPosZ[wolfIndex] = 0.0f;
				wolfRot[wolfIndex] = (float)rndRotation(rng);
				wolfSize[wolfIndex] = 3.8;
				wolfAgressiveness[wolfIndex] = 1;
				wolfHP[wolfIndex] = 1;

				wolfPosX[wolfIndex + 1] = 2.0f;
				wolfPosY[wolfIndex + 1] = 0.0f;
				wolfPosZ[wolfIndex + 1] = 2.5f;
				wolfRot[wolfIndex + 1] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 1] = 2.8;
				wolfAgressiveness[wolfIndex + 1] = 0;
				wolfHP[wolfIndex + 1] = 1;

				wolfIndex += 2;
			}
			else if (spawnQueue[i] == 2) {
			}
		}
	}

	for (int i = 0; i < grassNum; i++) {
		grassPosX[i] = (float)rnd1(rng) * 0.6f;
		grassPosY[i] = 0.0f;
		grassPosZ[i] = (float)rnd2(rng) * 0.6f;
		grassSize[i] = (float)rnd4(rng);
	}
	for (int i = 0; i < bushNum; i++) {
		bushPosX[i] = (float)rnd1(rng) * 0.6f;
		bushPosY[i] = 0.0f;
		bushPosZ[i] = (float)rnd2(rng) * 0.6f;
	}
	for (int i = 0; i < mudNum; i++) {
		mudPosX[i] = (float)rnd1(rng) * 0.6f;
		mudPosY[i] = 0.0f;
		mudPosZ[i] = (float)rnd2(rng) * 0.6f;
		mudSize[i] = (float)rnd3(rng) * 0.1f;
	}

	for (int i = 0; i < wolfNum; i++) {
		wolfAlive[i] = false;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		holding = true;
		movementDetected = false;
		if (button0_state == 1)
			button0_state = 2;
		if (button1_state == 4)
			button1_state = 5;
		if (button0_state == 7)
			button0_state = 8;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		holding = false;
		if (paused && (pauseX > 266 && pauseX < 634) && (pauseY > 339 && pauseY < 426)) {
			paused = !paused;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetCursorPos(window, (double)450, (double)450);
			if (tigerHP <= 0)
				initialize_game();

			GLint darkenSceneUniformLocation =
				glGetUniformLocation(programID, "darken_scene");
			float darken_scene = 1.0f;
			glUniform1f(darkenSceneUniformLocation, darken_scene);
		}
		else if (paused && (pauseX > 246 && pauseX < 654) && (pauseY > 474 && pauseY < 561)) {
			glfwSetWindowShouldClose(window, true);
		}
	}
}

void cursor_enter_callback(GLFWwindow* window, int entered) {
	if (entered == 1)
		mouseInScreen = true;
	else
		mouseInScreen = false;
	//cout << entered << endl;
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	/*
	mouseX += (x - 450);
	mouseY += (y - 450);
	if (mouseY > (9000.0f / mouseSensitivity))
		mouseY = 9000.0f / mouseSensitivity;

	if (mouseY < -1500)
		mouseY = -1500;
	*/
	if (!paused) {
		if (tigerHP > 0)
			button0_state = 0;
		else
			button0_state = 6;
		button1_state = 3;
		if (holding or autoMouse) {
			yaw += 0.03f * (450 - x);
			pitch += 0.03f * (y - 450);
			pitch = glm::clamp(pitch, -89.0f / (float)mouseSensitivity, 89.0f / (float)mouseSensitivity);
		}
		if (play_scene)
			glfwSetCursorPos(window, (double)450, (double)450);
	}
	else {
		pauseX = x;
		pauseY = y;
		if ((x > 266 && x < 634) && (y > 339 && y < 426)) {
			if (tigerHP > 0) {
				if (!holding)
					button0_state = 1;
				else
					button0_state = 2;
				button1_state = 3;
			}
			else {
				if (!holding)
					button0_state = 7;
				else
					button0_state = 8;
				button1_state = 3;
			}
		}
		else if ((x > 266 && x < 634) && (y > 474 && y < 561)) {
			if (!holding)
				button1_state = 4;
			else
				button1_state = 5;
			if (tigerHP > 0)
				button0_state = 0;
			else
				button0_state = 6;
		}
		else {
			if (tigerHP > 0)
				button0_state = 0;
			else
				button0_state = 6;
			button1_state = 3;
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	*/

	// Sets the Keyboard callback for the current window.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		if (tigerHP > 0) {
			paused = !paused;
			if (paused) {
				pauseTime = glfwGetTime();
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

				GLint darkenSceneUniformLocation =
					glGetUniformLocation(programID, "darken_scene");
				float darken_scene = 0.35f;
				glUniform1f(darkenSceneUniformLocation, darken_scene);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				glfwSetCursorPos(window, (double)450, (double)450);

				GLint darkenSceneUniformLocation =
					glGetUniformLocation(programID, "darken_scene");
				float darken_scene = 1.0f;
				glUniform1f(darkenSceneUniformLocation, darken_scene);
			}
		}
	}

	//SETTINGS
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		autoMouse = !autoMouse;
		cout << "autoMouse set to " << autoMouse << endl;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		snapCamera = !snapCamera;
		cout << "snapCamera set to " << snapCamera << endl;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		wasdMovements = !wasdMovements;
		cout << "wasdMovements set to " << wasdMovements << endl;
	}

	if (key == GLFW_KEY_COMMA && action == GLFW_PRESS) {
		if (mouseSensitivity <= 2.0 && mouseSensitivity > 0.1) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity - 0.1);
			pitch = pitch * mouseSensitivity / (mouseSensitivity - 0.1);
			mouseSensitivity -= 0.1;
		}
		if (mouseSensitivity > 2.0) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity - 0.3);
			pitch = pitch * mouseSensitivity / (mouseSensitivity - 0.3);
			mouseSensitivity -= 0.3;
		}
		cout << "Mouse Sensitivity set to: " << mouseSensitivity << endl;
	}
	if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {
		if (mouseSensitivity >= 2.0 && mouseSensitivity < 5.25) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity + 0.3);
			pitch = pitch * mouseSensitivity / (mouseSensitivity + 0.3);
			mouseSensitivity += 0.3;
		}
		if (mouseSensitivity < 2.0) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity + 0.1);
			pitch = pitch * mouseSensitivity / (mouseSensitivity + 0.1);
			mouseSensitivity += 0.1;
		}
		if (mouseSensitivity > 5.3) {
			yaw = yaw * mouseSensitivity / (5.3);
			pitch = pitch * mouseSensitivity / (5.3);
			mouseSensitivity = 5.3;
		}
		cout << "Mouse Sensitivity set to: " << mouseSensitivity << endl;
	}

	if (!paused) {
		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			theme_tiger = 0;
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			theme_tiger = 1;
		}
		if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
			theme_ground = 0;
		}
		if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
			theme_ground = 1;
		}


		if (key == GLFW_KEY_L && action == GLFW_PRESS) {
			random_device dev;
			mt19937 rng(dev());
			uniform_int_distribution<int> rnd_spd(-5, 5);
			uniform_int_distribution<int> rnd_rot(-180, 180);
			tigerDir = rnd_rot(rng);
			tigerPosX += rnd_spd(rng) * 1.3f * cos(glm::radians(tigerDir - 136.5f));
			tigerPosZ += rnd_spd(rng) * 1.3f * sin(glm::radians(tigerDir - 136.5f));
		}


		if (!wasdMovements && key == GLFW_KEY_W && action == GLFW_PRESS) {
			if (brightness < 1.0f)
				brightness += 0.1f;
			if (brightness > 1.0f)
				brightness = 1.0f;
			cout << "Brightness set to: " << brightness << endl;
		}
		if (!wasdMovements && key == GLFW_KEY_S && action == GLFW_PRESS) {
			if (brightness > 0.0f)
				brightness -= 0.1f;
			if (brightness < 0.0f)
				brightness = 0.0f;
			cout << "Brightness set to: " << brightness << endl;
		}

		//	MOVEMENT
		//Tiger Dash Motion
		if (key == GLFW_KEY_SPACE && (seconds - tigerEnergyTime) > 1.0f && action == GLFW_PRESS) {
			tigerEnergyTime = seconds;
			tigerDash = true;
			movementDetected = true;
			invincibleTime = seconds;
		}

		//Tiger Normal Motion
		if (!tigerDash && (key == GLFW_KEY_UP || (wasdMovements && key == GLFW_KEY_W)) && action == GLFW_PRESS) {
			tigerPosX += 0.9f * cos(glm::radians(tigerDir - 136.5f));
			tigerPosZ += -0.9f * sin(glm::radians(tigerDir - 136.5f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_DOWN || (wasdMovements && key == GLFW_KEY_S)) && action == GLFW_PRESS) {
			tigerPosX += -0.5f * cos(glm::radians(tigerDir - 136.5f));
			tigerPosZ += 0.5f * sin(glm::radians(tigerDir - 136.5f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_LEFT || (wasdMovements && key == GLFW_KEY_A)) && action == GLFW_PRESS) {
			tigerDir += 5.0f;
		}
		if (!tigerDash && (key == GLFW_KEY_RIGHT || (wasdMovements && key == GLFW_KEY_D)) && action == GLFW_PRESS) {
			tigerDir -= 5.0f;
		}
		//	REPEAT

		if (!tigerDash && (key == GLFW_KEY_UP || (wasdMovements && key == GLFW_KEY_W)) && action == GLFW_REPEAT) {
			tigerPosX += 1.8f * cos(glm::radians(tigerDir - 136.5f));
			tigerPosZ += -1.8f * sin(glm::radians(tigerDir - 136.5f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_DOWN || (wasdMovements && key == GLFW_KEY_S)) && action == GLFW_REPEAT) {
			tigerPosX += -1.0f * cos(glm::radians(tigerDir - 136.5f));
			tigerPosZ += 1.0f * sin(glm::radians(tigerDir - 136.5f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_LEFT || (wasdMovements && key == GLFW_KEY_A)) && action == GLFW_REPEAT) {
			tigerDir += 5.0f;
		}
		if (!tigerDash && (key == GLFW_KEY_RIGHT || (wasdMovements && key == GLFW_KEY_D)) && action == GLFW_REPEAT) {
			tigerDir -= 5.0f;
		}
	}



	if (key == GLFW_KEY_COMMA && action == GLFW_REPEAT) {
		if (mouseSensitivity <= 2.0 && mouseSensitivity > 0.1) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity - 0.05);
			pitch = pitch * mouseSensitivity / (mouseSensitivity - 0.05);
			mouseSensitivity -= 0.05;
		}
		if (mouseSensitivity > 2.0) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity - 0.1);
			pitch = pitch * mouseSensitivity / (mouseSensitivity - 0.1);
			mouseSensitivity -= 0.1;
		}
		cout << "Mouse Sensitivity set to: " << mouseSensitivity << endl;
	}
	if (key == GLFW_KEY_PERIOD && action == GLFW_REPEAT) {
		if (mouseSensitivity >= 2.0 && mouseSensitivity < 5.25) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity + 0.1);
			pitch = pitch * mouseSensitivity / (mouseSensitivity + 0.1);
			mouseSensitivity += 0.1;
		}
		if (mouseSensitivity < 2.0) {
			yaw = yaw * mouseSensitivity / (mouseSensitivity + 0.05);
			pitch = pitch * mouseSensitivity / (mouseSensitivity + 0.05);
			mouseSensitivity += 0.05;
		}
		if (mouseSensitivity > 5.3) {
			yaw = yaw * mouseSensitivity / (5.3);
			pitch = pitch * mouseSensitivity / (5.3);
			mouseSensitivity = 5.3;
		}
		cout << "Mouse Sensitivity set to: " << mouseSensitivity << endl;
	}


	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		initialize_game();
	}
}

int main(int argc, char* argv[])
{
	GLFWwindow* window;

	initialize_game();

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Fight of the Wild Tiger", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}


	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	//glewExperimental = GL_TRUE;

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);
	//HIDE CURSOR
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	initializedGL();

	GLint darkenSceneUniformLocation =
		glGetUniformLocation(programID, "darken_scene");
	float darken_scene = 1.0f;
	glUniform1f(darkenSceneUniformLocation, darken_scene);

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();
		if (paused)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






