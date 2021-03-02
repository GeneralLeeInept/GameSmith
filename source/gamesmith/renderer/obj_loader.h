#pragma once

namespace GameSmith
{

struct ObjVertex
{
    float x, y, z, w;
};

struct ObjTexCoord
{
    float u, v, w;
};

struct ObjNormal
{
    float x, y, z;
};

struct ObjTri
{
    int32_t v[3];
    int32_t t[3];
    int32_t n[3];
};

struct ObjFile
{
    std::vector<ObjVertex> vertices;
    std::vector<ObjTexCoord> texCoords;
    std::vector<ObjNormal> normals;
    std::vector<ObjTri> triangles;

    bool Load(const std::string& path);
};

}