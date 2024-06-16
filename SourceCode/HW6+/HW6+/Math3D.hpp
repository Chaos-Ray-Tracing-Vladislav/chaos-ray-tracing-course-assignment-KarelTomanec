#pragma once

#include <cstdint>
#include <numbers>
#include <string>
#include <cmath>

constexpr float DegToRad(float degrees)
{
	return degrees * (std::numbers::pi_v<float> / 180.f);
}

constexpr float RadToDeg(float radians)
{
	return radians * (180.f / std::numbers::pi_v<float>);
}

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
		s = 1.f / s;
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
	s = 1.f / s;
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

struct Point3 : Vector3
{
	Point3() = default;

	Point3(float a, float b, float c) : Vector3(a, b, c) {}

	Point3& operator =(const Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return (*this);
	}
};

inline Point3 operator +(const Point3& a, const Vector3& b)
{
	return Point3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Point3 operator -(const Point3& a, const Vector3& b)
{
	return Point3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3 operator -(const Point3& a, const Point3& b)
{
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
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

struct Matrix4
{
protected:

	float		n[4][4];

public:

	Matrix4() = default;

	Matrix4(float n00, float n01, float n02, float n03,
		float n10, float n11, float n12, float n13,
		float n20, float n21, float n22, float n23,
		float n30, float n31, float n32, float n33)
	{
		n[0][0] = n00; n[0][1] = n10; n[0][2] = n20; n[0][3] = n30;
		n[1][0] = n01; n[1][1] = n11; n[1][2] = n21; n[1][3] = n31;
		n[2][0] = n02; n[2][1] = n12; n[2][2] = n22; n[2][3] = n32;
		n[3][0] = n03; n[3][1] = n13; n[3][2] = n23; n[3][3] = n33;
	}


	float& operator ()(int i, int j)
	{
		return n[j][i];
	}

	const float& operator ()(int i, int j) const
	{
		return n[j][i];
	}

	const Point3& GetTranslation(void) const
	{
		return *reinterpret_cast<const Point3*>(n[3]);
	}

	static Matrix4 Identity()
	{
		return Matrix4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}
};

static Matrix4 MakeTranslation(Vector3 t)
{

	return Matrix4(	1.f, 0.f, 0.f, t.x,
					0.f, 1.f, 0.f, t.y,
					0.f, 0.f, 1.f, t.z,
					0.f, 0.f, 0.f, 1.f);
}

static Matrix4 MakeRotationX(float t)
{
	float c = cos(t);
	float s = sin(t);

	return Matrix4(1.f, 0.f, 0.f, 0.f,
		0.f,  c,   -s, 0.f,
		0.f,  s,    c , 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static Matrix4 MakeRotationY(float t)
{
	float c = cos(t);
	float s = sin(t);

	return Matrix4( c,   0.f,  s, 0.f,
		0.f, 1.f, 0.f, 0.f,
		-s,   0.f,  c  , 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static Matrix4 MakeRotationZ(float t)
{
	float c = cos(t);
	float s = sin(t);

	return Matrix4( c,   -s,   0.f, 0.f,
		s,    c,   0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

Vector3 operator *(const Matrix4& H, const Vector3& v)
{
	return Vector3(H(0,0) * v.x + H(0,1) * v.y + H(0,2) * v.z,
		H(1,0) * v.x + H(1,1) * v.y + H(1,2) * v.z,
		H(2,0) * v.x + H(2,1) * v.y + H(2,2) * v.z);
}

Point3 operator *(const Matrix4& H, const Point3& p)
{
	return Point3(H(0,0) * p.x + H(0,1) * p.y + H(0,2) * p.z + H(0,3),
		H(1,0) * p.x + H(1,1) * p.y + H(1,2) * p.z + H(1,3),
		H(2,0) * p.x + H(2,1) * p.y + H(2,2) * p.z + H(2,3));
}

Matrix4 operator*(const Matrix4& A, const Matrix4& B)
{
	Matrix4 result;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result(i, j) = 0;
			for (int k = 0; k < 4; ++k)
			{
				result(i, j) += A(i, k) * B(k, j);
			}
		}
	}
	return result;
}

