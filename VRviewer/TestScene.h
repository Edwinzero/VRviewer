#pragma once
#ifndef _TEST_SCENE_H
#define _TEST_SCENE_H
#include <vector>
#include <iostream>

#include <glm\mat4x4.hpp>
#include <glm\vec4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void PrintGLMmat4(glm::mat4 &mat) {
	printf("%f, %f, %f, %f \n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
	printf("%f, %f, %f, %f \n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
	printf("%f, %f, %f, %f \n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
	printf("%f, %f, %f, %f \n", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}
void PrintGLMmat4(glm::mat4 &mat, const char* str) {
	printf("%s: \n", str);	
	printf("%f, %f, %f, %f \n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
	printf("%f, %f, %f, %f \n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
	printf("%f, %f, %f, %f \n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
	printf("%f, %f, %f, %f \n", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}

void AddCubeVertex(float fl0, float fl1, float fl2, float fl3, float fl4, std::vector<float> &vertdata)
{
	vertdata.push_back(fl0);
	vertdata.push_back(fl1);
	vertdata.push_back(fl2);
	vertdata.push_back(fl3);
	vertdata.push_back(fl4);
}

void AddCubeToScene(glm::mat4 mat, std::vector<float> &vertdata)
{
	// glm::mat4 mat( outermat.data() );

	glm::vec4 A = mat * glm::vec4(0, 0, 0, 1);
	glm::vec4 B = mat * glm::vec4(1, 0, 0, 1);
	glm::vec4 C = mat * glm::vec4(1, 1, 0, 1);
	glm::vec4 D = mat * glm::vec4(0, 1, 0, 1);
	glm::vec4 E = mat * glm::vec4(0, 0, 1, 1);
	glm::vec4 F = mat * glm::vec4(1, 0, 1, 1);
	glm::vec4 G = mat * glm::vec4(1, 1, 1, 1);
	glm::vec4 H = mat * glm::vec4(0, 1, 1, 1);

	// triangles instead of quads
	AddCubeVertex(E.x, E.y, E.z, 0, 1, vertdata); //Front
	AddCubeVertex(F.x, F.y, F.z, 1, 1, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 1, 0, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 1, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 0, 0, vertdata);
	AddCubeVertex(E.x, E.y, E.z, 0, 1, vertdata);

	AddCubeVertex(B.x, B.y, B.z, 0, 1, vertdata); //Back
	AddCubeVertex(A.x, A.y, A.z, 1, 1, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 0, 0, vertdata);
	AddCubeVertex(B.x, B.y, B.z, 0, 1, vertdata);

	AddCubeVertex(H.x, H.y, H.z, 0, 1, vertdata); //Top
	AddCubeVertex(G.x, G.y, G.z, 1, 1, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 0, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 0, 1, vertdata);

	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata); //Bottom
	AddCubeVertex(B.x, B.y, B.z, 1, 1, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 1, 0, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 1, 0, vertdata);
	AddCubeVertex(E.x, E.y, E.z, 0, 0, vertdata);
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata);

	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata); //Left
	AddCubeVertex(E.x, E.y, E.z, 1, 1, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 1, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 0, 0, vertdata);
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata);

	AddCubeVertex(F.x, F.y, F.z, 0, 1, vertdata); //Right
	AddCubeVertex(B.x, B.y, B.z, 1, 1, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 0, 0, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 0, 1, vertdata);
}


//================================
// default draw event
//================================
void DrawCoord() {
	int num = 2 * 7;
	int grad = 1;

	glBegin(GL_LINES);

	for (int z = -num; z <= num; z++) {
		if (z == 0) {
			//x axis
			glColor3f(1, 0, 0);
		}
		else {
			glColor3f(0.2, 0.2, 0.3);
		}
		glVertex3f(-num * grad, 0, z * grad);
		glVertex3f(+num * grad, 0, z * grad);
	}

	for (int x = -num; x <= num; x++) {
		if (x == 0) {
			//z axis
			glColor3f(0, 0, 1);
		}
		else {
			glColor3f(0.2, 0.2, 0.3);
		}
		glVertex3f(x * grad, 0, -num * grad);
		glVertex3f(x * grad, 0, +num * grad);
	}

	glEnd();
}

#endif // !_TEST_SCENE_H