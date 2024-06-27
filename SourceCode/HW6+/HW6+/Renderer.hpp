#pragma once

#include "Math3D.hpp"
#include "PPMWriter.hpp"
#include "Scene.hpp"
#include <thread>
#include <mutex>
#include <barrier>
#include <utility>

class Image
{
public:
    Image(uint32_t width, uint32_t height) : width(width), height(height)
    {
        pixels.resize(width * height);
    }

    void SetPixel(uint32_t x, uint32_t y, const RGB& color)
    {
        pixels[y * width + x] = color;
    }

    const RGB& GetPixel(uint32_t x, uint32_t y) const
    {
        return pixels[y * width + x];
    }

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

private:
    uint32_t width, height;
    std::vector<RGB> pixels;
};

class Renderer
{
public:
    Renderer(Scene& scene) : scene(scene) {}

    void RenderImage()
    {
        Scene::Settings sceneSettings = scene.settings;

        const uint32_t imageWidth = sceneSettings.imageSettings.width;
        const uint32_t imageHeight = sceneSettings.imageSettings.height;

        Image image(imageWidth, imageHeight);

        auto renderTask = [&](uint32_t startRow, uint32_t endRow)
            {
                for (uint32_t rowIdx = startRow; rowIdx < endRow; ++rowIdx)
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

                        RGB color = GetPixel(x, y);

                        image.SetPixel(colIdx, rowIdx, color);
                    }
                }
            };

        uint32_t numThreads = std::thread::hardware_concurrency();
        std::vector<std::jthread> threads;
        uint32_t rowsPerThread = imageHeight / numThreads;

        for (uint32_t i = 0; i < numThreads; ++i)
        {
            uint32_t startRow = i * rowsPerThread;
            uint32_t endRow = (i == numThreads - 1) ? imageHeight : startRow + rowsPerThread;
            threads.emplace_back([&, startRow, endRow]()
                {
                    renderTask(startRow, endRow);
                });
        }

        for (auto& thread : threads)
            thread.join();

        WriteToFile(image, sceneSettings);
    }

protected:

    void WriteToFile(const Image& image, const Scene::Settings& sceneSettings)
    {
        const auto imageWidth = image.GetWidth();
        const auto imageHeight = image.GetHeight();
        PPMWriter writer(sceneSettings.sceneName + "_render", imageWidth, imageHeight, maxColorComponent);

        for (uint32_t rowIdx = 0; rowIdx < imageHeight; ++rowIdx)
        {
            for (uint32_t colIdx = 0; colIdx < imageWidth; ++colIdx)
            {
                writer << image.GetPixel(colIdx, rowIdx).ToString() << "\t";
            }
            writer << "\n";
        }
    }

    RGB GetPixel(float x, float y)
    {
        Vector3 origin = scene.camera.GetPosition();
        Vector3 forward = scene.camera.GetLookDirection();

        // Assume up vector is Y axis in camera space and right vector is X axis in camera space
        Vector3 up = Normalize(scene.camera.transform * Vector3(0.f, 1.f, 0.f));
        Vector3 right = Cross(forward, up);

        // Calculate direction to pixel in camera space
        Vector3 direction = Normalize(forward + right * x + up * y);

        Ray ray{ origin, direction };
        const uint32_t maxTraceDepth = 2;

        Vector3 throughput{ 1.f };
        Vector3 L{ 0.f };
        for (uint32_t depth = 0; depth < maxTraceDepth; ++depth)
        {
            HitInfo hitInfo = scene.ClosestHit(ray);
            if (hitInfo.hit)
            {
                const auto& mesh = scene.meshes[hitInfo.meshIndex];
                const auto& material = scene.materials[mesh.materialIndex];
                Vector3 normal = hitInfo.normal;
                if (material.smoothShading)
                {
                    const auto& triangle = mesh.triangles[hitInfo.triangleIndex];
                    normal = triangle.GetNormal(hitInfo.u, hitInfo.v);
                }

                for (const auto& light : scene.lights)
                {
                    Vector3 offsetOrigin = OffsetRayOrigin(hitInfo.point, hitInfo.normal);
                    Vector3 dirToLight = Normalize(light.position - offsetOrigin);
                    float distanceToLight = (light.position - offsetOrigin).Magnitude();
                    Ray shadowRay{ offsetOrigin, dirToLight, distanceToLight};
                    if (!scene.AnyHit(shadowRay))
                    {
                        float attenuation = 1.0f / (distanceToLight * distanceToLight);
                        L += throughput * material.albedo * std::max(0.f, Dot(normal, dirToLight)) * attenuation * light.intensity;
                        throughput *= material.albedo;
                    }
                }
                if (material.type == Material::Type::DIFFUSE)
                    break;
                ray.origin = OffsetRayOrigin(hitInfo.point, hitInfo.normal);
                ray.directionN = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
            }
            else
            {
                L += throughput * scene.settings.backgroundColor;
                break;
            }
        }
        return L.ToRGB();
    }

    static constexpr uint32_t maxColorComponent = 255;
    Scene& scene;
};