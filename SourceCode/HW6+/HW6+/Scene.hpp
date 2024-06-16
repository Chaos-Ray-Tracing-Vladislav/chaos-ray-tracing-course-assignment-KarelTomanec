#pragma once

#include "Math3D.hpp"
#include "Camera.hpp"

#include <vector>
#include <algorithm>

class Scene
{
public:

	Scene(Camera camera, std::vector<Triangle> triangles) : camera(std::move(camera)), triangles(std::move(triangles)) {}

	bool Intersect(const Ray& ray) const
	{
		return std::any_of(triangles.begin(), triangles.end(), [&ray](const Triangle& triangle)
			{
				return triangle.Intersect(ray);
			});
	}

	Camera camera;
protected:
	std::vector<Triangle> triangles;
};