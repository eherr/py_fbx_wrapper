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
#ifndef GRAPHIC_TYPES_H_
#define GRAPHIC_TYPES_H_

#include <glm\glm.hpp>

static const unsigned int VERTEX_SIZE = 3;
static const unsigned int RGBA_SIZE = 4;
static const unsigned int UV_SIZE = 2;
static const int NUM_JOINTS_PER_VEREX = 4;

/*typedef glm::vec3 Vertex;*/
class Vertex {
public:
    Vertex() {
        x = 0;
        y = 0;
        z = 0;
    }
    Vertex(glm::vec3& vector) {
        x = vector.x;
        y = vector.y;
        z = vector.z;
    }
    Vertex(glm::vec4& vector) {
        x = vector.x;
        y = vector.y;
        z = vector.z;
    }
    Vertex(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    glm::vec4 toglmVec() {
        return glm::vec4(x, y, z, 1);
    }
    Vertex operator+(const Vertex& other) {
        return Vertex(x + other.x,
            y + other.y,
            z + other.z);
    }
    void operator+=(const Vertex& other) {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    Vertex operator*(const float& factor) {
        return Vertex(x*factor, y*factor, z*factor);
    }

    Vertex operator*(const glm::mat4& matrix) {
        glm::vec4 v = glm::vec4(x, y, z, 1.0);
        v = v * matrix;
        return Vertex(v);
    }

    void operator*=(const float& factor) {
        x *= factor;
        y *= factor;
        z *= factor;

    }
    float x, y, z;        //Vertex
};

class Normal : public Vertex {
public:
    Normal() :Vertex() {};
    Normal(float x, float y, float z) :Vertex(x, y, z) {};

    Normal operator*(const glm::mat4& matrix) {
        glm::vec4 v = glm::vec4(x, y, z, 1.0);
        v = v * matrix;
        return Normal(v.x, v.y, v.z);
    }

};

struct UVCoord {
    UVCoord() {
        u = 0;
        v = 0;
    }
    UVCoord(float u, float v) {
        this->u = u;
        this->v = v;
    }
    void flip() {
        v = 1.0f - v;
    }

    float u, v;
};

struct Color {
    Color() {
        r = 0;
        g = 0;
        b = 0;
        a = 0;
    }
    Color(float r, float g, float b, float a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    float r, g, b, a;
};


struct TexturedVertex : public Vertex, public UVCoord
{
    TexturedVertex(Vertex vertex, UVCoord uvCoord) {
        x = vertex.x;
        y = vertex.y;
        z = vertex.z;
        u = uvCoord.u;
        v = uvCoord.v;
    }
};

struct ColorVertex : public Vertex, public Color
{
    ColorVertex() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
        this->r = 0;
        this->g = 0;
        this->b = 0;
        this->a = 1;
    }
    ColorVertex(float x, float y, float z, float r, float g, float b, float a) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    ColorVertex(float x, float y, float z, Color color) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->r = color.r;
        this->g = color.g;
        this->b = color.b;
        this->a = color.a;
    }
    ColorVertex(Vertex vertex, Color color) {
        this->x = vertex.x;
        this->y = vertex.y;
        this->z = vertex.z;
        this->r = color.r;
        this->g = color.g;
        this->b = color.b;
        this->a = color.a;
    }


};



//from http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
struct VertexJointData
{
    int IDs[NUM_JOINTS_PER_VEREX];
    float Weights[NUM_JOINTS_PER_VEREX];

    VertexJointData() {
        for (int i = 0; i < NUM_JOINTS_PER_VEREX; i++) {
            this->IDs[i] = -1;
            this->Weights[i] = 0.f;
        }
    }

    // adds the vertexId and weight to the next free slot of the struct vertexWeights
    void addJointWeight(int jointId, float jointWeight) {
        for (int i = 0; i < NUM_JOINTS_PER_VEREX; i++) {
            if (this->IDs[i] == -1) {
                this->IDs[i] = jointId;
                this->Weights[i] = jointWeight;
                return;
            }
        }
    }
    float getWeightSum() {
        float sum = 0;
        for (int i = 0; i < NUM_JOINTS_PER_VEREX; i++) {
            if (this->IDs[i] != -1) {
                sum += this->Weights[i];
            }
        }
        return sum;
    }

    void normalize() {
        float sum = getWeightSum();
        for (int i = 0; i < NUM_JOINTS_PER_VEREX; i++) {
            if (this->IDs[i] != -1) {
                this->Weights[i] /= sum;
            }
        }
    }
};


struct ColorSkinningVertex : public ColorVertex, public VertexJointData {

    ColorSkinningVertex(Vertex vertex, Color color, VertexJointData jointWeight) :ColorVertex(vertex, color) {
        for (int i = 0; i < NUM_JOINTS_PER_VEREX; i++) {
            if (jointWeight.IDs[i] >= 0) {
                IDs[i] = jointWeight.IDs[i];
                Weights[i] = jointWeight.Weights[i];
            }
            else {
                IDs[i] = 0;
                Weights[i] = 0;

            }
        }
    }
};

struct TexturedSkinningVertex : public TexturedVertex, public VertexJointData {

    TexturedSkinningVertex(Vertex vertex, UVCoord uv, VertexJointData jointWeight) :TexturedVertex(vertex, uv) {
        /*for (int i = 0; i < NUM_JOINTS_PER_VEREX; i++){
        IDs[i] = 0;
        Weights[i] = 0.0;
        }
        IDs[0] = 0;
        Weights[0] = 1.0;*/
        for (int i = 0; i < NUM_JOINTS_PER_VEREX; i++) {
            if (jointWeight.IDs[i] >= 0) {
                IDs[i] = jointWeight.IDs[i];
                Weights[i] = jointWeight.Weights[i];
            }
            else {
                IDs[i] = 0;
                Weights[i] = 0;

            }
        }
    }
};


class Ray {
public:
    Ray(glm::vec3& start, glm::vec3& dir) {
        this->start = start;
        this->dir = dir;
    }
    glm::vec3 start;
    glm::vec3 dir;
};

struct Intersection {
    float distance;
    glm::vec3 position;
    unsigned int object_id;
};


#endif //GRAPHIC_TYPES_H_