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
		ppmFileStream << "P3\n";
		ppmFileStream << imageWidth << " " << imageHeight << "\n";
		ppmFileStream << maxColorComponent << "\n";
	}

	~PPMWriter() 
	{
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

class HSV
{
public:
	float h; // Hue in degrees [0, 360)
	float s; // Saturation as a percentage [0, 100]
	float v; // Value as a percentage [0, 100]

	HSV(float hue, float saturation, float value)
		: h(hue), s(saturation), v(value) {}

	RGB ToRGB() const
	{
		float r, g, b;

		float s_norm = s / 100.0f;
		float v_norm = v / 100.0f;
		float c = v_norm * s_norm;
		float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
		float m = v_norm - c;

		if (h >= 0 && h < 60) {
			r = c, g = x, b = 0;
		}
		else if (h >= 60 && h < 120) {
			r = x, g = c, b = 0;
		}
		else if (h >= 120 && h < 180) {
			r = 0, g = c, b = x;
		}
		else if (h >= 180 && h < 240) {
			r = 0, g = x, b = c;
		}
		else if (h >= 240 && h < 300) {
			r = x, g = 0, b = c;
		}
		else {
			r = c, g = 0, b = x;
		}

		return RGB{ static_cast<uint8_t>((r + m) * 255), static_cast<uint8_t>((g + m) * 255), static_cast<uint8_t>((b + m) * 255) };
	}
};

float rnd() 
{
	static std::random_device rd;  // Seed
	static std::mt19937 gen(rd()); // Mersenne Twister generator
	std::uniform_real_distribution<> dis(0.0, 1.0);
	return dis(gen);
}

float frac(float number) 
{
	return number - std::floor(number);
}

void GenerateRectangles(uint32_t verticalSegments, uint32_t horizontalSegments)
{
	const uint32_t colorCount = horizontalSegments * verticalSegments;

	// Generate low-discrepancy colors using golden ratio
	std::vector<RGB> colors;
	colors.reserve(colorCount);
	float h = rnd();
	float s = 100.f;
	float v = 100.f;
	for (uint32_t i = 0; i < colorCount; i++)
	{
		h += std::numbers::phi_v<float>;
		h = frac(h);
		colors.emplace_back(HSV(h * 360.f, s, v).ToRGB());
	}

	PPMWriter writer("rectangles", imageWidth, imageHeight, maxColorComponent);
	for (uint32_t rowIdx = 0; rowIdx < imageHeight; ++rowIdx) 
	{
		for (uint32_t colIdx = 0; colIdx < imageWidth; ++colIdx)
		{
			uint32_t vSegment = verticalSegments * static_cast<float>(colIdx) / imageWidth;
			uint32_t hSegment = horizontalSegments * static_cast<float>(rowIdx) / imageHeight;
			uint32_t colorIndex = hSegment * verticalSegments + vSegment;
			const auto color = colors[colorIndex];
			writer << color.ToString() + "\t";
		}
		writer << "\n";
	}
}


void GenerateCircle()
{
	float radius = static_cast<float>(std::min(imageWidth, imageHeight)) / 2.0f;
	float radiusSqr = radius * radius;
	float hCenter = static_cast<float>(imageWidth) / 2.0f;
	float vCenter = static_cast<float>(imageHeight) / 2.0f;

	RGB backgroundColor{ 255, 255, 255 };
	RGB circleColor{ 0, 0, 0 };

	PPMWriter writer("circle", imageWidth, imageHeight, maxColorComponent);
	for (uint32_t rowIdx = 0; rowIdx < imageHeight; ++rowIdx)
	{
		float rowI = static_cast<float>(rowIdx);
		for (uint32_t colIdx = 0; colIdx < imageWidth; ++colIdx)
		{
			float colI = static_cast<float>(colIdx);
			if ((colI - hCenter) * (colI - hCenter) + (rowI - vCenter) * (rowI - vCenter) <= radiusSqr)
				writer << circleColor.ToString() << "\t";
			else
				writer << backgroundColor.ToString() << "\t";
		}
		writer << "\n";
	}
}

int main() 
{
	GenerateRectangles(6, 6);
	GenerateCircle();
	return 0;
}