#define GLEW_STATIC
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <string> 
#include <fstream>
#include <sstream>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/euler_angles.hpp>
#include <gtx/transform.hpp>
#include <gtc/quaternion.hpp> 
#include <gtx/quaternion.hpp>

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "assimp.lib")

#include "SOIL.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flag

#include "Skeleton.h"


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

GLuint shaderProgramID;

glm::mat4 identityMatrix = glm::mat4(1.0);

float zoom = 2.0f;
glm::mat4 perspectiveMat = 
		glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, zoom
		);

glm::mat4 CameraMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
glm::mat4 ModelMatrix;
glm::mat4 MVPmatrix;

glm::mat4 RotationMatrix;
glm::mat4 RotationX;
glm::mat4 RotationY;
glm::mat4 RotationZ;

glm::mat4 TranslationMatrix =
		glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);


double alpha = 0;
double beta = 0;
double gamma = 0;
glm::quat quaternion;

float *vertexArray;
float *normalArray;
float *uvArray;
float *tangentArray;

float *vertexArrayPlane;
float *vertexArrayRabbit;

float *vertexArrayCube;

int numVerts;
int numVertsPlane;
int numVertsRabbit;

Assimp::Importer importer;
GLuint ModelVBO;

GLuint planeVBO;
GLuint rabbitVBO;

GLfloat* g_vp = NULL; // array of vertex points
GLfloat* g_vn = NULL; // array of vertex normals
GLfloat* g_vt = NULL; // array of texture coordinates
GLfloat* g_vtans = NULL;
int g_point_count = 0;

GLuint tex_diffuse;
GLuint tex_normal;
GLuint tex_depth;
GLuint background;

void UpdateTranslationMatrix(float X, float Y, float Z) {

	TranslationMatrix =
		glm::mat4(
			1.0f, 0.0f, 0.0f, 0,
			0.0f, 1.0f, 0.0f, 0,
			0.0f, 0.0f, 1.0f, 0,
			X, Y, Z, 1.0f
		);

}

Skeleton createSkeleton(Bone *root) {

	Skeleton skeleton = Skeleton();
	skeleton = addBone(skeleton, root);
	return skeleton;

}

bool loadModel(const char* path)
{

	DWORD dwFlags =
		aiProcess_Triangulate | // triangulate n-polygons
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_SortByPType |
		aiProcess_FindDegenerates |
		aiProcess_FindInvalidData |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_GenUVCoords |
		aiProcess_FixInfacingNormals |
		aiProcess_FlipUVs |
		aiProcess_TransformUVCoords;
	
	const aiScene* scene = importer.ReadFile(path, dwFlags);

	if (!scene)
	{
		printf("3dModel loading error: '%s'\n", importer.GetErrorString());
		return false;
	}

	aiMesh *mesh = scene->mMeshes[0];

	GLfloat* g_vtans = NULL;
	if (mesh->HasTangentsAndBitangents()) {
		printf("mesh has tangents and bitangents\n");
		g_vtans = (GLfloat*)malloc(mesh->mNumFaces * 3 * 3);
	}

	numVerts = mesh->mNumFaces * 3;


	vertexArray = new float[mesh->mNumFaces * 3 * 3];
	normalArray = new float[mesh->mNumFaces * 3 * 3];
	uvArray = new float[mesh->mNumFaces * 3 * 2];

	tangentArray = new float[mesh->mNumFaces * 3 * 3];

	for (unsigned int i = 0; i<mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];

		for (int j = 0; j<3; j++)
		{
			aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[j]];
			memcpy(uvArray, &uv, sizeof(float) * 2);
			uvArray += 2;

			aiVector3D normal = mesh->mNormals[face.mIndices[j]];
			memcpy(normalArray, &normal, sizeof(float) * 3);
			normalArray += 3;

			aiVector3D pos = mesh->mVertices[face.mIndices[j]];
			memcpy(vertexArray, &pos, sizeof(float) * 3);
			vertexArray += 3;

			aiVector3D tangent = mesh->mTangents[face.mIndices[j]];
			memcpy(tangentArray, &tangent, sizeof(float) * 3);
			tangentArray += 3;

			if (mesh->HasTextureCoords(0)) {
				//printf_s("has tex coords");
			}
			if (mesh->HasTangentsAndBitangents()) {
				//printf_s("has tangents");
			}
		}
	}

	uvArray -= mesh->mNumFaces * 3 * 2;
	normalArray -= mesh->mNumFaces * 3 * 3;
	vertexArray -= mesh->mNumFaces * 3 * 3;
	tangentArray -= mesh->mNumFaces * 3 * 3;

	g_point_count = mesh->mNumVertices;

	if (mesh->HasPositions()) {
		printf("mesh has positions\n");
		g_vp = (GLfloat*)malloc(g_point_count * 3 * sizeof(GLfloat));
	}
	if (mesh->HasNormals()) {
		printf("mesh has normals\n");
		g_vn = (GLfloat*)malloc(g_point_count * 3 * sizeof(GLfloat));
	}
	if (mesh->HasTextureCoords(0)) {
		printf("mesh has texture coords\n");
		g_vt = (GLfloat*)malloc(g_point_count * 2 * sizeof(GLfloat));
	}
	if (mesh->HasTangentsAndBitangents()) {
		printf("mesh has tangents\n");
		g_vtans = (GLfloat*)malloc(g_point_count * 4 * sizeof(GLfloat));
	}

	for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
		if (mesh->HasPositions()) {
			const aiVector3D* vp = &(mesh->mVertices[v_i]);
			g_vp[v_i * 3] = (GLfloat)vp->x;
			g_vp[v_i * 3 + 1] = (GLfloat)vp->y;
			g_vp[v_i * 3 + 2] = (GLfloat)vp->z;
		}
		if (mesh->HasNormals()) {
			const aiVector3D* vn = &(mesh->mNormals[v_i]);
			g_vn[v_i * 3] = (GLfloat)vn->x;
			g_vn[v_i * 3 + 1] = (GLfloat)vn->y;
			g_vn[v_i * 3 + 2] = (GLfloat)vn->z;
		}
		if (mesh->HasTextureCoords(0)) {
			const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
			g_vt[v_i * 2] = (GLfloat)vt->x;
			g_vt[v_i * 2 + 1] = (GLfloat)vt->y;
		}
		if (mesh->HasTangentsAndBitangents()) {
			const aiVector3D* tangent = &(mesh->mTangents[v_i]);
			const aiVector3D* bitangent = &(mesh->mBitangents[v_i]);
			const aiVector3D* normal = &(mesh->mNormals[v_i]);

			glm::vec3 t(tangent->x, tangent->y, tangent->z);
			glm::vec3 n(normal->x, normal->y, normal->z);
			glm::vec3 b(bitangent->x, bitangent->y, bitangent->z);

			glm::vec3 t_i = normalize(t - n * dot(n, t));

			float det = (dot(cross(n, t), b));
			if (det < 0.0f) {
				det = -1.0f;
			}
			else {
				det = 1.0f;
			}

			g_vtans[v_i * 4] = t_i.x;
			g_vtans[v_i * 4 + 1] = t_i.y;
			g_vtans[v_i * 4 + 2] = t_i.z;
			g_vtans[v_i * 4 + 3] = det;
		}
	}

	return true;
}

void renderModel() {

	glPushMatrix(); // save current modelview matrix (mostly saves camera transform)
	glScalef(4, 4, 4);  //rescale model
	//glTranslated(xPos, yPos, zPos);	//reposition model
	//glRotated(rotAngle, 0, 1, 0); //rotate by about y-axis
	//glRotated(rotAngleVert, 1, 0, 0); //rotate by about x-axis

	glEnable(GL_COLOR_MATERIAL);
	//glColor3f(red, green, blue);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertexArray);
	glNormalPointer(GL_FLOAT, 0, normalArray);

	glClientActiveTexture(GL_TEXTURE0_ARB);
	glTexCoordPointer(2, GL_FLOAT, 0, uvArray);

	GLuint locVertex = glGetAttribLocation(shaderProgramID, "Vertex");
	glEnableVertexAttribArray(locVertex);
	glVertexAttribPointerARB(locVertex, 3, GL_FLOAT, GL_FALSE, 0, vertexArray);
	glBindAttribLocationARB(shaderProgramID, locVertex, "Vertex");

	GLuint locTangent = glGetAttribLocation(shaderProgramID, "Tangent");
	glEnableVertexAttribArray(locTangent);
	glVertexAttribPointerARB(locTangent, 3, GL_FLOAT, GL_FALSE, 0, tangentArray);
	glBindAttribLocationARB(shaderProgramID, locTangent, "Tangent");

	GLuint locNormal = glGetAttribLocation(shaderProgramID, "Normal");
	glEnableVertexAttribArray(locNormal);
	glVertexAttribPointerARB(locNormal, 3, GL_FLOAT, GL_FALSE, 0, normalArray);
	glBindAttribLocationARB(shaderProgramID, locNormal, "Normal");

	GLuint locTexcoords = glGetAttribLocation(shaderProgramID, "Texcoords");
	glEnableVertexAttribArray(locTexcoords);
	glVertexAttribPointerARB(locTexcoords, 2, GL_FLOAT, GL_FALSE, 0, uvArray);
	glBindAttribLocationARB(shaderProgramID, locTexcoords, "Texcoords");

	glLinkProgramARB(shaderProgramID);

	glDrawArrays(GL_TRIANGLES, 0, numVerts);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glPopMatrix();
	glFlush();


}

bool LoadFile(const std::string& fileName, std::string& outShader)
{
	std::ifstream file(fileName);
	if(!file.is_open())
	{
		std::cout << "Error Loading file: " << fileName << " - impossible to open file" << std::endl;
		return false;
	}

	if(file.fail())
	{
		std::cout << "Error Loading file: " << fileName << std::endl;
		return false;
	}

	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	outShader = stream.str();

	return true;
}

void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const std::string& vsFilename, const std::string& psFilename)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	std::string vs, ps;
	LoadFile(vsFilename, vs);
	AddShader(shaderProgramID, vs.c_str(), GL_VERTEX_SHADER);
	LoadFile(psFilename, ps);
	AddShader(shaderProgramID, ps.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

GLuint generateObjectBuffer(GLfloat vertices[], GLfloat colors[]) {
	
	GLuint numVertices = 12;
	// Genderate 1 generic buffer object, called VBO
	GLuint VBO;
 	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices*7*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	glBufferSubData (GL_ARRAY_BUFFER, 0, numVertices*3*sizeof(GLfloat), vertices);
	glBufferSubData (GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), numVertices*4*sizeof(GLfloat), colors);
return VBO;
}

GLuint generateObjectBuffer2() {

	GLuint numVertices = numVertsPlane + numVertsRabbit + 12;
	// Genderate 1 generic buffer object, called VBO
	//planeVBO;
	glGenBuffers(1, &planeVBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices * 7 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertsPlane * 3 * sizeof(GLfloat), vertexArrayPlane);
		
	//printf("vertex array: %i", vertexArray);
	
	glBufferSubData(GL_ARRAY_BUFFER, numVertsPlane * 3 * sizeof(GLfloat), numVertsRabbit * 3 * sizeof(GLfloat), vertexArrayRabbit);

	//glBufferSubData(GL_ARRAY_BUFFER, (numVertsPlane * 3 * sizeof(GLfloat)) + (numVertsRabbit * 3 * sizeof(GLfloat)), 12 * 3 * sizeof(GLfloat), vertexArrayCube);

	return planeVBO;
}

GLuint generateRabbitBuffer(GLuint verts) {

	GLuint numVertices = verts;
	glGenBuffers(1, &rabbitVBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, rabbitVBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), vertexArrayRabbit, GL_STATIC_DRAW);

	return rabbitVBO;
}

void linkCurrentBuffertoShader(GLuint shaderProgramID){
	GLuint numVertices = 12;
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	// Have to enable this
	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertices*3*sizeof(GLfloat)));

	int location = glGetUniformLocationARB(shaderProgramID, "perspectiveMat");
	
	glUniformMatrix4fvARB(location, 1 /*only setting 1 matrix*/, false /*transpose?*/, glm::value_ptr(perspectiveMat));


	GLuint locTangent = glGetAttribLocation(shaderProgramID, "Tangent");
	glEnableVertexAttribArray(locTangent);
	glVertexAttribPointer(locTangent, 3, GL_FLOAT, GL_FALSE, 0, tangentArray);
	glBindAttribLocation(shaderProgramID, locTangent, "Tangent");

	GLuint locNormal = glGetAttribLocation(shaderProgramID, "Normal");
	glEnableVertexAttribArray(locNormal);
	glVertexAttribPointer(locNormal, 3, GL_FLOAT, GL_FALSE, 0, normalArray);
	glBindAttribLocation(shaderProgramID, locNormal, "Normal");

	GLuint locTexcoords = glGetAttribLocation(shaderProgramID, "Texcoords");
	glEnableVertexAttribArray(locTexcoords);
	glVertexAttribPointer(locTexcoords, 2, GL_FLOAT, GL_FALSE, 0, uvArray);
	glBindAttribLocation(shaderProgramID, locTexcoords, "Texcoords");

}

void linkCurrentBuffertoShader2(GLuint shaderProgramID) {
	GLuint numVertices = numVertsPlane + numVertsRabbit + 12;
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	GLuint MatrixID = glGetUniformLocation(shaderProgramID, "MVP");
	// Have to enable this
	glEnableVertexAttribArray(positionID);
	glTranslatef(0.0f, 0.0f, -6.0f);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertsPlane * 3 * sizeof(GLfloat)));

	perspectiveMat +=
		glm::mat4(
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0, 0.0f, 1.0f
	);

	int location = glGetUniformLocationARB(shaderProgramID, "perspectiveMat");

	glUniformMatrix4fvARB(location, 1 /*only setting 1 matrix*/, false /*transpose?*/, glm::value_ptr(perspectiveMat));
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPmatrix[0][0]);

	GLint locTangent = glGetAttribLocation(shaderProgramID, "Tangent");
	glEnableVertexAttribArray(locTangent);
	glVertexAttribPointer(locTangent, 3, GL_FLOAT, GL_FALSE, 0, tangentArray);
	glBindAttribLocation(shaderProgramID, locTangent, "Tangent");

	GLint locNormal = glGetAttribLocation(shaderProgramID, "Normal");
	glEnableVertexAttribArray(locNormal);
	glVertexAttribPointer(locNormal, 3, GL_FLOAT, GL_FALSE, 0, normalArray);
	glBindAttribLocation(shaderProgramID, locNormal, "Normal");

	GLint locTexcoords = glGetAttribLocation(shaderProgramID, "Texcoords");
	glEnableVertexAttribArray(locTexcoords);
	glVertexAttribPointer(locTexcoords, 2, GL_FLOAT, GL_FALSE, 0, uvArray);
	glBindAttribLocation(shaderProgramID, locTexcoords, "Texcoords");

}

void renderRabbit() {
	//glScalef(4, 4, 4);  //rescale model

	glDrawArrays(GL_TRIANGLES, 0, numVerts);//g_point_count);
	
}

void display(){

	glClear(GL_COLOR_BUFFER_BIT);
	// NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!	
	//glDrawArrays(GL_TRIANGLES, 0, 12); //for triangles
	//glClear(GL_COLOR_BUFFER_BIT);
	
	/* For Euler angle rotation
	RotationMatrix = RotationX * RotationY * RotationZ;
	*/

	quaternion = glm::quat(glm::vec3(alpha, beta, gamma));
	RotationMatrix = glm::toMat4(quaternion);



	UpdateTranslationMatrix(0,0.0f,0.0f);
	glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
	ModelViewMatrix = TranslationMatrix * ModelViewMatrix;
	MVPmatrix = ProjectionMatrix * ModelViewMatrix *  RotationMatrix;
	//MVPmatrix *= TranslationMatrix;
	
	//glPushMatrix();
	//glTranslatef(4, 4, 4);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	//glTranslatef(0.0f, 0.0f, -6.0f);
	linkCurrentBuffertoShader2(shaderProgramID);
	renderRabbit();
	//glPopMatrix();
	
	//glBindBuffer(GL_ARRAY_BUFFER, rabbitVBO);
	//linkCurrentBuffertoShader2(shaderProgramID, numVertsRabbit);
	//renderRabbit();

	//assert(loadModel("../Rabbit.obj"));

	//glPopMatrix();
	//glFlush();


    glutSwapBuffers();
}

void init()
{
	// Create 3 vertices that make up a triangle that fits on the viewport 
	GLfloat vertices[] = {
			-1.0f, -1.2f, 0.0f,
			1.0f, -1.2f, 0.0f,
			0.0f, 0.0f, 0.0f,

			0.0f, 0.0f, 0.0f,
			1.0f, -1.2f, 0.0f,
			1.0f, 1.2f, 0.0f,

			0.0f, 0.0f, 0.0f,
			-1.0f, -1.2f, 0.0f,
			-1.0f, 1.2f, 0.0f,

			0.0f, 0.0f, 0.0f,
			1.0f, 1.2f, 0.0f,
			-1.0f, 1.2f, 0.0f
	};

	// Create a color array that identfies the colors of each vertex (format R, G, B, A)
	GLfloat colors[] = {
			1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,

			1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f,

			1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f,

			1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f

	};

	GLfloat cube_vertices[] = {
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end

		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end

		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,

		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,

		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,

		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,

		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,

		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,

		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};

	// Set up the shaders
	shaderProgramID = CompileShaders("../diffuse.vs", "../diffuse.ps");

	// Put the vertices and colors into a vertex buffer object
	generateObjectBuffer(vertices, colors);
	//generateObjectBuffer2();

	// Link the current buffer to the shader
	linkCurrentBuffertoShader(shaderProgramID);	

	assert(loadModel("../Rabbit.obj"));
	numVertsRabbit = numVerts;
	vertexArrayRabbit = vertexArray;
	//generateRabbitBuffer(numVertsRabbit);

	assert(loadModel("../biplane_complete.obj"));
	numVertsPlane = numVerts;
	vertexArrayPlane = vertexArray;
	generateObjectBuffer2();

	vertexArrayCube = vertices;
	numVerts = numVertsPlane + numVertsRabbit + 12;

}

void initGL()
{
	glClearColor(0.8, 0.8, 0.8, 0.8); //background color
	glClearDepth(1.0);	//background depth value


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 0.1, 100);  //setup a perspective projection

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glTranslatef(4,0,0);

	gluLookAt(				//set up camera
		0.0, 0.0, 5.0,		// eye position
		0.0, 0.0, 0.0,		// lookat position
		0.0, 1.0, 0.0		// up direction
	);

	CameraMatrix = glm::lookAt(			//set up camera
		glm::vec3(0.0, 0.0, 5.0),		// eye position
		glm::vec3(0.0, 0.0, 0.0),		// lookat position
		glm::vec3(0.0, 1.0, 0.0)		// up direction
	);

	ViewMatrix = glm::lookAt(
		glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	//UpdateTranslationMatrix(4.0f, 0, 0);
	ModelMatrix = glm::mat4(1.0f); //* TranslationMatrix;

	ProjectionMatrix = glm::perspective(
		glm::radians(90.0f),         // The horizontal Field of View, in degrees : the amount of "zoom". Think "camera lens". Usually between 90� (extra wide) and 30� (quite zoomed in)
		4.0f / 3.0f, // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
		0.1f,        // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		100.0f       // Far clipping plane. Keep as little as possible.
	);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	MVPmatrix = ProjectionMatrix * ViewMatrix * ModelMatrix; // Remember : inverted !
}

void loadWithNormalMap(const char *diffuse, const char *normal, const char *depth)
{
	GLint DiffuseTextureID = glGetUniformLocation(shaderProgramID, "diffuseTexture");
	GLint FeatureTextureID = glGetUniformLocation(shaderProgramID, "featureTexture");
	GLint DepthTextureID = glGetUniformLocation(shaderProgramID, "depthTexture");

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(DiffuseTextureID, 0);
	tex_diffuse;
	glGenTextures(1, &tex_diffuse);

	tex_diffuse = SOIL_load_OGL_texture
	(
		diffuse,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if (0 == tex_diffuse)
	{
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}
	glBindTexture(GL_TEXTURE_2D, tex_diffuse);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(FeatureTextureID, 1);
	tex_normal;
	glGenTextures(1, &tex_normal);

	tex_normal = SOIL_load_OGL_texture
	(
		normal,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if (0 == tex_normal)
	{
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}

	glBindTexture(GL_TEXTURE_2D, tex_normal);

	glActiveTexture(GL_TEXTURE2);
	glUniform1i(DepthTextureID, 2);
	tex_depth;
	glGenTextures(1, &tex_depth);

	tex_depth = SOIL_load_OGL_texture
	(
		depth,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if (0 == tex_depth)
	{
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}

	glBindTexture(GL_TEXTURE_2D, tex_depth);

}


/*************************************************************
** keyboard callback function **

*************************************************************/

void keyboard(unsigned char k, int x, int y)
{
	switch (k)
	{
	case 'a':
		printf("alpha: %i", alpha);
		alpha += 0.1f;
		//pitch rotation
		RotationX = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cos(alpha), -sin(alpha), 0.0f,
			0.0f, sin(alpha), cos(alpha), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	case 'd':
		alpha -= 0.1f;
		// - pitch rotation
		RotationX = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cos(alpha), -sin(alpha), 0.0f,
			0.0f, sin(alpha), cos(alpha), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	case 'w':
		beta += 0.1;
		//yaw rotation
		RotationY = glm::mat4(
			cos(beta), 0.0f, sin(beta), 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			-sin(beta), 0.0f, cos(beta), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	case 's':
		beta -= 0.1;
		// - yaw rotation
		RotationY = glm::mat4(
			cos(beta), 0.0f, sin(beta), 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			-sin(beta), 0.0f, cos(beta), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	case 'z':
		gamma += 0.1;
		// roll rotation
		RotationZ = glm::mat4(
			cos(gamma), -sin(gamma), 0.0f, 0.0f,
			sin(gamma), cos(gamma), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	case 'x':
		gamma -= 0.1;
		// - roll rotation
		RotationZ = glm::mat4(
			cos(gamma), -sin(gamma), 0.0f, 0.0f,
			sin(gamma), cos(gamma), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	}
	
	glutPostRedisplay();
}

void main(){

	// Set up the window
	//glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Hello Triangle");

	glewInit();
	initGL();

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	 // A call to glewInit() must be done after glut is initialized!
    /*
	GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
	*/

	//assert(loadModel("../Rabbit.obj"));
	loadWithNormalMap("../biplano_last.jpg", "../biplano_last.jpg", "../biplano_last.jpg"); //("Textures/lava.jpg", "Textures/lava_normal.jpg");
	
	Bone *testBone = createBone("test", glm::mat4(1.0));
	boneDumpTree(testBone, 1);
	Skeleton skeleton = createSkeleton(testBone);
	skeletonDumpTree(skeleton); 
	//testBone = skeleton.rootBone;
	//printf("skeleton root: ");
	//boneDumpTree(skeleton.rootBone, 1); 

	// Set up your objects and shaders
	init();

	// Begin infinite event loop
	glutMainLoop();
   // return 0;
}











