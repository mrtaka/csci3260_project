/*
Student Information
Student ID:	1155111158
Student Name:	Chung Yiu Ting Timothy

Student Information
Student ID: 1155126532
Student Name: Wong Tak Kai
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


//============intiitize setting=================
GLuint programID;

const int num_of_object = 100; //number of type of object
Model obj[num_of_object];
GLuint vao[num_of_object]; //vao for multi-object
GLuint vbo[num_of_object]; //vbo for multi-object
GLuint ebo[num_of_object]; //ebo for multi-object
GLuint Texture[num_of_object][10]; //number of texture

//texture related
GLuint TextureID;
int tiger_texture_id = 0;
int ground_texture_id = 0;

unsigned int slot = 0;
//===========intiitize planet ring================
const int rock_num = 250;
float rock_var[rock_num][6];

void initialize_ring_data(void) {

	random_device dev;
	mt19937 rng(dev());
	uniform_int_distribution<int> rndRotation(-180, 180);
	uniform_int_distribution<int> rndDis(-1, 1);
	uniform_int_distribution<int> rndSize(1, 5);

	for (int i = 0; i < rock_num; i++) {
		rock_var[i][0] = (float)rndRotation(rng); //set angle offset
		rock_var[i][1] = (float)rndDis(rng); //set x offset
		rock_var[i][2] = (float)rndDis(rng); //set y offset
		rock_var[i][3] = (float)rndDis(rng); //set z offset
		rock_var[i][4] = (float)rndRotation(rng); //set rotation
		rock_var[i][5] = (float)rndSize(rng); //set size
	}
}

//================================================

//others variables by timothy
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

float spacecraftPosX = 0.0f;
float spacecraftPosY = 0.0f;
float spacecraftPosZ = 0.0f;
const float initialDir = 0.0f;	//Facing Z direction, later maybe change to something else
float spacecraftDir;
int tigerHP = 100;
float invincibleTime;

int heartNum;
float heartPosX[1000];
float heartPosY[1000];
float heartPosZ[1000];
bool heartDestroyed[1000];

int theme_ground = 0;
int theme_spacecraft = 0;

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

	glDeleteShader(vertexShaderID); //added
	glDeleteShader(fragmentShaderID); //added

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

	//TODO: Create one OpenGL texture and set the texture parameter 
	GLuint textureID;
	glGenTextures(1, &textureID);
	// "Bind" the newly created texture :
	// to indicate all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	// OpenGL has now copied the data. Free our own version
	stbi_image_free(data);

	std::cout << "Load " << texturePath << " successfully!" << std::endl;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

GLuint Texture0;

//===================  cunstom object load function: taka ===================================================
void object_load(int Object_ID, const char* objPath, const char* texturePath, const char* texturePath2, const char* texturePath3) {

	obj[Object_ID] = loadOBJ(objPath);

	if (texturePath == "-1") {
		Texture[Object_ID][0] = loadTexture("resources/button/button_continue.jpg");
		Texture[Object_ID][1] = loadTexture("resources/button/button_continue_hovered.jpg");
		Texture[Object_ID][2] = loadTexture("resources/button/button_continue_clicked.jpg");
		Texture[Object_ID][3] = loadTexture("resources/button/button_quit.jpg");
		Texture[Object_ID][4] = loadTexture("resources/button/button_quit_hovered.jpg");
		Texture[Object_ID][5] = loadTexture("resources/button/button_quit_clicked.jpg");
		Texture[Object_ID][6] = loadTexture("resources/button/button_restart.jpg");
		Texture[Object_ID][7] = loadTexture("resources/button/button_restart_hovered.jpg");
		Texture[Object_ID][8] = loadTexture("resources/button/button_restart_clicked.jpg");
	}
	else {
		Texture[Object_ID][0] = loadTexture(texturePath);
		if (texturePath2 != "0") {
			Texture[Object_ID][1] = loadTexture(texturePath2);
			if (texturePath3 != "0") {
				Texture[Object_ID][2] = loadTexture(texturePath3);
			}
		}
	}

	glGenVertexArrays(1, &vao[Object_ID]);
	glBindVertexArray(vao[Object_ID]);

	glGenBuffers(1, &vbo[Object_ID]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[Object_ID]);
	glBufferData(GL_ARRAY_BUFFER, obj[Object_ID].vertices.size() * sizeof(Vertex), &obj[Object_ID].vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ebo[Object_ID]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[Object_ID]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[Object_ID].indices.size() * sizeof(unsigned int), &obj[Object_ID].indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[Object_ID]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	//Load textures
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

void sendDataToOpenGL()
{
	//Load object and it's textures
	object_load(10, "resources/ground/ground.obj", "resources/ground/ground_01.jpg", "resources/ground/ground_02.jpg", "0"); //ground
	object_load(11, "resources/spacecraft/spacecraft.obj", "resources/spacecraft/spacecraft_01.jpg", "resources/spacecraft/spacecraft_02.jpg", "0"); //spacecraft
	object_load(12, "resources/star/star.obj", "resources/star/star.jpg", "0", "0"); //star
	object_load(13, "resources/health_bar/health_bar.obj", "resources/health_bar/health_bar_01.jpg", "0", "0"); //healthbar
	object_load(14, "resources/energy_bar/energy_bar.obj", "resources/energy_bar/energy_bar_01.jpg", "0", "0"); //energy_bar
	object_load(15, "resources/heart/heart.obj", "resources/heart/heart_01.jpg", "0", "0"); //heart
	object_load(16, "resources/ufo/ufo.obj", "resources/ufo/ufo_01.jpg", "resources/ufo/ufo_02.jpg", "0"); //ufo
	object_load(17, "resources/button/button.obj", "-1", "0", "0"); //button (speical, have 9 different texture in total)
	object_load(18, "resources/planet/planet.obj", "resources/planet/planet_01.jpg", "0", "0"); //planet
	object_load(19, "resources/rock/rock.obj", "resources/rock/rock_01.jpg", "0", "0"); //rock
}

void create_object(int objID, int textureID, float x /*offset*/, float y /*offset*/, float z /*offset*/, float rotation, float obj_size, int index, int aggression) {
	
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
			camX = -20.0 * cos(glm::radians(spacecraftDir + 90.0f));
			camZ = 20.0 * sin(glm::radians(spacecraftDir + 90.0f));
		}
		/*
		const float radius = 10.0f;
		camX = camera_pos_x * sin(glm::radians(mouseY * 0.01f * mouseSensitivity + 90.0f));
		camY = 14.0f + 10.0f * sin(glm::radians(mouseY * 0.01f * mouseSensitivity));
		camZ = camera_pos_z * sin(glm::radians(mouseY * 0.01f * mouseSensitivity + 90.0f));
		*/
		glm::mat4 view;
		viewMatrix = glm::lookAt(glm::vec3(camX + spacecraftPosX, camY + spacecraftPosY, camZ + spacecraftPosZ), glm::vec3(+spacecraftPosX, +spacecraftPosY, +spacecraftPosZ), glm::vec3(0.0, 1.0, 0.0));
	}

	if (objID == 10) {	//ground
		shinyLevel = 0.5f;
	}
	else if (objID == 16) {//ufo
		trans = glm::translate(trans, glm::vec3(x, y - 1.0f, z));
		if (aggression == -1) {
			trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			trans = glm::translate(trans, glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else
			trans = glm::rotate(trans, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::scale(trans, glm::vec3(obj_size, obj_size, obj_size));	//y is wolfSize here
	}
	else if (objID == 17) {	//button
		screen_anchor = true;
		realColor = 1;
		trans = glm::translate(trans, glm::vec3(x, y, 0.0f));
		trans = glm::scale(trans, glm::vec3(0.08f, 0.08f, 1.0f));
	}
	else if (objID == 13 || objID == 14) {	//health_bar or energy_bar
		screen_anchor = true;
		realColor = 2;
		trans = glm::translate(trans, glm::vec3(x + rotation * 0.5f - 0.5f, y, 0.0f));
		trans = glm::scale(trans, glm::vec3(rotation, 0.5f, 1.0f));
	}
	else if (objID == 19) {	//health_bar or energy_bar
		trans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
		trans = glm::scale(trans, glm::vec3(obj_size, obj_size, obj_size));
		trans = glm::rotate(trans, glm::radians(rotation), glm::vec3(0.5f, 0.5f, 0.5f));
	}
	else { //general object
		trans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
		trans = glm::scale(trans, glm::vec3(obj_size, obj_size, obj_size));
		trans = glm::rotate(trans, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
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

	//==================== At the end render the object =========================
	glBindVertexArray(vao[objID]);
	glBindTexture(GL_TEXTURE_2D, Texture[objID][textureID]);
	glDrawElements(GL_TRIANGLES, obj[objID].indices.size(), GL_UNSIGNED_INT, 0);
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

	glClearColor(0.8, 0.8, 0.95, 1);
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
			if ((((int)spacecraftPosX / 60) + ((int)spacecraftPosZ / 48) + i + j) % 2 == 0)
				invert = 0;
			else
				invert = 1;
		}
	}

	//Render Existing Hearts
	for (int i = 0; i < heartNum; i++) {
		//Check if Collecting Hearts
		if (!heartDestroyed[i]) {
			float distance = sqrt(pow((heartPosZ[i] - spacecraftPosZ), 2) + pow((heartPosX[i] - spacecraftPosX), 2));
			if (distance < 4.0f) {
				heartDestroyed[i] = true;
				tigerHP += 10;
			}
		}

		if (!heartDestroyed[i]) {
			create_object(15, 0, heartPosX[i], heartPosY[i]-0.6f, heartPosZ[i], seconds * 90.0f, 0.075f, 0, 0);	//heart
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
			spacecraftPosX += 2.7f * cos(glm::radians(spacecraftDir + 90.0f));
			spacecraftPosY = sin(glm::radians((seconds - tigerEnergyTime) * 900)) * 2.3f;
			spacecraftPosZ += -2.7f * sin(glm::radians(spacecraftDir + 90.0f));
		}
		else {
			spacecraftPosY = 0.0f;
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
					distance = sqrt(pow((wolfPosZ[i] - spacecraftPosZ), 2) + pow((wolfPosX[i] - spacecraftPosX), 2));
					//distance = glm::distance(glm::vec2(wolfPosX[i], wolfPosZ[i]), glm::vec2(tigerPosX, tigerPosZ));
					vector_towards_tiger = glm::normalize(glm::vec2(spacecraftPosX, spacecraftPosZ) - glm::vec2(wolfPosX[i], wolfPosZ[i]));
					angle = atan2(-vector_towards_tiger.y, vector_towards_tiger.x) + glm::radians(90.0f);

					//Check if damaging tiger
					if (!wolfAttacked[i] && distance < 3.5f) {
						wolfAttacked[i] = true;
						if ((seconds - invincibleTime) > 0.2f) {
							//Hurt();
							tigerHP -= 5;
							cout << "spcaeshipHP:" << tigerHP << endl;
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
			if(wolfAgressiveness[i] == -1)
				create_object(16, 1, wolfPosX[i], wolfPosY[i], wolfPosZ[i], wolfRot[i] , wolfSize[i], i, wolfAgressiveness[i]);//destroy ufo
			else
				create_object(16, 0, wolfPosX[i], wolfPosY[i], wolfPosZ[i], wolfRot[i] + seconds * 5.0f, wolfSize[i], i, wolfAgressiveness[i]);//moving ufo
		}
	}

	//=============================generate 2D and 3D object=====================================

	//Display Health and Energy bar
	create_object(13, 0, -0.4f, 0.9f, 0.0f, (float)(tigerHP) / 100.0f, 0.0f, 0, 0); //health_bar
	create_object(14, 0, -0.4f, 0.8f, 0.0f, glm::clamp(seconds - tigerEnergyTime, 0.0f, 1.0f), 0.0f, 0, 0); //energy_bar

	//Display paused menu
	if (paused) {
		create_object(17, button0_state, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f, 0, 0); //button_continue(0)/_clicked(1)/_hovered(2) , button_restart(6)/_clicked(7)/_hovered(8)
		create_object(17, button1_state, 0.0f, -0.15f, 0.0f, 0.0f, 0.0f, 0, 0); //button_quit(3)/_clicked(4)/_hovered(5)
	}

	//normal 3D object
	create_object(11, theme_spacecraft, spacecraftPosX + 0.8f * cos(glm::radians(spacecraftDir)), spacecraftPosY, spacecraftPosZ + 0.8f * -sin(glm::radians(spacecraftDir)), spacecraftDir + 180.0f, 0.01f, 0, 0); //spacecraft
	create_object(12, 0, 0, 0, 0, 0.0f, 0.1f, 0, 0); //star
	create_object(12, 0, -1, 0, 0, 0.0f, 0.1f, 0, 0); //star
	create_object(12, 0, -5, 0, 0, 0.0f, 0.1f, 0, 0); //star
	create_object(10, 0, 0, 0, 0, 0.0f, 1.0f, 0, 0); //ground
	create_object(16, 0, -7, 0, 0, 0.0f, 0.1f, 0, 0);//ufo

	float planet_posz = -150;
	float ring_radius = 50;

	create_object(18, 0, 0, 0, planet_posz, 2.5f * seconds, 10.0f, 0, 0);//planet
	for (int i = 0; i < rock_num; i++) {
		create_object(19, 0, rock_var[i][1] + ring_radius * (sin(0.1f * (seconds + rock_var[i][0]))), rock_var[i][2] +5.0f , planet_posz + rock_var[i][3] + ring_radius * (cos(0.1f*(seconds + rock_var[i][0]))), rock_var[i][4] * 0.5f * seconds, rock_var[i][5] * 0.2f, 0, 0);//rock
	}

	//===================================Lights=================================================
	glm::mat4 transLight;

	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	glm::vec4 ambientLight(0.8f * brightness, 0.8f * brightness, 0.9f * brightness, 1.0f);
	glUniform4fv(ambientLightUniformLocation, 1, &ambientLight[0]);

	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(camX + spacecraftPosX, camY + spacecraftPosY, camZ + spacecraftPosZ);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


void initialize_game() {
	glfwSetTime(0.0f);
	updateTime = 0.0f;
	spacecraftPosX = 0.0f;
	spacecraftPosY = 0.0f;
	spacecraftPosZ = 0.0f;
	spacecraftDir = initialDir;
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
				wolfSize[wolfIndex] = 0.38;
				wolfAgressiveness[wolfIndex] = 1;
				wolfHP[wolfIndex] = 1;

				wolfPosX[wolfIndex + 1] = 2.5f;
				wolfPosY[wolfIndex + 1] = 0.0f;
				wolfPosZ[wolfIndex + 1] = -1.0f;
				wolfRot[wolfIndex + 1] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 1] = 0.35;
				wolfAgressiveness[wolfIndex + 1] = 1;
				wolfHP[wolfIndex + 1] = 1;

				wolfPosX[wolfIndex + 2] = -2.5f;
				wolfPosY[wolfIndex + 2] = 0.0f;
				wolfPosZ[wolfIndex + 2] = -1.0f;
				wolfRot[wolfIndex + 2] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 2] = 0.28;
				wolfAgressiveness[wolfIndex + 2] = 0;
				wolfHP[wolfIndex + 2] = 1;

				wolfPosX[wolfIndex + 3] = 1.0f;
				wolfPosY[wolfIndex + 3] = 0.0f;
				wolfPosZ[wolfIndex + 3] = -2.5f;
				wolfRot[wolfIndex + 3] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 3] = 0.28;
				wolfAgressiveness[wolfIndex + 3] = 0;
				wolfHP[wolfIndex + 3] = 1;

				wolfPosX[wolfIndex + 4] = -1.0f;
				wolfPosY[wolfIndex + 4] = 0.0f;
				wolfPosZ[wolfIndex + 4] = -2.5f;
				wolfRot[wolfIndex + 4] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 4] = 0.28;
				wolfAgressiveness[wolfIndex + 4] = 0;
				wolfHP[wolfIndex + 4] = 1;

				wolfIndex += 5;
			}
			else if (spawnQueue[i] == 1) {
				wolfPosX[wolfIndex] = 0.0f;
				wolfPosY[wolfIndex] = 0.0f;
				wolfPosZ[wolfIndex] = 0.0f;
				wolfRot[wolfIndex] = (float)rndRotation(rng);
				wolfSize[wolfIndex] = 0.38;
				wolfAgressiveness[wolfIndex] = 1;
				wolfHP[wolfIndex] = 1;

				wolfPosX[wolfIndex + 1] = 2.0f;
				wolfPosY[wolfIndex + 1] = 0.0f;
				wolfPosZ[wolfIndex + 1] = 2.5f;
				wolfRot[wolfIndex + 1] = (float)rndRotation(rng);
				wolfSize[wolfIndex + 1] = 0.28;
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
			theme_spacecraft = 0;
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			theme_spacecraft = 1;
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
			spacecraftDir = rnd_rot(rng);
			spacecraftPosX += rnd_spd(rng) * 1.3f * cos(glm::radians(spacecraftDir + 90.0f));
			spacecraftPosZ += rnd_spd(rng) * 1.3f * sin(glm::radians(spacecraftDir + 90.0f));
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
			spacecraftPosX += 0.9f * cos(glm::radians(spacecraftDir + 90.0f));
			spacecraftPosZ += -0.9f * sin(glm::radians(spacecraftDir + 90.0f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_DOWN || (wasdMovements && key == GLFW_KEY_S)) && action == GLFW_PRESS) {
			spacecraftPosX += -0.5f * cos(glm::radians(spacecraftDir + 90.0f));
			spacecraftPosZ += 0.5f * sin(glm::radians(spacecraftDir + 90.0f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_LEFT || (wasdMovements && key == GLFW_KEY_A)) && action == GLFW_PRESS) {
			spacecraftDir += 5.0f;
		}
		if (!tigerDash && (key == GLFW_KEY_RIGHT || (wasdMovements && key == GLFW_KEY_D)) && action == GLFW_PRESS) {
			spacecraftDir -= 5.0f;
		}
		//	REPEAT

		if (!tigerDash && (key == GLFW_KEY_UP || (wasdMovements && key == GLFW_KEY_W)) && action == GLFW_REPEAT) {
			spacecraftPosX += 1.8f * cos(glm::radians(spacecraftDir + 90.0f));
			spacecraftPosZ += -1.8f * sin(glm::radians(spacecraftDir + 90.0f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_DOWN || (wasdMovements && key == GLFW_KEY_S)) && action == GLFW_REPEAT) {
			spacecraftPosX += -1.0f * cos(glm::radians(spacecraftDir + 90.0f));
			spacecraftPosZ += 1.0f * sin(glm::radians(spacecraftDir + 90.0f));
			movementDetected = true;
		}
		if (!tigerDash && (key == GLFW_KEY_LEFT || (wasdMovements && key == GLFW_KEY_A)) && action == GLFW_REPEAT) {
			spacecraftDir += 5.0f;
		}
		if (!tigerDash && (key == GLFW_KEY_RIGHT || (wasdMovements && key == GLFW_KEY_D)) && action == GLFW_REPEAT) {
			spacecraftDir -= 5.0f;
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

	/* Initialize cube data*/
	initialize_ring_data();

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SpaceAngel v0.1", NULL, NULL);
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