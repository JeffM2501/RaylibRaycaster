/**********************************************************************************************
*
*   raylibExtras v0.1 - Extra functions for more advanced uses of raylib primitives
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2015-2020 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/


#include "raylibExtras.h"
#include "utils.h"
#include "rlgl.h"
#include <stdlib.h>         // Required for: malloc(), free()

void FreeData(void* data)
{
	if (data != NULL)
		RL_FREE(data);
}

void UnloadModelNoMesh(Model model)
{
	// As the user could be sharing shaders and textures between models,
	// we don't unload the material but just free it's maps, the user
	// is responsible for freeing models shaders and textures
	for (int i = 0; i < model.materialCount; i++) RL_FREE(model.materials[i].maps);

	RL_FREE(model.meshes);
	RL_FREE(model.materials);
	RL_FREE(model.meshMaterial);

	// Unload animation data
	RL_FREE(model.bones);
	RL_FREE(model.bindPose);

	TRACELOG(LOG_INFO, "MODEL: Unloaded model(no meshes) from RAM and VRAM");
}

#ifndef DEFAULT_MESH_VERTEX_BUFFERS
#define DEFAULT_MESH_VERTEX_BUFFERS    7    // Number of vertex buffers (VBO) per mesh
#endif

void AllocateMeshData(Mesh* mesh, int verts, int triangles, int colors, bool uvs, bool uv2s)
{
	mesh->vboId = (unsigned int*)RL_CALLOC(DEFAULT_MESH_VERTEX_BUFFERS, sizeof(unsigned int));

	mesh->vertices = (float*)RL_MALLOC(verts * 3 * sizeof(float));
	mesh->normals = (float*)RL_MALLOC(verts * 3 * sizeof(float));

	if (uvs)
		mesh->texcoords = (float*)RL_MALLOC(verts * 2 * sizeof(float));
	else
		mesh->texcoords = NULL;

	if (uv2s)
		mesh->texcoords2 = (float*)RL_MALLOC(verts * 2 * sizeof(float));
	else
		mesh->texcoords2 = NULL;

	mesh->indices = (unsigned short*)RL_MALLOC(triangles * 3 * sizeof(unsigned short));
	mesh->vertexCount = verts;
	mesh->triangleCount = triangles;
}

Mesh GenMeshCustom(MeshGenerateCallback generate, void* userData)
{
	Mesh mesh = { 0 };
	
	if (generate != NULL)   // ask the callback to generate the mesh data
		generate(&mesh, userData);

	// Upload vertex data to GPU (static mesh)
	rlLoadMesh(&mesh, false);

	return mesh;
}

void DrawVector3Text(Vector3* vector, int posX, int posY, bool rightJustify)
{
	const int size = 20;
	const char* text = TextFormat("%.2f,%.2f,%.2f", vector->x, vector->y, vector->z);

	int offset = 0;
	if (rightJustify)
		offset = MeasureText(text, size);
	DrawText(text, posX - offset, posY, 20, LIME);
}

void DrawVector2Text(Vector2* vector, int posX, int posY, bool rightJustify)
{
	const int size = 20;
	const char* text = TextFormat("%.2f,%.2f", vector->x, vector->y);

	int offset = 0;
	if (rightJustify)
		offset = MeasureText(text, size);
	DrawText(text, posX - offset, posY, 20, LIME);
}

void DrawLine3DF(float spx, float spy, float spz, float epx, float epy, float epz, Color color)
{
	rlBegin(RL_LINES);
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlVertex3f(spx, spy, spz);
	rlVertex3f(epx, epy, epz);
	rlEnd();
}

void DrawRect3DXZ(Rectangle rect, float y, Color color)
{
	rlBegin(RL_LINES);
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlVertex3f(rect.x, y, rect.y);
	rlVertex3f(rect.x + rect.width, y, rect.y);

	rlVertex3f(rect.x + rect.width, y, rect.y);
 	rlVertex3f(rect.x + rect.width, y, rect.y + rect.height);

	rlVertex3f(rect.x + rect.width, y, rect.y + rect.height);
 	rlVertex3f(rect.x, y, rect.y + rect.height);
	
	rlVertex3f(rect.x, y, rect.y + rect.height);
	rlVertex3f(rect.x, y, rect.y);
	rlEnd();
}

void DrawCircleF(float centerX, float centerY, float radius, Color color)
{
	DrawCircleV((Vector2) { centerX, centerY }, radius, color);
}

void DrawLineF(float startPosX, float startPosY, float endPosX, float endPosY, Color color)
{
	rlBegin(RL_LINES);
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlVertex2f(startPosX, startPosY);
	rlVertex2f(endPosX, endPosY);
	rlEnd();
}

void DrawRectangleF(float posX, float posY, float width, float height, Color color)
{
	DrawRectangleV((Vector2) { posX, posY }, (Vector2) { width, height }, color);
}
