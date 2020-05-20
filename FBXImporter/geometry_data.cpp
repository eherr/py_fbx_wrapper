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
#include "geometry_data.h"
#include <GL\glew.h>

GeometryData::GeometryData(){
	vertices = std::vector<Vertex>();
	colors = std::vector<Color>();
	normals = std::vector<Normal>();
	uvs = std::vector<UVCoord>();
	originalIndexVertexMapping = std::map<int, std::vector<int>>();
	textureName = "";
    skeleton = NULL;
	indices = std::vector<unsigned short>();
	shaderName = "color";
	drawMode = GL_LINES;
	
}




bool GeometryData::hasColor(){
	return colors.size() > 0;
}
bool GeometryData::hasTexture(){
	return textureName != "";
}


bool GeometryData::hasJointWeightData(){
	return jointWeights.size() > 0 && skeleton != NULL;
}

void GeometryData::scale(float factor){
	for (int i = 0; i < vertices.size(); i++){
		vertices[i] *= factor;
	}
}


void GeometryData::flipYandZ(){
	for (int i = 0; i < vertices.size(); i++){
		float temp = vertices[i].y;
		vertices[i].y = vertices[i].z;
		vertices[i].z = temp;
	}
}

void GeometryData::flipUVCoords(){
	for (int i = 0; i < uvs.size(); i++){
		uvs[i].flip();
	}
}
int GeometryData::getNumAnimations() {
    return animations.size();
}

GeometryDataList::GeometryDataList() {
    meshList = std::vector<GeometryData*>();
    skeleton = NULL;
}
