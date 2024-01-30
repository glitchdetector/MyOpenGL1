#pragma once
#include <string>
#include <vector>

struct Vertex
{
	float x, y, z, w;
	float r, g, b;
};

struct VertexTexture
{
	float u, v, w;
};

struct VertexNormals
{
	float x, y, z;
};

struct VertexParameter
{
	float u, v, w;
};

struct Face
{
	std::vector<Vertex> m_vertices;
	std::vector<VertexTexture> m_vertexTextures;
};

struct Object
{
	std::string name;
	std::vector<Vertex> m_vertices;
	std::vector<VertexTexture> m_vertexTextures;
	std::vector<Face> m_faces;
	bool smoothShading;
};

class ObjHelper
{
};

