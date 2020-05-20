/*
*
* Copyright 2019 DFKI GmbH.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files(the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and / or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions :
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN
* NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
* USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "joint.h"
#include "skeleton.h"
#include <glm\glm.hpp>
#include <graphic_types.h>

Joint::Joint(Skeleton* skeleton){
	offsetMatrix = glm::mat4();
	invBindPose = glm::mat4();
	cachedGlobalTransformationMatrix = glm::mat4();
	children = std::vector<Joint*>();
	index = -1;
	numChannels = 0;
	this->skeleton = skeleton;
}


Joint::~Joint(){

}


void Joint::updateCacheFromOffset(glm::mat4& parentT){
	//update cache by frame paramters in range (index, index+numChannels)
	if (index > -1){
		cachedGlobalTransformationMatrix = parentT *offsetMatrix;
		skeleton->cachedTransformations[index] =  cachedGlobalTransformationMatrix;
		for (int i = 0; i < children.size(); i++){
			children[i]->updateCacheFromOffset(cachedGlobalTransformationMatrix);
		}
	}
}



void Joint::addVertexRecursively(std::vector<Vertex>* vertices, glm::mat4* parentTransform){
	if (parentTransform != NULL){
		glm::vec3 lastOffset = glm::vec3((*parentTransform)[3][0],
			(*parentTransform)[3][1],
			(*parentTransform)[3][2]);
		glm::vec4 nodeOffset = glm::vec4();
		if (index >= 0){
			nodeOffset = glm::vec4(cachedGlobalTransformationMatrix[3][0],
				cachedGlobalTransformationMatrix[3][1],
				cachedGlobalTransformationMatrix[3][2], 1);
		}
		else{
			nodeOffset = (*parentTransform) * glm::vec4(offsetMatrix[3][0],
																		offsetMatrix[3][1],
																		offsetMatrix[3][2],1);
	
		}
		vertices->push_back(Vertex(lastOffset));
		vertices->push_back(Vertex(nodeOffset.x, nodeOffset.y, nodeOffset.z));
	}
	for (int i = 0; i < children.size(); i++){
		children[i]->addVertexRecursively(vertices, &cachedGlobalTransformationMatrix);
	}

}