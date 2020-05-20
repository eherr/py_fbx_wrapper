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
#ifndef FBX_GEOMETRY_LOADER_H_
#define FBX_GEOMETRY_LOADER_H_
#include <fbxsdk.h>
#include <geometry_data.h>
class Skeleton;
class FBXGeometryLoader{
	public:
		FBXGeometryLoader();
		~FBXGeometryLoader();
		bool loadGeometryDataFromFile(const char* path, GeometryDataList* geometryDataList);
	private:
		bool extractAnimations(fbxsdk::FbxNode* pNode, GeometryDataList* geometryData);
		bool extractSkeletonWeightsFromMeshNode(FbxMesh* mesh, GeometryData* geometryData);
		void extractTextureNamesFromNode(fbxsdk::FbxNode* pNode, std::vector<std::string>& textureFileNames);
		GeometryData* createGeometryDataFromMesh(fbxsdk::FbxMesh* pMesh,  bool& success);
		GeometryData* createColoredGeometryDataFromMesh(fbxsdk::FbxMesh* pMesh, bool& success);
		GeometryData* extractGeometryDataFromNodeAttribute(fbxsdk::FbxNode* node, int attributeIndex, bool& success);
		void extractMeshListFromNode(fbxsdk::FbxNode* node, GeometryDataList* geometryDataList, int level);
        void extractSkeletonFromNode(fbxsdk::FbxNode* node, GeometryDataList* geometryDataList, int level);
        UVCoord getUVCoordinate(fbxsdk::FbxMesh* pMesh, FbxGeometryElementUV* fbxLayerUV, int controlPointIndex, int iPolygon, int iPolygonVertex);
        Normal getNormal(fbxsdk::FbxMesh* pMesh,  int controlPointIndex, int vertexCount);
		fbxsdk::FbxManager* lSdkManager = NULL;
		fbxsdk::FbxScene* fbxScene = NULL;
		fbxsdk::FbxGeometryConverter* geometryConverter = NULL;

};

#endif //FBX_GEOMETRY_LOADER_H_