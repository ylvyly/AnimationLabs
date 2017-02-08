

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

	Skeleton() {};

}; Skeleton;

Skeleton addBone(Skeleton skeleton, Bone *bone) {

	if (!skeleton.rootBone) /* If there is no root, create one */
	{
		skeleton.rootBone = bone;
	}
	else
	{
		skeleton.boneCount++;
		skeleton.bones[skeleton.boneCount] = bone;
	}
	return skeleton;
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