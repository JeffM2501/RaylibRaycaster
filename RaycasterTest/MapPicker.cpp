#include "MapPicker.h"
#include "raymath.h"

namespace MapPicker
{
	Directions DirectionFromNormal(Vector3 &normal)
	{
		constexpr float limit = 0.75f;
		if (normal.x > limit)
			return Directions::XNeg;
		else if (normal.x < -limit)
			return Directions::XPos;

		if (normal.z > limit)
			return Directions::ZNeg;
		else if (normal.z < -limit)
			return Directions::ZPos;

		if (normal.y < -limit)
			return Directions::YPos;

		return Directions::YNeg;
	}

	// Get collision info between ray and mesh
	RayHitInfo GetCollisionRayMesh2(Ray& ray, Mesh& mesh, Matrix& transform)
	{
		RayHitInfo result = { 0 };

		// Check if mesh vertex data on CPU for testing
		if (mesh.vertices != NULL)
		{
			// model->mesh.triangleCount may not be set, vertexCount is more reliable
			int triangleCount = mesh.triangleCount;

			// Test against all triangles in mesh
			for (int i = 0; i < triangleCount; i++)
			{
				Vector3 a, b, c;
				Vector3* vertdata = (Vector3*)mesh.vertices;

				if (mesh.indices)
				{
					a = vertdata[mesh.indices[i * 3 + 0]];
					b = vertdata[mesh.indices[i * 3 + 1]];
					c = vertdata[mesh.indices[i * 3 + 2]];
				}
				else
				{
					a = vertdata[i * 3 + 0];
					b = vertdata[i * 3 + 1];
					c = vertdata[i * 3 + 2];
				}

				a = Vector3Transform(a, transform);
				b = Vector3Transform(b, transform);
				c = Vector3Transform(c, transform);

				RayHitInfo triHitInfo = GetCollisionRayTriangle(ray, a, b, c);

				if (triHitInfo.hit)
				{
					// Save the closest hit triangle
					if ((!result.hit) || (result.distance > triHitInfo.distance)) result = triHitInfo;
				}
			}
		}
		return result;
	}

	PickResults PickFace(MapRenderer& renderer, MapVisibilitySet viewSet, Vector2 screenPoint)
	{
		float distance = FLT_MAX;

		PickResults results;
		results.PickRay = { 0 };

		results.PickRay = GetMouseRay(screenPoint, viewSet.ViewCamera.GetCamera());
		
		Matrix identity = MatrixIdentity();

		for (auto& viscell : viewSet.VisibleCells)
		{
			RenderCell* cell = viscell.second;

			for (auto& face : cell->RenderFaces)
			{
				RayHitInfo meshHitInfo = GetCollisionRayMesh2(results.PickRay, face.FaceMesh, identity);

				if (meshHitInfo.hit && meshHitInfo.distance < distance)
				{
					distance = meshHitInfo.distance;
					results.CellIndex = cell->MapCell->Index;
					results.FaceDirection = DirectionFromNormal(meshHitInfo.normal);
					results.HitPosition = meshHitInfo.position;
				}
			}
		}

		return results;
	}
}