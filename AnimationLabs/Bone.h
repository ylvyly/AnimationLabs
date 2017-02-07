
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
	char boneName[20];
	//Bone *parent;
	//list<Bone*> lChildren;
	struct Bone *children[MAX_CHILD_COUNT],
		*parent;
	uint8_t childCount;
	glm::mat4 transform;

}; Bone;

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

		strcpy(root->boneName, childName);
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

	printf("%d %d %s\n",
		root->childCount, root->boneName, root->transform);

	/* Recursively call this on my children */
	for (i = 0; i < root->childCount; i++)
		boneDumpTree(root->children[i], level + 1);
}