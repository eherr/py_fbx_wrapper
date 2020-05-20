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
#include <skeleton.h>
#include <glm\glm.hpp>
#include <graphic_types.h>
#include <geometry_data.h>

Skeleton::Skeleton(){
	root = "";
    frameTime = 0.013889;
	joints = std::map<std::string, Joint*>();
	jointOrder = std::vector<std::string>();
	cachedTransformations = std::vector<glm::mat4>();
	while (cachedTransformations.size() < MAX_BONES){
		cachedTransformations.push_back(glm::mat4());
	}
}



Skeleton::~Skeleton(){

}


void Skeleton::updateCacheFromOffset(){
	glm::mat4 parentT = glm::mat4();
	joints[root]->updateCacheFromOffset(parentT);

}


int Skeleton::getJointIndex(const char* name){
	auto pos = std::find(jointOrder.begin(), jointOrder.end(), name);
	if (pos != jointOrder.end())
		return pos - jointOrder.begin();
	else
		return -1;
}


void Skeleton::setInverseBindPoseFromOffset(){
	updateCacheFromOffset();
	for (auto it = joints.begin(); it != joints.end(); it++){
		 glm::mat4 inverse = glm::inverse(joints[it->first]->cachedGlobalTransformationMatrix);
		 joints[it->first]->invBindPose = inverse;
	}
}

void Skeleton::resetOffset(){
	for (auto it = joints.begin(); it != joints.end(); it++){
		joints[it->first]->offsetMatrix = glm::mat4();
	}
}


std::vector<Vertex>* Skeleton::generateVertices(){
	std::vector<Vertex>* vertices = new std::vector<Vertex>();
	joints[root]->addVertexRecursively(vertices, NULL);
	return vertices;
}

std::vector<Vertex>* Skeleton::transformVertices(GeometryData* geometry){
	std::vector<Vertex>* vertices = new std::vector<Vertex>();
	for (int i = 0; i < geometry->jointWeights.size(); i++){
		glm::vec4 v = glm::vec4(0,0,0,1);
		for (int j = 0; j < 4; j++){
			int id = geometry->jointWeights[i].IDs[j];
			if (id >= 0 && id <jointOrder.size()){
				v += geometry->jointWeights[i].Weights[j] * joints[jointOrder[id]]->cachedGlobalTransformationMatrix* joints[jointOrder[id]]->invBindPose * geometry->vertices[i].toglmVec();
			}
		}
		vertices->push_back(Vertex(v));
	}
	return vertices;
}