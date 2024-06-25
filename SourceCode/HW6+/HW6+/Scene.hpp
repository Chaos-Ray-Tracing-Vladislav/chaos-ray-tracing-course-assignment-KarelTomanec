#pragma once

#include "Math3D.hpp"
#include "Camera.hpp"

#define RAPIDJSON_NOMEMBERITERATORCLASS
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include <vector>
#include <algorithm>
#include <iostream>



// Helper functions
Vector3 loadVector(const rapidjson::Value::ConstArray& arr)
{
	assert(arr.Size() == 3);
	return Vector3{
		static_cast<float>(arr[0].GetDouble()),
		static_cast<float>(arr[1].GetDouble()),
		static_cast<float>(arr[2].GetDouble())
	};
}

Matrix4 loadMatrix(const rapidjson::Value::ConstArray& arr) 
{
	assert(arr.Size() == 9);
	Matrix4 result = Matrix4::Identity();
	for(uint32_t i = 0; i < 3; i++)
	{
		for(uint32_t j = 0; j < 3; j++) 
			result(i, j) = static_cast<float>(arr[i + 3 * j].GetDouble());
	}
	return result;
}

std::vector<Vector3> loadVertices(const rapidjson::Value::ConstArray& arr) 
{
	assert(arr.Size() % 3 == 0);
	std::vector<Vector3> result;
	result.reserve(arr.Size() / 3);
	for(uint32_t i = 0; i < arr.Size(); i += 3) {
		result.emplace_back(
			Vector3(
				static_cast<float>(arr[i].GetDouble()), 
				static_cast<float>(arr[i+1].GetDouble()), 
				static_cast<float>(arr[i+2].GetDouble())
			)
		);
	}
	return result;
}

std::vector<uint32_t> loadIndices(const rapidjson::Value::ConstArray& arr) 
{
	assert(arr.Size() % 3 == 0);
	std::vector<uint32_t> result;
	result.reserve(arr.Size() / 3);
	for(uint32_t i = 0; i < arr.Size(); ++i)
		result.emplace_back(arr[i].GetInt());
	return result;
}

struct Light
{
	float intensity;
	Vector3 position;
};

class Scene
{
public:

	struct ImageSettings
	{
		uint32_t width;
		uint32_t height;
	};
	struct Settings
	{
		std::string sceneName;
		RGB backgroundColor;
		ImageSettings imageSettings;
	};


	Scene(const std::string& fileName)
	{
		parseSceneFile(fileName);
	}

	Scene(Camera camera, std::vector<Triangle> triangles) : camera(std::move(camera)), triangles(std::move(triangles)) {}

	HitInfo ClosestHit(const Ray& ray) const
	{
		HitInfo hitInfo;
		for (const auto& triangle : triangles)
		{
			HitInfo currHitInfo = triangle.Intersect(ray);
			if (currHitInfo.hit && currHitInfo.t < hitInfo.t)
				hitInfo = std::move(currHitInfo);
		}
		return hitInfo;
	}

	bool AnyHit(const Ray& ray) const
	{
		return std::any_of(triangles.begin(), triangles.end(), [&ray](const Triangle& triangle)
			{
				HitInfo hitInfo = triangle.Intersect(ray);
				return hitInfo.hit;
			});
	}

	Settings GetSettings() const { return settings; }

	const std::vector<Light>& GetLights() const { return lights; }

	Camera camera;
protected:

	inline static const std::string kSceneSettingsStr{ "settings" };
	inline static const std::string kBackgroundColorStr{ "background_color" };
	inline static const std::string kImageSettingsStr{ "image_settings" };
	inline static const std::string kImageWidthStr{ "width" };
	inline static const std::string kImageHeightStr{ "height" };
	inline static const std::string kCameraStr{ "camera" };
	inline static const std::string kMatrixStr{ "matrix" };
	inline static const std::string kLightsStr{ "lights" };
	inline static const std::string kIntensityStr{ "intensity" };
	inline static const std::string kPositionStr{ "position" };
	inline static const std::string kObjectsStr{ "objects" };
	inline static const std::string kVerticesStr{ "vertices" };
	inline static const std::string kTrianglesStr{ "triangles" };


	void parseSceneFile(const std::string& fileName)
	{
		using namespace rapidjson;
		Document doc = getJsonDocument(fileName);
		settings.sceneName = fileName;

		const Value& settingsVal = doc.FindMember(kSceneSettingsStr.c_str())->value;
		if (!settingsVal.IsNull() && settingsVal.IsObject())
		{
			const Value& bgColorVal = settingsVal.FindMember(kBackgroundColorStr.c_str())->value;
			assert(!bgColorVal.IsNull() && bgColorVal.IsArray());
			settings.backgroundColor = loadVector(bgColorVal.GetArray()).ToRGB();

			const Value& imageSettingsVal = settingsVal.FindMember(kImageSettingsStr.c_str())->value;
			if (!imageSettingsVal.IsNull() && imageSettingsVal.IsObject())
			{
				const Value& imageWidthVal = imageSettingsVal.FindMember(kImageWidthStr.c_str())->value;
				const Value& imageHeightVal = imageSettingsVal.FindMember(kImageHeightStr.c_str())->value;
				assert(!imageWidthVal.IsNull() && imageWidthVal.IsInt() && !imageHeightVal.IsNull() && imageHeightVal.IsInt());
				settings.imageSettings.width = imageWidthVal.GetInt();
				settings.imageSettings.height = imageHeightVal.GetInt();
			}
		}

		const Value& cameraVal = doc.FindMember(kCameraStr.c_str())->value;
		if (!cameraVal.IsNull() && cameraVal.IsObject())
		{
			const Value& matrixVal = cameraVal.FindMember(kMatrixStr.c_str())->value;
			assert(!matrixVal.IsNull() && matrixVal.IsArray());
			Matrix4 rotation = loadMatrix(matrixVal.GetArray());

			const Value& positionVal = cameraVal.FindMember(kPositionStr.c_str())->value;
			assert(!positionVal.IsNull() && positionVal.IsArray());
			Matrix4 translation = MakeTranslation(loadVector(positionVal.GetArray()));

			camera.transform = rotation * translation;
		}

		const Value& lightsValue = doc.FindMember(kLightsStr.c_str())->value;
		if (!lightsValue.IsNull() && lightsValue.IsArray())
		{
			for (Value::ConstValueIterator it = lightsValue.Begin(); it != lightsValue.End(); ++it)
			{
				Light light;
				const Value& intensityValue = it->FindMember(kIntensityStr.c_str())->value;
				assert(!intensityValue.IsNull() && intensityValue.IsFloat());
				light.intensity = intensityValue.GetFloat();

				const Value& positionVal = it->FindMember(kPositionStr.c_str())->value;
				assert(!positionVal.IsNull() && positionVal.IsArray());
				light.position = loadVector(positionVal.GetArray());

				lights.push_back(light);
			}
		}

		const Value& objectsValue = doc.FindMember(kObjectsStr.c_str())->value;
		if(!objectsValue.IsNull() && objectsValue.IsArray()) 
		{
			for(Value::ConstValueIterator it = objectsValue.Begin(); it != objectsValue.End(); ++it)
			{
				const Value& verticesValue = it->FindMember(kVerticesStr.c_str())->value;
				assert(!verticesValue.IsNull() && verticesValue.IsArray());
				std::vector<Vector3> vertices = loadVertices(verticesValue.GetArray());

				const Value& trianglesValue = it->FindMember(kTrianglesStr.c_str())->value;
				assert(!trianglesValue.IsNull() && trianglesValue.IsArray());
				std::vector<uint32_t> indices = loadIndices(trianglesValue.GetArray());

				triangles.reserve(indices.size() / 3);
				for (uint32_t i = 0; i < indices.size(); i += 3)
				{
					triangles.emplace_back(
						vertices[indices[i]],
						vertices[indices[i + 1]],
						vertices[indices[i + 2]]
					);
				}
			}
		}

	}

	rapidjson::Document getJsonDocument(const std::string& fileName)
	{
		using namespace rapidjson;

		std::ifstream ifs(fileName);
		assert(ifs.is_open());

		IStreamWrapper isw(ifs);
		Document doc;
		doc.ParseStream(isw);

		if (doc.HasParseError())
		{
			std::cout << "Error : " << doc.GetParseError() << '\n';
			std::cout << "Offset : " << doc.GetErrorOffset() << '\n';
			assert(false);
		}
		assert(doc.IsObject());

		return doc;	// RVO
	}

	std::vector<Triangle> triangles;
	std::vector<Light> lights;
	Settings settings;
};