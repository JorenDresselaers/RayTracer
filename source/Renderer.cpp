//External includes
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
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float aspectRatio = float(m_Width) / float(m_Height);

	const uint32_t numPixels{ static_cast<uint32_t>(m_Width) * static_cast<uint32_t>(m_Height) };

	//if(m_FunkyMode) pScene->MoveLight(camera.origin + camera.forward*5.f);

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
						RenderPixel(pScene, pixelIndex, camera.fovAngle, aspectRatio, camera, cameraToWorld, lights, materials);
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
			RenderPixel(pScene, pixelIndex, camera.fovAngle, aspectRatio, camera, cameraToWorld, lights, materials);
		});
#else
	for (uint32_t i{ 0 }; i < numPixels; ++i)
	{
		RenderPixel(pScene, i, camera.fovAngle, aspectRatio, camera, cameraToWorld, lights, materials);
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

void dae::Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera,
	Matrix cameraToWorld, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px{ static_cast<int>(pixelIndex) % m_Width };
	const int py{ static_cast<int>(pixelIndex) / m_Width };

	float gradient = px / static_cast<float>(m_Width);
	gradient += py / static_cast<float>(m_Width);
	gradient /= 2.0f;
	
	const float directionX = (2 * ((px + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians;
	const float directionY = (1 - 2 * ((py + .5f) / m_Height)) * camera.fovRadians;
	
	Vector3 rayDirection{ directionX, directionY, 1 };
	rayDirection = cameraToWorld.TransformVector(rayDirection);
	Ray hitRay{ camera.origin, rayDirection };
	HitRecord hitRecord{};
	
	pScene->GetClosestHit(hitRay, hitRecord);
	
	ColorRGB finalColor{};
	if (hitRecord.didHit)
	{
		//finalColor = materials[hitRecord.materialIndex]->Shade();
	
		for (const auto& currentLight : lights)
		{
			Vector3 directionToLight{ LightUtils::GetDirectionToLight(currentLight, hitRecord.origin) };
			Ray rayToLight{ hitRecord.origin, directionToLight.Normalized() };
			rayToLight.min = 0.01f;
			rayToLight.max = directionToLight.Magnitude();
	
			if (m_ShadowsEnabled && pScene->DoesHit(rayToLight))
				continue;
	
			const float lambertCos{ Vector3::Dot(hitRecord.normal, directionToLight.Normalized()) };
			if (lambertCos < 0)
				continue;
	
			finalColor += LightUtils::GetRadiance(currentLight, hitRecord.origin) * lambertCos *
				materials[hitRecord.materialIndex]->Shade(hitRecord, directionToLight.Normalized(), rayDirection);
		}
	}
	
	//Update Color in Buffer
	finalColor.MaxToOne();
	
	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

void dae::Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		break;
	case dae::Renderer::LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		break;
	case dae::Renderer::LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		break;
	case dae::Renderer::LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		break;
	default:
		break;
	}
}

//test
void dae::Renderer::AddBall(float x, float y, Scene* pScene)
{
	if (!m_FunkyMode) return;

	Camera& camera = pScene->GetCamera();
	float aspectRatio = float(m_Width) / float(m_Height);

	float directionX = (2 * ((x + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians;
	float directionY = (1 - 2 * ((y + .5f) / m_Height)) * camera.fovRadians;

	Vector3 rayDirection{ directionX, directionY, 1 };
	rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
	Ray hitRay{ camera.origin, rayDirection };
	HitRecord hitRecord{};

	pScene->GetClosestHit(hitRay, hitRecord);

	if (hitRecord.didHit)
	{
		pScene->AddSphereOnClick(hitRecord.origin);
	}
}

void dae::Renderer::RemoveBall(float x, float y, Scene* pScene)
{
	if (!m_FunkyMode) return;

	Camera& camera = pScene->GetCamera();
	float aspectRatio = float(m_Width) / float(m_Height);

	float directionX = (2 * ((x + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians;
	float directionY = (1 - 2 * ((y + .5f) / m_Height)) * camera.fovRadians;

	Vector3 rayDirection{ directionX, directionY, 1 };
	rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
	Ray hitRay{ camera.origin, rayDirection };
	HitRecord hitRecord{};

	pScene->GetClosestHit(hitRay, hitRecord);

	if (hitRecord.didHit)
	{
		pScene->RemoveSphereOnClick(hitRecord.origin);
	}
}
