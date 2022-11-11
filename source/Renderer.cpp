//External includes
// ReSharper disable CppUnusedIncludeDirective
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <thread>
#include <mutex>
#include <iostream>
#include <ppl.h>
#include <future>

using namespace dae;

//#define ASYNC
#define PARALLEL_FOR

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();


	const uint32_t numPixels{ static_cast<uint32_t>(m_Width) * static_cast<uint32_t>(m_Height) };

#if defined(ASYNC)
	const uint32_t numCores{ std::thread::hardware_concurrency() };
	std::vector<std::future<void>> async_futures{};

	const uint32_t numPixelPerTask{ numPixels / numCores };
	uint32_t numAssignedPixels{ numPixels % numCores };
	uint32_t currentPixelIndex{ 0 };

	for (uint32_t coreId{ 0 }; coreId < numCores; ++coreId)
	{
		uint32_t taskSize{ numPixelPerTask };
		if (numAssignedPixels > 0)
		{
			++taskSize;
			--numAssignedPixels;
		}

		async_futures.push_back(
			std::async(std::launch::async, [=, this]
				{
					const uint32_t endPixelIndex{ currentPixelIndex + taskSize };
					for (uint32_t pixelIndex{ currentPixelIndex }; pixelIndex < endPixelIndex; ++pixelIndex)
					{
						RenderPixel(pScene, pixelIndex, aspectRatio, camera, cameraToWorld, lights, materials);
					}
				})
		);

		currentPixelIndex += taskSize;
	}

	//wait until all tasks are finished
	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	concurrency::parallel_for(0u, numPixels, [=, this](int pixelIndex)
		{
			RenderPixel(pScene, pixelIndex, m_AspectRatio, camera, cameraToWorld, lights, materials);
		});
#else
	for (uint32_t i{ 0 }; i < numPixels; ++i)
	{
		RenderPixel(pScene, i, aspectRatio, camera, cameraToWorld, lights, materials);
	}

#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::RenderPixel(const Scene* pScene, const int pixelIndex, const float aspectRatio, const Camera& camera,
                           const Matrix cameraToWorld, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px{ pixelIndex % m_Width };
	const int py{ pixelIndex / m_Width };
	
	const float directionX{ (2.f * ((px + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians };
	const float directionY{ (1.f - 2.f * ((py + .5f) / m_Height)) * camera.fovRadians };
	
	const Vector3 rayDirection{ cameraToWorld.TransformVector(directionX, directionY, 1.f) };
	const Ray hitRay{ camera.origin, rayDirection };
	HitRecord hitRecord{};
	
	pScene->GetClosestHit(hitRay, hitRecord);
	
	ColorRGB finalColor{};
	if (hitRecord.didHit)
	{
		Ray rayToLight{};
		for (const auto& currentLight : lights)
		{
			Vector3 directionToLight{ LightUtils::GetDirectionToLight(currentLight, hitRecord.origin) };
			Vector3 directionNormalized{ directionToLight.Normalized() };
			rayToLight = { hitRecord.origin, directionNormalized };
			rayToLight.min = 0.01f;
			rayToLight.max = directionToLight.Magnitude();
			rayToLight.castsShadow = true;
	
			if (m_ShadowsEnabled && pScene->DoesHit(rayToLight))
				continue;
	
			const float lambertCos{ Vector3::Dot(hitRecord.normal, directionNormalized) };
			if (lambertCos < 0)
				continue;
	
			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				finalColor += ColorRGB({1.f, 1.f, 1.f}) * lambertCos;
				break;
			case LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(currentLight, hitRecord.origin);
				break;
			case LightingMode::BRDF:
				finalColor += materials[hitRecord.materialIndex]->Shade(hitRecord, directionNormalized, -rayDirection.Normalized());
				break;
			case LightingMode::Combined:
				finalColor += LightUtils::GetRadiance(currentLight, hitRecord.origin) * lambertCos *
					materials[hitRecord.materialIndex]->Shade(hitRecord, directionNormalized, -rayDirection.Normalized());
				break;
			}
		}
	}
	
	//Update Color in Buffer
	finalColor.MaxToOne();
	
	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		break;
	case LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		break;
	case LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		break;
	case LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		break;
	default:
		break;
	}
}

#pragma region Level Editing
void dae::Renderer::SelectGeometry(const float x, const float y, Scene* pScene) const
{
	if (!m_EditMode) return;

	Camera& camera = pScene->GetCamera();
	const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	const float directionX = (2 * ((x + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians;
	const float directionY = (1 - 2 * ((y + .5f) / m_Height)) * camera.fovRadians;

	Vector3 rayDirection{ directionX, directionY, 1 };
	rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
	const Ray hitRay{ camera.origin, rayDirection };

	pScene->SelectSphere(hitRay);
}

void dae::Renderer::AddSphere(const float x, const float y, Scene* pScene) const
{
	if (!m_EditMode) return;

	Camera& camera = pScene->GetCamera();
	const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	const float directionX = (2 * ((x + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians;
	const float directionY = (1 - 2 * ((y + .5f) / m_Height)) * camera.fovRadians;

	Vector3 rayDirection{ directionX, directionY, 1 };
	rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
	const Ray hitRay{ camera.origin, rayDirection };
	HitRecord hitRecord{};

	pScene->GetClosestHit(hitRay, hitRecord);
	if (hitRecord.didHit)
	{
		pScene->AddSphereOnClick(hitRecord.origin);
	}
}
#pragma endregion
