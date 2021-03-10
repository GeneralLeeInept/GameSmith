#include "gspch.h"

#include "obj_loader.h"

#include <gamesmith.h>

namespace gs
{

inline float ReadFloat(const char*& c, float d)
{
    for (; *c == ' '; ++c);
    if (!*c) return d;

    float sign = 1.0f;
    float scale = 1.0f;
    float v = 0.0f;

    if (*c == '-')
    {
        sign = -1.0f;
        ++c;
    }

    for (; *c && *c != ' ' && *c != '.'; ++c)
    {
        v = v * 10.f + float(*c - '0');
    }

    if (*c == '.')
    {
        for (++c; *c && *c != ' '; ++c)
        {
            v = v * 10.f + float(*c - '0');
            scale *= 10.f;
        }
    }

    return sign * v / scale;
}

inline void ReadVertexIndices(const char*& c, int32_t& vi, int32_t& vti, int32_t& vni)
{
    for (; *c == ' '; ++c);
    if (!*c) return;

    vi = 0;
    for (; *c && *c != '/'; ++c)
    {
        vi = vi * 10 + int32_t(*c - '0');
    }

    if (*c == '/')
    {
        if (c[1] != '/')
        {
            vti = 0;
        }

        for (++c; *c && *c != '/'; ++c)
        {
            vti = vti * 10 + int32_t(*c - '0');
        }
    }

    if (*c == '/')
    {
        vni = 0;

        for (++c; *c && *c != ' '; ++c)
        {
            vni = vni * 10 + int32_t(*c - '0');
        }
    }
}

bool ObjFile::Load(const std::string& path)
{
    std::ifstream fs(path, std::ios::in);
    std::string s;

    while (std::getline(fs, s))
    {
        if (s[0] == 'v')
        {
            if (s[1] == ' ')
            {
                // Geometric vertex
                ObjVertex vertex{};
                const char* p = &s[2];
                vertex.x = ReadFloat(p, 0.f);
                vertex.y = ReadFloat(p, 0.f);
                vertex.z = ReadFloat(p, 0.f);
                vertex.w = ReadFloat(p, 1.f);
                vertices.push_back(vertex);
            }
            else if (s[1] == 't')
            {
                // Texture coordinate
                ObjTexCoord texCoord{};
                const char* p = &s[3];
                texCoord.u = ReadFloat(p, 0.f);
                texCoord.v = ReadFloat(p, 0.f);
                texCoord.w = ReadFloat(p, 0.f);
                texCoords.push_back(texCoord);
            }
            else if (s[1] == 'n')
            {
                // Vertex normal
                ObjNormal normal{};
                const char* p = &s[3];
                normal.x = ReadFloat(p, 0.f);
                normal.y = ReadFloat(p, 0.f);
                normal.z = ReadFloat(p, 0.f);
                normals.push_back(normal);
            }
            // else - parameter space vertex, ignore
        }
        else if (s[0] == 'f')
        {
            // Polygonal face
            // TODO: Just assuming triangles here, could assert or.. 
            ObjTri tri{};
            const char* p = &s[2];
            ReadVertexIndices(p, tri.v[0], tri.t[0], tri.n[0]);
            ReadVertexIndices(p, tri.v[1], tri.t[1], tri.n[1]);
            ReadVertexIndices(p, tri.v[2], tri.t[2], tri.n[2]);
            triangles.push_back(tri);
        }
        // else - ignore everything else
    }

    return true;
}

} // namespace GameSmith
