#
# Copyright 2019 DFKI GmbH.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so, subject to the
# following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
# NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
# USE OR OTHER DEALINGS IN THE SOFTWARE.

# distutils: language = c++

from libcpp cimport bool
import numpy as np
from libcpp.map cimport map
from libcpp.string cimport string
from cython.operator cimport dereference as deref, preincrement as inc

#cdef extern from "<string>" namespace "std":
#    cdef cppclass string:
#        string() except +


cdef extern from "<vector>" namespace "std":
    cdef cppclass vector[T]:
        cppclass iterator:
            T operator*()
            iterator operator++()
            bint operator==(iterator)
            bint operator!=(iterator)
        vector()
        void push_back(T&)
        T& operator[](int)
        T& at(int)
        iterator begin()
        iterator end()
        int size()

cdef extern from "glm/vec3.hpp" namespace "glm":
    cdef cppclass vec3:
        vec3() except +
        vec3(double, double, double) except +
        double x
        double y
        double z

cdef extern from "glm/gtc/quaternion.hpp" namespace "glm":
    cdef cppclass quat:
        quat() except +
        quat(double w, double x, double y, double z) except +
        double w
        double x
        double y
        double z


cdef extern from "glm/matrix.hpp" namespace "glm":
    cdef cppclass mat4:
        mat4() except +
        mat4(float &m) except +
        double* operator[](int)

cdef extern from "joint.h":
    cdef cppclass Joint:
        Joint() except +
        string name
        int index
        vec3 offset
        quat rotation
        mat4 invBindPose
        vector[Joint*] children

cdef extern from "joint_frames.h":
    cdef struct JointFrames:
        vector[vec3] localTranslation
        vector[vec3] localEulerAngles
        vector[string] channels
        vector[quat] localQuaternions

    cdef struct JointFramesMap:
        map[string, JointFrames] frames
        float frameTime

cdef extern from "skeleton.h":
    cdef cppclass Skeleton:
        Skeleton() except +
        map[string, Joint*] joints
        vector[string] jointOrder
        double frameTime
        string root

cdef extern from "graphic_types.h":
    cdef struct Vertex:
        Vertex() except +
        float x
        float y
        float z

    cdef struct Normal:
        Normal() except +
        float x
        float y
        float z

    cdef struct Color:
        Color() except +
        float r
        float g
        float b
        float a

    cdef struct UVCoord:
        UVCoord() except +
        float u
        float v

    cdef struct VertexJointData:
        VertexJointData() except +
        int IDs[4]
        float Weights[4]

cdef extern from "geometry_data.h":
    cdef cppclass GeometryData:
        GeometryData() except +
        vector[Vertex] vertices
        vector[Normal] normals
        vector[unsigned short] indices
        vector[Color] colors
        vector[UVCoord] uvs
        vector[VertexJointData] jointWeights
        string texturePath
        int nPolyVertices
        Skeleton skeleton
        
    cdef cppclass GeometryDataList:
        GeometryDataList() except +
        vector[GeometryData*] meshList
        Skeleton* skeleton
        map[string, JointFramesMap] animations

cdef extern from "fbx_geometry_loader.h":
    cdef cppclass FBXGeometryLoader:
        FBXGeometryLoader() except +
        bool loadGeometryDataFromFile(const char* path, GeometryDataList* geometryDataList)


cdef mat4_to_numpy(mat4& m):
   return np.array( [[m[0][0], m[0][1], m[0][2], m[0][3]],
                     [m[1][0], m[1][1], m[1][2], m[1][3]], 
                     [m[2][0], m[2][1], m[2][2], m[2][3]],
                     [m[3][0], m[3][1], m[3][2], m[3][3]]], dtype=np.float).T

cdef convert_joint_to_dict(Joint* j):
    joint_dict = dict()
    if j.index == 0:
        joint_dict["node_type"] = 0 # root
        joint_dict["channels"] = ["Xposition", "Yposition", "Zposition", "Xrotation", "Yrotation", "Zrotation"]
    else:
        if j.children.size() >0:
            node_type = 1 # joint 
            joint_dict["channels"] = ["Xrotation", "Yrotation", "Zrotation"]
        else:
            node_type = 2 # end site
            joint_dict["channels"] = list()
    joint_dict["node_type"] = node_type
    joint_dict["index"] = j.index
    joint_dict["fixed"] = False
    joint_dict["rotation"] = [j.rotation.w, j.rotation.x, j.rotation.y, j.rotation.z]
    joint_dict["offset"] = [j.offset.x, j.offset.y,j.offset.z]
    joint_dict["inv_bind_pose"] = mat4_to_numpy(j.invBindPose)
    joint_dict["children"] = list()
    n_children = j.children.size()
    for i in range(n_children):
        name = j.children.at(i).name.decode("utf-8")
        joint_dict["children"].append(name)
    return joint_dict

cdef convert_skeleton_to_dict(GeometryDataList* data_list):
    skeleton_dict = dict()
    cdef Skeleton* s = data_list.skeleton;
    skeleton_dict["frame_time"] = s.frameTime;
    skeleton_dict["root"] = s.root.decode("utf-8")
    skeleton_dict["nodes"] = dict()
    cdef map[string, Joint*].iterator it = s.joints.begin()
    while it != s.joints.end():
        name = deref(it).second.name.decode("utf-8")
        skeleton_dict["nodes"][name] = convert_joint_to_dict(deref(it).second)
        inc(it)
    
    skeleton_dict["animated_joints"] = list()
    for i in range(s.jointOrder.size()):
        name = s.jointOrder.at(i).decode("utf-8")
        skeleton_dict["animated_joints"].append(name)
    return skeleton_dict

cdef convert_mesh_data_to_dict(GeometryData*& data):
    mesh_data = dict()
    mesh_data["texture"] = data.texturePath
    if data.nPolyVertices == 4:
        mesh_data["type"] = "quads"
    else:
        mesh_data["type"] = "triangles"
    mesh_data["indices"] = list()
    for j in range(data.indices.size()):
        idx = data.indices.at(j)
        mesh_data["indices"].append(idx)

    mesh_data["vertices"] = list()
    for j in range(data.vertices.size()):
        x = data.vertices.at(j).x
        y = data.vertices.at(j).y
        z = data.vertices.at(j).z
        mesh_data["vertices"].append([x,y,z])
            
    mesh_data["normals"] = list()
    for j in range(data.normals.size()):
        x = -data.normals.at(j).x
        y = -data.normals.at(j).y
        z = -data.normals.at(j).z
        mesh_data["normals"].append([x,y,z])

    mesh_data["texture_coordinates"] = list()
    for j in range(data.uvs.size()):
        u = data.uvs.at(j).u
        v = data.uvs.at(j).v
        mesh_data["texture_coordinates"].append([u,v])

    mesh_data["colors"] = list()
    for j in range(data.colors.size()):
        r = data.colors.at(j).r
        g = data.colors.at(j).g
        b = data.colors.at(j).b
        a = data.colors.at(j).a
        mesh_data["colors"].append([r,g,b,a])

    mesh_data["weights"] = list()
    for j in range(data.jointWeights.size()):
        entry = ([data.jointWeights.at(j).IDs[0], data.jointWeights.at(j).IDs[1], data.jointWeights.at(j).IDs[2], data.jointWeights.at(j).IDs[3]],
                  [data.jointWeights.at(j).Weights[0], data.jointWeights.at(j).Weights[1], data.jointWeights.at(j).Weights[2], data.jointWeights.at(j).Weights[3]] )
        mesh_data["weights"].append(entry)
    return mesh_data

cdef convert_joint_frames_to_list(JointFrames& joinFrames):
    frames = list()
    for i in range(joinFrames.localTranslation.size()):
        frame = dict()
        tx = joinFrames.localTranslation[i].x
        ty = joinFrames.localTranslation[i].y
        tz = joinFrames.localTranslation[i].z
        frame["local_translation"] = [tx, ty, tz]
        qx = joinFrames.localQuaternions[i].x
        qy = joinFrames.localQuaternions[i].y
        qz = joinFrames.localQuaternions[i].z
        qw = joinFrames.localQuaternions[i].w
        frame["local_rotation"] = [qw, qx, qy, qz]
        frames.append(frame)
    return frames

cdef convert_animation_to_dict(JointFramesMap& jointFramesMap):
    animation = dict()
    animation["curves"] = dict()
    animation["frame_time"] = jointFramesMap.frameTime
    cdef map[string, JointFrames].iterator it = jointFramesMap.frames.begin()
    while it != jointFramesMap.frames.end():
        name = deref(it).first.decode("utf-8")
        animation["curves"][name] = convert_joint_frames_to_list(deref(it).second)
        inc(it)
    return animation

cdef convert_mesh_data_list_to_dict(GeometryDataList* data_list):
    mesh_data = dict()
    mesh_list = list()
    for i in range(data_list.meshList.size()):
        mesh = convert_mesh_data_to_dict(data_list.meshList.at(i))
        mesh_list.append(mesh)
    
    mesh_data["skeleton"] = convert_skeleton_to_dict(data_list)
    mesh_data["mesh_list"] = mesh_list
    print("mesh_list", len(mesh_list), data_list.meshList.size())
    mesh_data["animations"] = dict()
    cdef map[string, JointFramesMap].iterator it = data_list.animations.begin()
    while it != data_list.animations.end():
        name = deref(it).first.decode("utf-8")
        mesh_data["animations"][name] = convert_animation_to_dict(deref(it).second)
        inc(it)
    return mesh_data

def load_fbx_file(filename):
    cdef char* f = filename
    cdef GeometryDataList* data = new GeometryDataList()
    cdef FBXGeometryLoader* loader = new FBXGeometryLoader()
    cdef bool success = loader.loadGeometryDataFromFile(f, data)
    #del data
    #del loader
    if success:
        return convert_mesh_data_list_to_dict(data)
