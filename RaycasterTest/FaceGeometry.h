#pragma once
#include "raylib.h"
#include "raylibExtras.h"
#include "raymath.h"
#include <cstdint>
#include <memory>

class CellParams
{
public:
    float mapX = 0;
    float mapY = 0;

    float top = 0;
    float bottom = 0;
};

inline void GenCeilingMesh(Mesh* mesh, void* userData)
{
    CellParams* data = static_cast<CellParams*>(userData);
    if (data == nullptr)
        return;

    Vector3 minOrigin{ data->mapX, data->top, data->mapY };
	Vector3 maxOrigin{ data->mapX+1, data->top, data->mapY + 1 };

    float vertices[] =
    {
        minOrigin.x,minOrigin.y,minOrigin.z,
        maxOrigin.x,minOrigin.y,minOrigin.z,
        minOrigin.x,minOrigin.y,maxOrigin.z,
        maxOrigin.x,minOrigin.y,maxOrigin.z,
    };

    float texcoords[] =
    {
        1,0,
        0,0,
        1,1,
        0,1,
    };

    float normals[] =
    {
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
    };

    AllocateMeshData(mesh, 4, 2, 0, true, false);
    memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
    memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
    memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

    mesh->indices[0] = 0;
    mesh->indices[1] = 1;
    mesh->indices[2] = 2;

    mesh->indices[3] = 1;
    mesh->indices[4] = 3;
    mesh->indices[5] = 2;

    mesh->vertexCount = 4;
    mesh->triangleCount = 2;
}

inline void GenFloorMesh(Mesh* mesh, void* userData)
{
	CellParams* data = static_cast<CellParams*>(userData);
	if (data == nullptr)
		return;

	Vector3 minOrigin{ data->mapX, data->bottom, data->mapY };
	Vector3 maxOrigin{ data->mapX + 1, data->bottom, data->mapY + 1 };

    float vertices[] =
    {
		minOrigin.x,minOrigin.y,minOrigin.z,
		maxOrigin.x,minOrigin.y,minOrigin.z,
		minOrigin.x,minOrigin.y,maxOrigin.z,
		maxOrigin.x,minOrigin.y,maxOrigin.z,
    };

    float texcoords[] =
    {
        0,0,
        1,0,
        0,1,
        1,1

    };

    float normals[] =
    {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    AllocateMeshData(mesh, 4, 2, 0, true, false);
    memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
    memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
    memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

    mesh->indices[0] = 2;
    mesh->indices[1] = 1;
    mesh->indices[2] = 0;

    mesh->indices[3] = 2;
    mesh->indices[4] = 3;
    mesh->indices[5] = 1;

    mesh->vertexCount = 4;
    mesh->triangleCount = 2;
}

inline void GenWestMesh(Mesh* mesh, void* userData)
{
	CellParams* data = static_cast<CellParams*>(userData);
	if (data == nullptr)
		return;

	Vector3 minOrigin{ data->mapX, data->bottom, data->mapY };
	Vector3 maxOrigin{ data->mapX + 1, data->top, data->mapY + 1 };

    float minV = 1.0f - (data->top - data->bottom);

    float vertices[] =
    {
        minOrigin.x,maxOrigin.y,minOrigin.z,
        minOrigin.x,minOrigin.y,minOrigin.z,
        minOrigin.x,maxOrigin.y,maxOrigin.z,
        minOrigin.x,minOrigin.y,maxOrigin.z,
    };

    float texcoords[] =
    {
        1,minV,
        1,1,
        0,minV,
        0,1,
    };

    float normals[] =
    {
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };

    AllocateMeshData(mesh, 4, 2, 0, true, false);
    memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
    memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
    memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

    mesh->indices[0] = 2;
    mesh->indices[1] = 1;
    mesh->indices[2] = 0;

    mesh->indices[3] = 2;
    mesh->indices[4] = 3;
    mesh->indices[5] = 1;

    mesh->vertexCount = 4;
    mesh->triangleCount = 2;
}

inline void GenEastMesh(Mesh* mesh, void* userData)
{
	CellParams* data = static_cast<CellParams*>(userData);
	if (data == nullptr)
		return;

	Vector3 minOrigin{ data->mapX, data->bottom, data->mapY };
	Vector3 maxOrigin{ data->mapX + 1, data->top, data->mapY + 1 };

	float minV = 1.0f - (data->top - data->bottom);

    float vertices[] =
    {
		maxOrigin.x,maxOrigin.y,minOrigin.z,
		maxOrigin.x,minOrigin.y,minOrigin.z,
		maxOrigin.x,maxOrigin.y,maxOrigin.z,
		maxOrigin.x,minOrigin.y,maxOrigin.z,
    };

    float texcoords[] =
    {
        0,minV,
        0,1,
        1,minV,
        1,1,
    };

    float normals[] =
    {
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
    };

    AllocateMeshData(mesh, 4, 2, 0, true, false);
    memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
    memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
    memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

    mesh->indices[0] = 0;
    mesh->indices[1] = 1;
    mesh->indices[2] = 2;

    mesh->indices[3] = 1;
    mesh->indices[4] = 3;
    mesh->indices[5] = 2;

    mesh->vertexCount = 4;
    mesh->triangleCount = 2;
}

inline void GenSouthMesh(Mesh* mesh, void* userData)
{
	CellParams* data = static_cast<CellParams*>(userData);
	if (data == nullptr)
		return;

	Vector3 minOrigin{ data->mapX, data->bottom, data->mapY };
	Vector3 maxOrigin{ data->mapX + 1, data->top, data->mapY + 1 };

	float minV = 1.0f - (data->top - data->bottom);

    float vertices[] =
    {
		minOrigin.x,maxOrigin.y,maxOrigin.z,
		minOrigin.x,minOrigin.y,maxOrigin.z,
		maxOrigin.x,maxOrigin.y,maxOrigin.z,
		maxOrigin.x,minOrigin.y,maxOrigin.z,
    };

    float texcoords[] =
    {
        1,minV,
        1,1,
        0,minV,
        0,1,
    };

    float normals[] =
    {
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
    };

    AllocateMeshData(mesh, 4, 2, 0, true, false);
    memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
    memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
    memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

    mesh->indices[0] = 2;
    mesh->indices[1] = 1;
    mesh->indices[2] = 0;

    mesh->indices[3] = 2;
    mesh->indices[4] = 3;
    mesh->indices[5] = 1;

    mesh->vertexCount = 4;
    mesh->triangleCount = 2;
}

inline void GenNorthMesh(Mesh* mesh, void* userData)
{
	CellParams* data = static_cast<CellParams*>(userData);
	if (data == nullptr)
		return;

	Vector3 minOrigin{ data->mapX, data->bottom, data->mapY };
	Vector3 maxOrigin{ data->mapX + 1, data->top, data->mapY + 1 };

	float minV = 1.0f - (data->top - data->bottom);

    float vertices[] =
    {
		minOrigin.x,maxOrigin.y,minOrigin.z,
		minOrigin.x,minOrigin.y,minOrigin.z,
		maxOrigin.x,maxOrigin.y,minOrigin.z,
		maxOrigin.x,minOrigin.y,minOrigin.z,
    };

    float texcoords[] =
    {
        0,minV,
        0,1,
        1,minV,
        1,1,
    };

    float normals[] =
    {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
    };

    AllocateMeshData(mesh, 4, 2, 0, true, false);
    memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
    memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
    memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

    mesh->indices[0] = 0;
    mesh->indices[1] = 1;
    mesh->indices[2] = 2;

    mesh->indices[3] = 1;
    mesh->indices[4] = 3;
    mesh->indices[5] = 2;

    mesh->vertexCount = 4;
    mesh->triangleCount = 2;
}