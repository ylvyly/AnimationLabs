
#pragma once

#include <string>
#include <list>
#include <glm.hpp>
#include <stdlib.h>

using namespace std;
/*
struct Bone;
*/

#define _CRT_SECURE_NO_WARNINGS

/*
class Bone
{
public:

	string m_boneName;
	Bone *m_pParent;
	list<Bone*> m_lChildren;
	glm::mat4 m_transform;

	Bone addChild(Bone *child);

	Bone();
	~Bone();
};

*/

#pragma once
/*
#include "Bone.h"

using namespace std;

Bone::Bone()
{
string m_boneName;
Bone *m_pParent;
list<Bone*> m_lChildren;
glm::mat4 m_transform;
}


Bone::~Bone()
{
}

Bone Bone::addChild(Bone *child) {

m_lChildren.push_back(child);
}
*/

#define MAX_CHILD_COUNT 8
#define _CRT_SECURE_NO_WARNINGS 
#pragma warning(disable:4996)

typedef struct Bone
{
	//char boneName[20];
	string boneName;
	//Bone *parent;
	//list<Bone*> lChildren;
	struct Bone *children[MAX_CHILD_COUNT],
		*parent;
	uint8_t childCount;
	glm::mat4 transform;

	Bone() {}; //: boneName(""), children(), parent(NULL), childCount(0), transform(NULL) {};
	//Bone(char name[20], Bone *childrenInit[0], Bone *parentBone, uint8_t childNum, glm::mat4 boneTransform) : boneName(""), children(), parent(parentBone), childCount(childNum), transform(boneTransform)
	//{
	//};

}; Bone;


Bone *createBone(string name, glm::mat4 boneTransform) {

	Bone* newBone = new Bone;
	newBone->boneName = name;
	newBone->childCount = 0;
	newBone->transform = boneTransform;

	return newBone;
}


Bone *addChild(Bone *root, char *childName, glm::mat4 childTransform) {

	Bone *t;
	int i;

	if (!root) /* If there is no root, create one */
	{
		if (!(root = (Bone *)malloc(sizeof(Bone))))
			return NULL;
		root->parent = NULL;
	}
	else
	{
		/* Allocate the child */
		if (!(t = (Bone *)malloc(sizeof(Bone))))
			return NULL; /* Error! */

		t->parent = root; /* Set it's parent */
		root->children[root->childCount++] = t; /* Increment the childCounter and set the pointer */
		root = t; /* Change the root */

		root->boneName=childName;
	}

	for (i = 0; i < MAX_CHILD_COUNT; i++)
		root->children[i] = NULL;


	return root;
}

/* Dump on stdout the bone structure. Root of the tree should have level 1 */
void boneDumpTree(Bone *root, uint8_t level)
{
	int i;

	if (!root)
		return;

	for (i = 0; i < level; i++)
		printf("#"); /* We print # to signal the level of this bone. */

	//std::cout << "Bone: " << root->boneName;
	printf(" Child count: %x Bone: %s Transform: %s\n",
		root->childCount, root->boneName, root->transform);

	/* Recursively call this on my children */
	for (i = 0; i < root->childCount; i++)
		boneDumpTree(root->children[i], level + 1);
}

//transform bone and all its children
void transformBone(Bone *bone, glm::mat4 transformationMatrix) {

	bone->transform *= transformationMatrix;

	int i;
	for (i = 0; i < bone->childCount; i++) {
		transformBone(bone->children[i], transformationMatrix);
	}
}

