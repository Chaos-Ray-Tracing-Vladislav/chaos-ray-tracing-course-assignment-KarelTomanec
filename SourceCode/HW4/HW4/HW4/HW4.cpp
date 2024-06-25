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

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vector3& operator *=(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return (*this);
	}

	Vector3& operator /=(float s)
	{
		s = 1.0F / s;
		x *= s;
		y *= s;
		z *= s;
		return (*this);
	}

	Vector3& operator +=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return (*this);
	}

	Vector3& operator -=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return (*this);
	}

	float Magnitude() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	RGB ToRGB() const
	{
		return RGB{ static_cast<uint8_t>(x * 256), static_cast<uint8_t>(y * 256), static_cast<uint8_t>(z * 256) };
	}

	std::string ToString() const
	{
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
	}
};

inline Vector3 operator +(const Vector3& a, const Vector3& b)
{
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vector3 operator -(const Vector3& a, const Vector3& b)
{
	return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vector3 operator *(const Vector3& v, float s)
{
	return { v.x * s, v.y * s, v.z * s };
}

inline Vector3 operator /(const Vector3& v, float s)
{
	s = 1.0f / s;
	return { v.x * s, v.y * s, v.z * s };
}

inline Vector3 Normalize(const Vector3& v)
{
	return v / v.Magnitude();
}

inline Vector3 Cross(const Vector3& a, const Vector3& b)
{
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

struct Triangle
{
	Vector3 a;
	Vector3 b;
	Vector3 c;


	Triangle(Vector3 a, Vector3 b, Vector3 c) : a(a), b(b), c(c) {}

	Vector3 Normal() const
	{
		return Normalize(Cross(b - a, c - a));
	}

	float Area() const
	{
		return Cross(b - a, c - a).Magnitude() * 0.5f;
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
			direction = Normalize(direction);

			RGB color = direction.ToRGB();
			color.b = 0u;

			writer << color.ToString() << "\t";
		}
		writer << "\n";
	}
}

int main() 
{
	// Task 2
	std::cout << "Task 2: " << '\n';
	{
		std::cout << "2.1" << '\n';
		Vector3 a{ 3.5f, 0.f, 0.f };
		Vector3 b{ 1.75f, 3.5f, 0.f };
		Vector3 res = Cross(a, b);
		std::cout << res.ToString() << '\n';
	}
	{
		std::cout << "2.2" << '\n';
		Vector3 a{ 3.f, -3.f, 1.f };
		Vector3 b{ 4.f, 9.f, 3.f };
		Vector3 res = Cross(a, b);
		std::cout << res.ToString() << '\n';
	}
	{
		std::cout << "2.3" << '\n';
		Vector3 a{ 3.f, -3.f, 1.f };
		Vector3 b{ 4.f, 9.f, 3.f };
		float res = Cross(a, b).Magnitude();
		std::cout << res << '\n';
	}
	{
		std::cout << "2.4" << '\n';
		Vector3 a{ 3.f, -3.f, 1.f };
		Vector3 b{ -12.f, 12.f, -4.f };
		float res = Cross(a, b).Magnitude();
		std::cout << res << '\n';
	}

	// Task 3
	std::cout << "Task 3: " << '\n';
	std::cout << "3.1" << '\n';
	Vector3 a1{ -1.75f, -1.75f, -3.f };
	Vector3 b1{ 1.75f, -1.75f, -3.f };
	Vector3 c1{ 0.f, 1.75f, -3.f };
	Triangle tri1{ a1, b1, c1 };
	std::cout << tri1.Normal().ToString() << '\n';

	std::cout << "3.2" << '\n';
	Vector3 a2{ 0.f, 0.f, -1.f };
	Vector3 b2{ 1.f, 0.f, 1.f };
	Vector3 c2{ -1.f, 0.f, 1.f };
	Triangle tri2{ a2, b2, c2 };
	std::cout << tri2.Normal().ToString() << '\n';

	std::cout << "3.3" << '\n';
	Vector3 a3{ 0.56f, 1.11f, 1.23f };
	Vector3 b3{ 0.44f, -2.368f, -0.54f };
	Vector3 c3{ -1.56f, 0.15f, -1.92f };
	Triangle tri3{ a3, b3, c3 };
	std::cout << tri3.Normal().ToString() << '\n';

	std::cout << "3.4" << '\n';
	std::cout << tri1.Area() << '\n';
	std::cout << tri2.Area() << '\n';
	std::cout << tri3.Area() << '\n';

	return 0;
}