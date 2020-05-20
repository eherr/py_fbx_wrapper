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

/* Wrapper for the FBX SDK to import a character mesh and skeleton
sources:
FBX SDK samples
https://github.com/sho3la/FBX-Loader-OpenGL-C-/tree/master/fbxLoading/mypro 
https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582
*/

#include <glm\glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>
#include "fbx_geometry_loader.h"
#include <algorithm>
#include <iostream>

using namespace fbxsdk;

FBXGeometryLoader::FBXGeometryLoader(){

	lSdkManager = NULL;
	fbxScene = NULL;

}
FBXGeometryLoader::~FBXGeometryLoader(){
}


//based on http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/how-to-work-with-fbx-sdk-r3582
Joint* extractSkeletonDataNodeHierarchyRecursively(FbxNode* node, Skeleton* skeleton, std::string& parent, int depth) {
    auto name = std::string(node->GetName());
	FbxDouble3 t = node->LclTranslation.Get();
    auto time = FbxTime();
    time.SetFrame(0);
    FbxAMatrix localTransform = node->EvaluateLocalTransform(time);
    FbxQuaternion q = localTransform.GetQ();
	Joint* joint = new Joint(skeleton);
    joint->children = std::vector<Joint*>();
	joint->parent = parent;
    joint->name = name;
    joint->offset = glm::vec3(t.mData[0], t.mData[1], t.mData[2]);
    //change order from xyzw to wxyz
    joint->rotation = glm::quat(q.mData[3], q.mData[0], q.mData[1], q.mData[2]); 
	skeleton->joints[name] = joint;

    //Log::write((std::string)"add " + name + " to skeleton");
	joint->index = skeleton->jointOrder.size(); //set it later based on lookup in cluster list
	skeleton->jointOrder.push_back(joint->name);
    int nChildren = node->GetChildCount();
	for (int i = 0; i < nChildren; i++)
	{
        auto attribute = node->GetChild(i)->GetNodeAttribute();
        if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton) {
            auto c_joint = extractSkeletonDataNodeHierarchyRecursively(node->GetChild(i), skeleton, joint->name, depth + 1);
            skeleton->joints[name]->children.push_back(c_joint);
          }
	}
    if(nChildren < 1){
        Joint* endSite = new Joint(skeleton);
        endSite->name = name+"EndSite";
        endSite->offset = glm::vec3();
        endSite->rotation = glm::quat();
        endSite->parent = name;
        endSite->children = std::vector<Joint*>();
        skeleton->joints[endSite->name] = endSite;
        skeleton->joints[name]->children.push_back(endSite);
    }
    return joint;
}


// source: http://stackoverflow.com/questions/19634369/read-texture-filename-from-fbx-with-fbx-sdk-c
void FBXGeometryLoader::extractTextureNamesFromNode(FbxNode* pNode, std::vector<std::string>& textureFileNames){

	int materialCount = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();
	for (int i = 0; i < materialCount; i++){
		FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)pNode->GetSrcObject<FbxSurfaceMaterial>(i);
		if (material != NULL)
		{
			FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
			if (layeredTextureCount > 0)
			{
				std::cout << "handling of layered texture is not implemented" << std::endl;
			}
			else{
				int textureCount = prop.GetSrcObjectCount<FbxTexture>();
				for (int j = 0; j < textureCount; j++){
					FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(j));
					FbxFileTexture* filetexture = (FbxFileTexture*)texture;
					textureFileNames.push_back(filetexture->GetFileName());
				}
			}
		}
	}
}

// sources: http://www.gamedev.net/topic/656345-texture-uvs-on-fbx-mesh-are-foo-bared/
//  http://www.gamedev.net/topic/577127-fbx-sdkprolem-with-getting-coords-and-normals/
GeometryData* FBXGeometryLoader::createGeometryDataFromMesh(FbxMesh* pMesh, bool& success){
	
	GeometryData* geometryData = new GeometryData();
	FbxVector4* controlPoints = pMesh->GetControlPoints();
	FbxLayerElementUV* fbxLayerUV = pMesh->GetLayer(0)->GetUVs();
    //FbxGeometryElementUV* fbxLayerUV = pMesh->GetElementUV(0);

	if (fbxLayerUV == NULL){
		std::cout << "No UV layers in mesh" << std::endl;
		success = false;
		return geometryData;
	}
	unsigned int ctrlPointCount = pMesh->GetControlPointsCount();
	unsigned int polygonCount = pMesh->GetPolygonCount();
    bool polyCountSet = false;
	int countPolyVerts = 3;
	int vertexCount = 0;
    
	for (int iPolygon = 0; iPolygon < polygonCount; iPolygon++) {
		int tempCountPolyVerts = pMesh->GetPolygonSize(iPolygon);
		if (tempCountPolyVerts < 3 && tempCountPolyVerts > 4 && tempCountPolyVerts != countPolyVerts && polyCountSet){
			//Log::write((std::string) "Only triangles and quads are supported for now");
			success = false;
			return geometryData;
		}
		countPolyVerts = tempCountPolyVerts;
		polyCountSet = true;
		//TODO either map uvs to indices or map weights to vertices
		for (unsigned iPolygonVertex = 0; iPolygonVertex < countPolyVerts; iPolygonVertex++) {
            geometryData->indices.push_back(vertexCount);

			int controlPointIndex = pMesh->GetPolygonVertex(iPolygon, iPolygonVertex);
            auto vertex = Vertex(controlPoints[controlPointIndex].mData[0],
                                 controlPoints[controlPointIndex].mData[1],
                                 controlPoints[controlPointIndex].mData[2]);
			geometryData->vertices.push_back(vertex);

            auto normal = getNormal(pMesh, controlPointIndex, vertexCount);
            geometryData->normals.push_back(normal);

			auto uv = getUVCoordinate(pMesh, fbxLayerUV, controlPointIndex, iPolygon, iPolygonVertex);
			geometryData->uvs.push_back(uv);

            if (geometryData->originalIndexVertexMapping.find(controlPointIndex) == geometryData->originalIndexVertexMapping.end())
                geometryData->originalIndexVertexMapping[controlPointIndex] = std::vector<int>();
            geometryData->originalIndexVertexMapping[controlPointIndex].push_back(vertexCount);
         
            vertexCount++;
		}
	}
    geometryData->nPolyVertices = countPolyVerts;
	if (countPolyVerts == 3)
		geometryData->drawMode = 3;
	else
		geometryData->drawMode = 4;
	success = true;
	return geometryData;
}

Normal FBXGeometryLoader::getNormal(fbxsdk::FbxMesh* mesh, int controlPointIndex, int vertexCount) {
    FbxLayer* layer0 = mesh->GetLayer(0);
    FbxLayerElementNormal* normals = layer0->GetNormals();
    float sign = -1;
    Normal normal = Normal();
    auto refMode = normals->GetReferenceMode();
    switch (refMode){
        case FbxGeometryElement::eDirect:
        {
            //auto n = normals->GetDirectArray().GetAt(controlPointIndex).mData;
            auto n = normals->GetDirectArray().GetAt(vertexCount).mData;
            normal = Normal(sign*n[0], sign*n[1], sign*n[2]);
            break;
        }
        case FbxGeometryElement::eIndexToDirect:
        {
            int index = normals->GetIndexArray().GetAt(vertexCount);
            auto n = normals->GetDirectArray().GetAt(index).mData;
            normal = Normal(sign*n[0], sign*n[1], sign*n[2]);
            break;
        }case FbxGeometryElement::eIndex:
        {
            int index = normals->GetIndexArray().GetAt(vertexCount);
            auto n = normals->GetDirectArray().GetAt(index).mData;
            normal = Normal(sign*n[0], sign*n[1], sign*n[2]);
            break;
        }default: {
            std::cout << "Error: unknow reference mode " << refMode  << std::endl;
            break;
        }
       }
     return normal;
}

UVCoord FBXGeometryLoader::getUVCoordinate(FbxMesh* pMesh, FbxGeometryElementUV* fbxLayerUV, int controlPointIndex, int iPolygon, int iPolygonVertex){
	int UVIndex = -1;
	FbxVector2 fbxUV;
	switch (fbxLayerUV->GetMappingMode())
	{
	case FbxLayerElement::eByControlPoint:
		UVIndex = controlPointIndex;
		switch (fbxLayerUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			fbxUV = fbxLayerUV->GetDirectArray().GetAt(UVIndex);
			break;
		case FbxGeometryElement::eIndexToDirect://the indices do not correspond so you have to first look up the index using a mapping
			int id = fbxLayerUV->GetIndexArray().GetAt(UVIndex);
			fbxUV = fbxLayerUV->GetDirectArray().GetAt(id);
			break;
		}
		break;
	case FbxLayerElement::eByPolygonVertex:
		UVIndex = pMesh->GetTextureUVIndex(iPolygon, iPolygonVertex);//, FbxLayerElement::eTextureDiffuse
		switch (fbxLayerUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			fbxUV = fbxLayerUV->GetDirectArray().GetAt(UVIndex);
			break;

		case FbxGeometryElement::eIndexToDirect://for some reason it still means direct access in this case
			/*int id = fbxLayerUV->GetIndexArray().GetAt(UVIndex);
			fbxUV = fbxLayerUV->GetDirectArray().GetAt(id);*/
			fbxUV = fbxLayerUV->GetDirectArray().GetAt(UVIndex);
			break;
		}
		//iUVIndex = pMesh->GetTextureUVIndex(iPolygon, iPolygonVertex, FbxLayerElement::eTextureDiffuse);
		//break;
	}
    return UVCoord(fbxUV[0], fbxUV[1]);

}

//source: http://www.gamedev.net/topic/577127-fbx-sdkprolem-with-getting-coords-and-normals/
GeometryData* FBXGeometryLoader::createColoredGeometryDataFromMesh(FbxMesh* pMesh, bool& success){
	
	GeometryData* geometryData = new GeometryData();
	FbxVector4* fbxControlPoints = pMesh->GetControlPoints();
	for (int i = 0; i < pMesh->GetControlPointsCount(); i++)
	{
		geometryData->vertices.push_back(Vertex(fbxControlPoints[i].mData[0],
			fbxControlPoints[i].mData[1],
			fbxControlPoints[i].mData[2]));
	}

	for (int iPolygon = 0; iPolygon < pMesh->GetPolygonCount(); iPolygon++) {
		for (unsigned iPolygonVertex = 0; iPolygonVertex < 3; iPolygonVertex++) {
			int fbxCornerIndex = pMesh->GetPolygonVertex(iPolygon, iPolygonVertex);
			geometryData->indices.push_back(fbxCornerIndex);

		}
	}

	geometryData->shaderName = "color";
	geometryData->drawMode = 3;
	Color color = Color(1, 0, 0, 1);
	for (unsigned short i = 0; i < geometryData->vertices.size(); i++){
		geometryData->colors.push_back(color);
	}

	success = true;
	return geometryData;
}
void convertToOpenGLCoordinateSystem(FbxAMatrix& input){
	FbxVector4 translation = input.GetT();
	FbxVector4 rotation = input.GetR();
	translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]); // This negate Z of Translation Component of the matrix
	rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]); // This negate X,Y of Rotation Component of the matrix
	// These 2 lines finally set "input" to the eventual converted result
	input.SetT(translation);
	input.SetR(rotation);
}


bool FBXGeometryLoader::extractAnimations(FbxNode* node, GeometryDataList* geometryData){
	//https://github.com/gameplay3d/GamePlay/blob/master/tools/encoder/src/FBXSceneEncoder.cpp
	//http://oddeffects.blogspot.de/2013/10/fbx-sdk-tips.html
	for (int i = 0; i < fbxScene->GetSrcObjectCount<FbxAnimStack>(); i++){
		FbxAnimStack* currAnimStack = fbxScene->GetSrcObject<FbxAnimStack>(i); 
		fbxScene->SetCurrentAnimationStack(currAnimStack);
		FbxString animStackName = currAnimStack->GetName();
		std::string animationName = animStackName.Buffer();
		FbxTakeInfo* takeInfo = fbxScene->GetTakeInfo(animStackName);
		FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
		FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
		int numFrames = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
		geometryData->animations[animationName] = JointFramesMap();
		float duration = end.GetSecondCount() - start.GetSecondCount();
		geometryData->animations[animationName].frameTime = duration / numFrames;
		//prepare joint frame map
		for (auto it = geometryData->skeleton->joints.begin(); it != geometryData->skeleton->joints.end(); it++){
			geometryData->animations[animationName].frames[it->first] = JointFrames();
		}
		//fill joint frame map
		/*unsigned int numOfClusters = currSkin->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex <numOfClusters; clusterIndex++){
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			std::string name = currCluster->GetLink()->GetName();
			if (geometryData->animations[animationName].frames.find(name) != geometryData->animations[animationName].frames.end()){
				//if (name == skeleton->root)
				//geometryData->animations[animationName].frames[name].channels = { "Xtranslation", "Ytranslation", "Ztranslation", "Xrotation", "Yrotation", "Zrotation" };
				//else
				//	geometryData->animations[animationName].frames[name].channels = {  "Xrotation", "Yrotation", "Zrotation" };
				FbxTime currTime;
				for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
				{
					//http://forums.autodesk.com/t5/fbx-sdk/what-am-i-doing-with-animation/td-p/4249593
					currTime.SetFrame(i, FbxTime::eFrames24);
					FbxAMatrix localTransform = currCluster->GetLink()->EvaluateLocalTransform(currTime);
					//convertToOpenGLCoordinateSystem(localTransform);
					FbxQuaternion q = localTransform.GetQ();
					//FbxVector4 localEulerAngles = localTransform.GetR();
					FbxVector4 localTranslation = localTransform.GetT();
					//FbxVector4 localEulerAngles = currCluster->GetLink()->EvaluateLocalRotation(currTime);
					//FbxVector4 localTranslation = currCluster->GetLink()->EvaluateLocalTranslation(currTime);

					//geometryData->animations[animationName].frames[name].localEulerAngles.push_back(glm::vec3(localEulerAngles[0], localEulerAngles[1], localEulerAngles[2]));
					geometryData->animations[animationName].frames[name].localTranslation.push_back(glm::vec3(localTranslation[0], localTranslation[1], localTranslation[2]));
					geometryData->animations[animationName].frames[name].localQuaternions.push_back(glm::quat(q[3], q[0], q[1], q[2]));
				}
			}
		}*/
	}
	return true;
}

void processSkin(FbxSkin* currSkin, FbxAMatrix& geometryTransform, GeometryData* geometryData) {
	Skeleton* skeleton = geometryData->skeleton;
	int numVertices = geometryData->vertices.size();
	std::cout << "Extract weights for " << numVertices << "vertices" << std::endl;
	// create empty joint weights
	for (unsigned int vertexIndex = 0; vertexIndex < numVertices; vertexIndex++) {
		geometryData->jointWeights.push_back(VertexJointData());
		//weights.push_back(false);
	}
	int numOfClusters = currSkin->GetClusterCount();
	for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; clusterIndex++) {
		FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);

		FbxNode* jnode = currCluster->GetLink();
		std::string name = jnode->GetName();

		if (skeleton->joints.find(name) == skeleton->joints.end()) {
			//  Log::write((std::string)"skip " + name);
			continue;
		}
		//TODO move inverse bind pose to skeleton construction
		FbxAMatrix transformMatrix;
		FbxAMatrix transformLinkMatrix;
		FbxAMatrix inverseBindPose;

		currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
		currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
		inverseBindPose = transformLinkMatrix.Inverse()*transformMatrix*geometryTransform;
		FbxQuaternion inverseQuat = inverseBindPose.GetQ();
		FbxVector4 inverseTranslation = inverseBindPose.GetT();
		auto m = glm::toMat4(glm::quat(inverseQuat[3], inverseQuat[0], inverseQuat[1], inverseQuat[2]));

		skeleton->joints[name]->invBindPose = m;
		skeleton->joints[name]->invBindPose[3][0] = inverseTranslation[0];
		skeleton->joints[name]->invBindPose[3][1] = inverseTranslation[1];
		skeleton->joints[name]->invBindPose[3][2] = inverseTranslation[2];
		//extract weights
		int jointIndex = skeleton->joints[name]->index;//get joint index from joint order

		auto cluster_weights = currCluster->GetControlPointWeights();
		for (unsigned int i = 0; i < currCluster->GetControlPointIndicesCount(); i++)
		{
			int controlPointIndex = currCluster->GetControlPointIndices()[i];
			auto vertexList = geometryData->originalIndexVertexMapping[controlPointIndex];
			for (auto it = vertexList.begin(); it != vertexList.end(); it++) {
				geometryData->jointWeights[*it].addJointWeight(jointIndex, cluster_weights[i]);
			}
		}
	}
}

bool FBXGeometryLoader::extractSkeletonWeightsFromMeshNode(FbxMesh* mesh, GeometryData* geometryData){
	//based on http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/how-to-work-with-fbx-sdk-r3582

	mesh->GetNode()->GetTransform();
	const FbxVector4 lT = mesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = mesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = mesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix geometryTransform = FbxAMatrix(lT, lR, lS);
	int deformerCounter = mesh->GetDeformerCount();
	if (deformerCounter > 0){
		int deformerIndex = 0;// only look at the first deformer
		// There are many types of deformers in Maya,
		// We are using only skins, so we see if this is a skin
		FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (currSkin)
		{
			processSkin(currSkin, geometryTransform, geometryData);
	
        } else {
            std::cout << "Did not find weights" << std::endl;
        }
    } else {
        std::cout << "No deformer defined" << std::endl;
    }

	for (auto it = geometryData->jointWeights.begin(); it != geometryData->jointWeights.end(); it++){
		it->normalize();
	}
	return true;
}


GeometryData* FBXGeometryLoader::extractGeometryDataFromNodeAttribute(FbxNode* node, int attributeIndex, bool& success){
	std::vector<std::string> textureFileNames = std::vector<std::string>();
	std::vector<std::string> textureNames = std::vector<std::string>();
	extractTextureNamesFromNode(node, textureFileNames);
	for (int i = 0; i < textureFileNames.size(); i++){
		size_t pos = textureFileNames[i].find_last_of("\\/");
		std::string name = (std::string::npos == pos) ? textureFileNames[i] : textureFileNames[i].substr(pos + 1, std::string::npos);
		textureNames.push_back(name);
	}

	FbxMesh* mesh = (FbxMesh*)node->GetNodeAttributeByIndex(attributeIndex);
	if (!mesh->IsTriangleMesh()){
		mesh = (FbxMesh*)geometryConverter->Triangulate((FbxNodeAttribute*)mesh, true);
	}
	GeometryData* geometryData = NULL;
	if (textureNames.size()> 0){
		geometryData = createGeometryDataFromMesh(mesh, success);
		geometryData->textureName = textureNames[0];
        geometryData->texturePath = textureFileNames[0];
		//geometryData->flipYandZ();
		//geometryData->flipUVCoords();
		//geometryData->scale(0.1);
		
	}
	else{
		geometryData = createColoredGeometryDataFromMesh(mesh, success);
	}

	return geometryData;
}



void FBXGeometryLoader::extractSkeletonFromNode(FbxNode* node, GeometryDataList* geometryDataList, int level) {
	if (!node) return;
    bool foundSkeleton = false;
    auto attribute = node->GetNodeAttribute();
    if (attribute){
        int attributeCount = node->GetNodeAttributeCount();
        std::string name = node->GetName();
        for (int i = 0; i < attributeCount; i++) {
            FbxNodeAttribute::EType attributeType = node->GetNodeAttributeByIndex(i)->GetAttributeType();
         
            bool isCustomRoot = name.find("FK_back1_jnt") != std::string::npos;
            bool isRoot = attributeType == FbxNodeAttribute::EType::eSkeleton || isCustomRoot;
            if (geometryDataList->skeleton == NULL && isRoot) { // &&  node->GetChildCount() > 0 && level > 0

                //http://stackoverflow.com/questions/13566608/loading-skinning-information-from-fbx
                geometryDataList->skeleton = new Skeleton();
                extractSkeletonDataNodeHierarchyRecursively(node, geometryDataList->skeleton, std::string(""), 0);
                geometryDataList->skeleton->root = name;
            }
        }
    }

    if(!foundSkeleton){
        for (int i = 0; i < node->GetChildCount(); i++) extractSkeletonFromNode(node->GetChild(i), geometryDataList, level + 1);
    }
   
}

void FBXGeometryLoader::extractMeshListFromNode(FbxNode* node, GeometryDataList* geometryDataList, int level){
	if (!node) return;
	
      
	bool foundGeometry = false;
	GeometryData * geometry = NULL;
	if (node->GetNodeAttribute() != NULL){

        //check for mesh
		int attributeCount = node->GetNodeAttributeCount();
        int meshAttributeIndex = -1;
		for (int i = 0; i < attributeCount; i++){
			FbxNodeAttribute::EType lAttributeType = node->GetNodeAttributeByIndex(i)->GetAttributeType();
            if (lAttributeType == FbxNodeAttribute::eMesh){
                meshAttributeIndex = i;
                std::cout << "found mesh" << std::endl;
                break;
                    
            }
		}

        //extract data
		if (meshAttributeIndex != -1){
            geometry = extractGeometryDataFromNodeAttribute(node, meshAttributeIndex, foundGeometry);
            geometry->skeleton = geometryDataList->skeleton;
            //  Log::write((std::string)"extract weights from mesh in attribute " + node->GetName());
            if (geometry->skeleton != NULL) {
                FbxMesh* mesh = (FbxMesh*)node->GetNodeAttributeByIndex(meshAttributeIndex);
                extractSkeletonWeightsFromMeshNode(mesh, geometry);
            }
            geometryDataList->meshList.push_back(geometry);
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		extractMeshListFromNode(node->GetChild(i), geometryDataList, level+1);
	}
}


bool FBXGeometryLoader::loadGeometryDataFromFile(const char* path, GeometryDataList* geometryDataList){
	// Prepare the FBX SDK.
	lSdkManager = FbxManager::Create();
	if (!lSdkManager){
		std::cout << "Unable to create FBX Manager" << std::endl;
		return false;
	}
	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);
	fbxScene = FbxScene::Create(lSdkManager, "My Scene");

	FbxImporter* Importer = FbxImporter::Create(lSdkManager, "");
	int lFileFormat = -1;
	Importer = FbxImporter::Create(lSdkManager, "");
	if (!lSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(path, lFileFormat))
	{
		// Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
		lFileFormat = lSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
	}

	if (!Importer->Initialize(path, lFileFormat)) {
		std::cout << "Unable to initialize FBX Importer using file " << path << std::endl;
		return false;
	}

	if (!Importer->Import(fbxScene)){
		std::cout << "Unable to create scene from importer" << std::endl;
		return false;
	}

	// Convert scene to OpenGL
	geometryConverter = new FbxGeometryConverter(lSdkManager);
	//Parse the scene node hiearachy to extract meshes and skeletons
	FbxNode* root = fbxScene->GetRootNode();
    extractSkeletonFromNode(root, geometryDataList, 0);
    std::cout << "loaded skeleton" <<geometryDataList->skeleton->joints.size() << std::endl;
    extractMeshListFromNode(root, geometryDataList, 0);
    //extractAnimations(root, geometryDataList );

	// Destroy all objects created by the FBX SDK.
	if (lSdkManager) lSdkManager->Destroy();
	return geometryDataList->meshList.size()>0;
}


