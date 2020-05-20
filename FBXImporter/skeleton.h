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
#ifndef AAT_SKELETON_H
#define AAT_SKELETON_H

#include <string>
#include <vector>
#include <map>
#include "joint.h"

static const int MAX_BONES = 100;
class Vertex;
class GeometryData;
class Skeleton{
public:
	Skeleton();
	~Skeleton();
	
	void updateCacheFromOffset();
	void resetOffset();
	void setInverseBindPoseFromOffset();
	int getJointIndex(const char* name);
	std::vector<Vertex>* generateVertices();
	std::vector<Vertex>* transformVertices(GeometryData* geometry);
	std::string root;
	std::map<std::string, Joint*> joints;
	std::vector<std::string> jointOrder;
	std::vector<glm::mat4> cachedTransformations;
    double frameTime;
};

#endif //AAT_SKELETON_H