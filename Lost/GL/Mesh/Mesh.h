#pragma once
#include <vector>
#include "../Vector.h"
#include "../Texture/Material.h"

// Mesh rendering method, given to OpenGL
enum MeshRenderMethod
{ 
	// Renders each vertex as a point
	// [a], [b], [c] = o  o  o
	// Equivalent of GL_POINTS
	LOST_MESH_POINTS,

	// Renders every pair of verticies as a seperate line 
	// [a, b], [c, d] = o--o  o--o
	// Equivalent of GL_LINES
	LOST_MESH_LINES,

	// Renders every pair of verticies as a seperate line but joins the first and last verticies given
	// [a, b], [c, d] then (d, a) is added by the renderer
	// Equivalent of GL_LINE_LOOP
	LOST_MESH_LINE_LOOP,

	// Renders a line of verticies connecting each one given with no seperation, excluding the first and last vertex
	// [a, b, c, d] = o--o--o--o
	// Equivalent of GL_LINE_STRIP
	LOST_MESH_LINE_STRIP,

	// Renders a triangle for every three verticies given, this has seperation
	// [a, b, c], [d, e, f]
	// Equivalent of GL_TRIANGLES
	LOST_MESH_TRIANGLES,

	// Renders a strip of triangles, the first three given form a triangle, then any further vertex added creates a new triangle using the last two as it's base
	// Eg. Given the vertex list A, B, C, D, E, these triangles would be formed: [A, B, C], [B, C, D], [C, D, E]
	// Equivalent of GL_TRIANGLE_STRIP
	LOST_MESH_TRIANGLE_STRIP,

	// Renders a strip of triangles, the first three given form a triangle, then any further vertex added creates a new triangle using the first vertex given and the last vertex given as it's base
	// Eg. Given the vertex list A, B, C, D, E, these triangles would be formed: [A, B, C], [A, C, D], [A, D, E]
	// They all share A, the first vertex given, but they use the last vertex given as the second vertex
	// Equivalent of GL_TRIANGLE_FAN
	LOST_MESH_TRIANGLE_FAN
};

namespace lost
{

	// A reference to a mesh
	typedef void* Mesh;

	struct CompiledMeshData
	{
		std::vector<float> vectorData;
		std::vector<unsigned int> materialSlotIndicies;
		std::vector<unsigned int> indexData;
		unsigned int meshRenderMode = LOST_MESH_TRIANGLES;
	};

	union Vertex {
		struct
		{
			Vec3 position;
			Vec2 textureCoord;
			Vec4 vertexColor;
			Vec3 vertexNormal;
		};
		float data[12];
	};

	struct MeshData
	{
		// The verticies of the mesh, stored as { X, Y, Z, U, V, R, G, B }
		std::vector<Vertex> verticies; 

		// The indicies which form the mesh using the render method given, by default LOST_MESH_TRIANGLES
		// Set "meshFaceMethod" to change this
		std::vector<unsigned int> indexArray;

		// The indicies which each material uses, used to split up the mesh into it's seperate parts
		std::vector<unsigned int> materialSlotIndicies; // The index which the material slot takes over when rendering faces

		// The method which the render will use to fill in the faces, by default LOST_MESH_TRIANGLES
		// Follows the MeshRenderMethod enum
		unsigned int meshFaceMethod = LOST_MESH_TRIANGLES; // [!] TODO: Make this actually do stuff in the renderer
	};

}