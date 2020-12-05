#pragma once
#include "raylib.h"
#include "raylibExtras.h"
#include "raymath.h"
#include <cstdint>
#include <memory>

constexpr float xmin = -0.001f;
constexpr float xmax = 1.001f;

constexpr float zmin = -0.001f;
constexpr float zmax = 1.001f;

constexpr float ymin = 0;
constexpr float ymax = 1;

void GenCeilingMesh(Mesh* mesh, void* userData)
{
    float width = 1;
    float height = 1;
    float length = 1;

    float vertices[] =
    {
        xmin,ymin,zmin,
        xmax,ymin,zmin,
        xmin,ymin,zmax,
        xmax,ymin,zmax,
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

void GenFloorMesh(Mesh* mesh, void* userData)
{
    float width = 1;
    float height = 1;
    float length = 1;

    float vertices[] =
    {
        xmin,ymin,zmin,
        xmax,ymin,zmin,
        xmin,ymin,zmax,
        xmax,ymin,zmax,
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

void GenWestMesh(Mesh* mesh, void* userData)
{
    float width = 1;
    float height = 1;
    float length = 1;

    float vertices[] =
    {
        xmin,ymax,zmin,
        xmin,ymin,zmin,
        xmin,ymax,zmax,
        xmin,ymin,zmax,
    };

    float texcoords[] =
    {
        1,0,
        1,1,
        0,0,
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

void GenEastMesh(Mesh* mesh, void* userData)
{
    float width = 1;
    float height = 1;
    float length = 1;

    float vertices[] =
    {
        xmax,ymax,zmin,
        xmax,ymin,zmin,
        xmax,ymax,zmax,
        xmax,ymin,zmax,
    };

    float texcoords[] =
    {
        0,0,
        0,1,
        1,0,
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

void GenSouthMesh(Mesh* mesh, void* userData)
{
    float width = 1;
    float height = 1;
    float length = 1;

    float vertices[] =
    {
        xmin,ymax,zmax,
        xmin,ymin,zmax,
        xmax,ymax,zmax,
        xmax,ymin,zmax,
    };

    float texcoords[] =
    {
        1,0,
        1,1,
        0,0,
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

void GenNorthMesh(Mesh* mesh, void* userData)
{
    float width = 1;
    float height = 1;
    float length = 1;

    float vertices[] =
    {
        xmin,ymax,zmin,
        xmin,ymin,zmin,
        xmax,ymax,zmin,
        xmax,ymin,zmin,
    };

    float texcoords[] =
    {
        0,0,
        0,1,
        1,0,
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