//#pragma once
/*
#include "Skeleton.h"

using namespace std;

Skeleton::Skeleton()
{
	Bone rootBone;
	list<Bone*> boneList = {};
}


Skeleton::~Skeleton()
{
}

//transform bone and all its children
void Skeleton::transformBone(Bone movedBone, glm::mat4 transformationMatrix) {

	movedBone.m_transform *= transformationMatrix;

	list<Bone*> children = movedBone.m_lChildren;
	typename std::list<Bone*>::iterator bone;

	for (bone = children.begin(); bone != children.end(); ++bone) {
	
		Bone curBone = *(*bone);
		transformBone(curBone, transformationMatrix);

	}

}
#include "Skeleton.h"
#include "Bone.h"

#define MAX_BONE_COUNT 256
*/




/*
//transform bone and all its children
void transformBone(Bone *movedBone, glm::mat4 transformationMatrix) {

	//glm::mat4 movedBoneTranform = movedBone.m_transform;

	movedBone->transform *= transformationMatrix;

	Bone* children = *(movedBone->children);
	typename std::list<Bone*>::iterator bone;

	for (bone = children.begin(); bone != children.end(); ++bone) {

		Bone curBone = *(*bone);
		transformBone(curBone, transformationMatrix);

	}
}
*/