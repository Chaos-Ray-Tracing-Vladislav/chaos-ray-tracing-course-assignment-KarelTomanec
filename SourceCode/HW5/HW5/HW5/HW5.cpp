#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <random>
#include <numbers>
#include <iostream>

#include "Math3D.hpp"

/// Output image resolution
static constexpr uint32_t imageWidth = 1000;
static constexpr uint32_t imageHeight = 1000;
static constexpr uint32_t maxColorComponent = 255;

class PPMWriter 
{
public:

	PPMWriter(std::string filename, uint32_t imageWidth, uint32_t imageHeight, uint32_t maxColorComponent)
		: ppmFileStream(filename + ".ppm", std::ios::out | std::ios::binary)
	{
		if (!ppmFileStream.is_open())
			throw std::runtime_error("Failed to open file: " + filename + ".ppm");

		ppmFileStream << "P3\n";
		ppmFileStream << imageWidth << " " << imageHeight << "\n";
		ppmFileStream << maxColorComponent << "\n";
	}

	~PPMWriter() 
	{
		if (ppmFileStream.is_open())
			ppmFileStream.close();
	}

	friend PPMWriter& operator<<(PPMWriter& writer, const std::string& data) 
	{
		writer.ppmFileStream << data;
		return writer;
	}

private:
	std::ofstream ppmFileStream;
};

class Scene
{
public:

	Scene(std::vector<Triangle> triangles) : triangles(std::move(triangles)) {}

	bool Intersect(const Ray& ray) const
	{
		return std::any_of(triangles.begin(), triangles.end(), [&ray](const Triangle& triangle)
			{
				return triangle.Intersect(ray);
			});
	}

private:
	std::vector<Triangle> triangles;
};

void CreateImage(const Scene& scene)
{
	RGB backgroundColor{ 0, 0, 0 };
	RGB triangleColor{ 255, 255, 255 };
	PPMWriter writer("img", imageWidth, imageHeight, maxColorComponent);
	for (uint32_t rowIdx = 0; rowIdx < imageHeight; ++rowIdx)
	{
		float y = static_cast<float>(rowIdx) + 0.5f; // To pixel center
		y /= imageHeight; // To NDC
		y = 1.f - (2.f * y); // To screen space
		for (uint32_t colIdx = 0; colIdx < imageWidth; ++colIdx)
		{
			float x = static_cast<float>(colIdx) + 0.5f; // To pixel center
			x /= imageWidth; // To NDC
			x = 2.f * x - 1.f; // To screen space
			x *= static_cast<float>(imageWidth) / imageHeight; // Consider aspect ratio

			Vector3 origin{ 0.f, 0.f, 0.f };
			Vector3 direction{ x, y, -1.f };
			direction = Normalize(direction);

			Ray ray{ origin, direction };
			RGB color = backgroundColor;
			if (scene.Intersect(ray))
				color = triangleColor;

			writer << color.ToString() << "\t";
		}
		writer << "\n";
	}
}

int main() 
{

	// Create scene
	Triangle tri
	{
		Vector3{-1.75f, -1.75f, -3.f},
		Vector3{1.75f, -1.75f, -3.f},
		Vector3{0.f, 1.75f, -3.f},
	};

	Triangle secondTri
	{
		Vector3{ 2, 2, -3 },
		Vector3{ 1.75, 2, -3 },
		Vector3{ 2, 1.75, -3 },
	};

	Scene scene{ { tri, secondTri } };

	CreateImage(scene);

	return 0;
}