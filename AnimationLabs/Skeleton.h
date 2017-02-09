

#pragma once

#include "Bone.h"
#include <list>
#include <glm.hpp>
#include <iostream>
#include <iterator>

#define MAX_BONE_COUNT 256

using namespace std;

typedef struct Skeleton
{
	struct Bone *rootBone;
	struct Bone *bones[MAX_BONE_COUNT];
	uint8_t boneCount;
	//list<Bone*> boneList; //= {};

	Skeleton() : rootBone(NULL), boneCount(0) {};

}; Skeleton;

Skeleton addBone(Skeleton skeleton, Bone *bone) {

	if (!skeleton.rootBone) /* If there is no root, create one */
	{
		printf("Skeleton doesn't have a root, adding one. ");
		skeleton.rootBone = bone;
		//boneDumpTree(bone, 1);
		//boneDumpTree(skeleton.rootBone, 1);
		skeleton.boneCount++;
		//printf("Bone count: %i ", skeleton.boneCount);
	}
	else
	{		
		printf("Adding a bone to skeleton. ");
		skeleton.bones[skeleton.boneCount] = bone;
		skeleton.boneCount++;
	}
	return skeleton;
}


/* Dump on stdout the bone structure. Root of the tree should have level 1 */
void skeletonDumpTree(Skeleton skeleton)
{
	printf("Bone count now: %i ", skeleton.boneCount);
	int i;
	if (!skeleton.rootBone){
		printf("No skeleton root. ");
		return;
	}
	else {
		printf("Skeleton root bone: ");
		boneDumpTree(skeleton.rootBone, 1);
	}
	printf("Skeleton bone count: %i ", skeleton.boneCount);

	/* Recursively call this on my children */
	for (i = 1; i < skeleton.boneCount; i++) {
		printf("Skeleton child bone # % i", i);
		boneDumpTree(skeleton.bones[i-1], 1);
	}
}

/*
class Skeleton
{
public:

	Bone rootBone;
	list<Bone*> boneList;

	void transformBone(Bone movedBone, glm::mat4 transformationMatrix);

	Skeleton();
	~Skeleton();
};

*/