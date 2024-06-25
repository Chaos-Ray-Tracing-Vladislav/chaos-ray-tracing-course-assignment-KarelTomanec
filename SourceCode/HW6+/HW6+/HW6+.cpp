
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
	Scene::Settings sceneSettings = scene.GetSettings();
	RGB backgroundColor = sceneSettings.backgroundColor;
	RGB triangleColor{ 255, 255, 255 };

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
			if (scene.Intersect(ray))
				color = triangleColor;

			writer << color.ToString() << "\t";
		}
		writer << "\n";
	}
}

int main()
{
	std::vector<Scene> scenes{
		Scene{ "scene0.crtscene" },
		Scene{ "scene1.crtscene" },
		Scene{ "scene2.crtscene" },
		Scene{ "scene3.crtscene" },
		Scene{ "scene4.crtscene" },
	};
	for(auto& scene : scenes)
		RenderImageSequence(scene);

	return 0;
}