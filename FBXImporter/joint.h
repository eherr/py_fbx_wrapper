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
#ifndef AAT_JOINT_H
#define AAT_JOINT_H

#include <glm\vec3.hpp>
#include <string>
#include <vector>
#include <glm\mat4x4.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>

class Vertex;
class Skeleton;
class Joint{
public:
		Joint(Skeleton* skeleton);
		~Joint();
		void updateCacheFromOffset(glm::mat4& parentT);
		void addVertexRecursively(std::vector<Vertex>* vertices, glm::mat4* parentTransform);
		std::string name;
		int index;
		unsigned int numChannels;
		glm::mat4 offsetMatrix;
        glm::vec3 offset;
        glm::quat rotation;
		std::string parent;
		std::vector<Joint*> children;
		glm::mat4 invBindPose;
		glm::mat4 cachedGlobalTransformationMatrix;
		Skeleton* skeleton;


};

#endif //AAT_JOINT_H