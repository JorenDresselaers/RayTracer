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

using namespace dae;

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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float aspectRatio = float(m_Width) / float(m_Height);

	if(m_FunkyMode) pScene->MoveLight(camera.origin + camera.forward*5.f);

#pragma omp parallel for
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				float gradient = px / static_cast<float>(m_Width);
				gradient += py / static_cast<float>(m_Width);
				gradient /= 2.0f;

				const float directionX = (2 * ((px + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians;
				const float directionY = (1 - 2 * ((py + .5f) / m_Height)) * camera.fovRadians;

				Vector3 rayDirection{ directionX, directionY, 1 };
				rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
				Ray hitRay{ camera.origin, rayDirection };
				HitRecord hitRecord{};

				pScene->GetClosestHit(hitRay, hitRecord);

				ColorRGB finalColor{};
				if (hitRecord.didHit)
				{
					//finalColor = materials[hitRecord.materialIndex]->Shade();

					for (const auto& currentLight : pScene->GetLights())
					{
						Vector3 directionToLight{ LightUtils::GetDirectionToLight(currentLight, hitRecord.origin) };
						const float lambertCos{ Vector3::Dot(hitRecord.normal, directionToLight.Normalized()) };

						if (lambertCos > 0)
						{
							finalColor += LightUtils::GetRadiance(currentLight, hitRecord.origin) * lambertCos *
								materials[hitRecord.materialIndex]->Shade();
						}

						if (m_ShadowsEnabled)
						{
							Vector3 directionToLight{ LightUtils::GetDirectionToLight(currentLight, hitRecord.origin) };
							Ray rayToLight{ hitRecord.origin, directionToLight.Normalized() };
							rayToLight.min = 0.01f;
							rayToLight.max = directionToLight.Magnitude();

							if (pScene->DoesHit(rayToLight))
							{
								finalColor *= 0.5f;
							}
						}
					}

					//if (m_ShadowsEnabled)
					//{
					//	for (const auto& currentLight : pScene->GetLights())
					//	{
					//		Vector3 directionToLight{ LightUtils::GetDirectionToLight(currentLight, hitRecord.origin) };
					//		Ray rayToLight{ hitRecord.origin, directionToLight.Normalized() };
					//		rayToLight.min = 0.01f;
					//		rayToLight.max = directionToLight.Magnitude();
					//
					//		if (pScene->DoesHit(rayToLight))
					//		{
					//			finalColor *= 0.5f;
					//		}
					//	}
					//}
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
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
