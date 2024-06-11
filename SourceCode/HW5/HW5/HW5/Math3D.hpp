#include <cstdint>

struct RGB 
{
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

inline float Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct Ray
{
	Vector3 origin;
	Vector3 directionN;

	Vector3 operator()(float t) const
	{
		return origin + directionN * t;
	}
};

struct Triangle
{
	Vector3 a;
	Vector3 b;
	Vector3 c;

	Vector3 normal;

	Triangle(Vector3 a, Vector3 b, Vector3 c)
		: a(a), b(b), c(c), normal(Normalize(Cross(b - a, c - a)))
	{}

	float Area() const
	{
		return Cross(b - a, c - a).Magnitude() * 0.5f;
	}

	bool Intersect(const Ray& ray) const
	{
		float dirDotNorm = Dot(ray.directionN, normal);
		if (dirDotNorm >= 0.f)
			return false;

		float t = Dot(a - ray.origin, normal) / dirDotNorm;
		if (t < 0.f)
			return false;

		Vector3 p = ray(t);

		Vector3 edge0 = b - a;
		Vector3 edge1 = c - b;
		Vector3 edge2 = a - c;
		Vector3 C0 = p - a;
		Vector3 C1 = p - b;
		Vector3 C2 = p - c;

		if (Dot(normal, Cross(edge0, C0)) < 0.f) 
			return false;
		if (Dot(normal, Cross(edge1, C1)) < 0.f) 
			return false;
		if (Dot(normal, Cross(edge2, C2)) < 0.f) 
			return false;

		return true;
	}
};
