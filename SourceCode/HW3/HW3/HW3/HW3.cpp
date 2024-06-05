#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <random>
#include <numbers>
#include <iostream>

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

class RGB 
{
public:
	uint8_t r;
	uint8_t g;
	uint8_t b;

	std::string ToString() const
	{
		return std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
	}
};

// TODO: use glm for following homeworks
struct Vector3
{
	float x;
	float y;
	float z;

	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	void Normalize()
	{
		float length = std::sqrt(x * x + y * y + z * z);
		x /= length;
		y /= length;
		z /= length;
	}

	RGB ToRGB() const
	{
		return RGB{ static_cast<uint8_t>(x * 256), static_cast<uint8_t>(y * 256), static_cast<uint8_t>(z * 256) };
	}
};

void CreateImage()
{
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

			Vector3 direction{ x, y, -1 };
			direction.Normalize();

			RGB color = direction.ToRGB();
			color.b = 0u;

			writer << color.ToString() << "\t";
		}
		writer << "\n";
	}
}

int main() 
{
	try 
	{
		CreateImage();
	}
	catch (const std::runtime_error& e) 
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}