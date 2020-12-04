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

#ifndef RAYLIB_EXTRAS_H
#define RAYLIB_EXTRAS_H

#include "raylib.h"															// Required for structs: Vector3, Matrix

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

RLAPI void FreeData(void* data);                                            // deallocates memory allocated by raylib

RLAPI void UnloadModelNoMesh(Model model);                                  // Unload model from memory but leave the meshes(RAM and/or VRAM)

RLAPI void AllocateMeshData(Mesh* mesh, int verts, int triangles, int colors, bool uvs, bool uv2s);	// takes an empty mesh and allocates buffers for the required sizes

typedef void (*MeshGenerateCallback)(Mesh* mesh, void* userData);
RLAPI Mesh GenMeshCustom(MeshGenerateCallback generate, void* userData);     // Generate cubes-based map mesh from image data


RLAPI void DrawVector3Text(Vector3* vector, int posX, int posY, bool rightJustify);
RLAPI void DrawVector2Text(Vector2* vector, int posX, int posY, bool rightJustify);

RLAPI void DrawLine3DF(float spx, float spy, float spz, float epx, float epy, float epz, Color color);
RLAPI void DrawRect3DXZ(Rectangle rect, float y, Color color);
RLAPI void DrawCircleF(float centerX, float centerY, float radius, Color color);
RLAPI void DrawLineF(float startPosX, float startPosY, float endPosX, float endPosY, Color color);
RLAPI void DrawRectangleF(float posX, float posY, float width, float height, Color color);


typedef struct Vector2i
{
    int x;
    int y;
 } Vector2i;

#if defined(__cplusplus)
}
#endif


#endif  // RAYLIB_EXTRAS_H
