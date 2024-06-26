
#include "Math3D.hpp"
#include "Camera.hpp"
#include "PPMWriter.hpp"
#include "Scene.hpp"

/// Output image resolution
static constexpr uint32_t imageWidth = 1000;
static constexpr uint32_t imageHeight = 1000;
static constexpr uint32_t maxColorComponent = 255;

void RenderImageSequence(Scene& scene)
{
	Scene::Settings sceneSettings = scene.settings;
	RGB backgroundColor = sceneSettings.backgroundColor;

	const uint32_t imageWidth = sceneSettings.imageSettings.width;
	const uint32_t imageHeight = sceneSettings.imageSettings.height;

	PPMWriter writer(sceneSettings.sceneName + "_render", imageWidth, imageHeight, maxColorComponent);
		
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

			Vector3 origin = scene.camera.GetPosition();
			Vector3 forward = scene.camera.GetLookDirection();

			// Assume up vector is Y axis in camera space and right vector is X axis in camera space
			Vector3 up = Normalize(scene.camera.transform * Vector3(0.f, 1.f, 0.f));
			Vector3 right = Cross(forward, up);

			// Calculate direction to pixel in camera space
			Vector3 direction = Normalize(forward + right * x + up * y);

			Ray ray{ origin, direction };
			RGB color = backgroundColor;

			HitInfo hitInfo = scene.ClosestHit(ray);
			if (hitInfo.hit)
			{
				Vector3 L{ 0.f };
				for (const auto& light : scene.lights)
				{
					Vector3 dirToLight = Normalize(light.position - hitInfo.point);
					float distanceToLight = (light.position - hitInfo.point).Magnitude();
					Ray shadowRay{ hitInfo.point + hitInfo.normal * 0.001f, dirToLight, distanceToLight };
					if (!scene.AnyHit(shadowRay))
					{
						float attenuation = 1.0f / (distanceToLight * distanceToLight);
						const auto& mesh = scene.meshes[hitInfo.meshIndex];
						const auto& material = scene.materials[mesh.materialIndex];
						Vector3 normal = hitInfo.normal;
						if (material.smoothShading)
						{
							const auto& triangle = mesh.triangles[hitInfo.triangleIndex];
							normal = triangle.GetNormal(hitInfo.u, hitInfo.v);
						}

						L = L + material.albedo * std::max(0.f, Dot(normal, dirToLight)) * attenuation * light.intensity;
					}
				}
				L = L * 0.1f; // Exposure
				color = L.ToRGB();
			}

			writer << color.ToString() << "\t";
		}
		writer << "\n";
	}
}

int main()
{
	std::vector<Scene> scenes{
		//Scene{ "scene0.crtscene" },
		//Scene{ "scene1.crtscene" },
		//Scene{ "scene2.crtscene" },
		Scene{ "scene3.crtscene" },
		//Scene{ "scene4.crtscene" },
		//Scene{ "scene5.crtscene" },
	};
	for(auto& scene : scenes)
		RenderImageSequence(scene);

	return 0;
}