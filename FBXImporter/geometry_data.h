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
#ifndef GEOMETRY_DATA_
#define GEOMETRY_DATA_
#include <string>
#include <vector>
#include <map>
#include "graphic_types.h"
#include <skeleton.h>
#include <joint_frames.h>

class GeometryData{
	public:
		GeometryData();
		std::vector<Vertex> vertices;
		std::vector<Normal> normals;
		std::vector<unsigned short> indices;
		std::map<int, std::vector<int>> originalIndexVertexMapping;
		std::vector<Color> colors;
		std::vector<UVCoord> uvs;
        Skeleton* skeleton;
		std::vector<VertexJointData> jointWeights;
        std::map<std::string, JointFramesMap> animations;
        int nPolyVertices;
		std::string textureName; //owned by texture manager
        std::string texturePath;
		unsigned int drawMode;
		std::string shaderName;
		bool hasColor();
		bool hasTexture();
		bool hasJointWeightData();
		void scale(float factor);
		void flipYandZ();
		void flipUVCoords();
        int getNumAnimations();
};

class GeometryDataList {
    public:
        GeometryDataList();
        std::vector<GeometryData*> meshList;
        Skeleton* skeleton;
        std::map<std::string, JointFramesMap> animations;
       
       
};

#endif // GEOMETRY_DATA_