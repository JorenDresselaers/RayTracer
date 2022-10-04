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

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
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

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			float directionX = (2 * ((px + 0.5f) / m_Width) - 1) * aspectRatio * camera.fovRadians;
			float directionY = (1 - 2 * ((py + .5f) / m_Height)) * camera.fovRadians;

			Vector3 rayDirection{ directionX, directionY, 1 };
			rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
			Ray hitRay{ camera.origin, rayDirection };
			HitRecord hitRecord{};

			pScene->GetClosestHit(hitRay, hitRecord);

			ColorRGB finalColor{};
			if (hitRecord.didHit)
			{
				finalColor = materials[hitRecord.materialIndex]->Shade();

				for (const auto& currentLight : pScene->GetLights())
				{
					Vector3 directionToLight{ LightUtils::GetDirectionToLight(currentLight, hitRecord.origin) };
					Ray rayToLight{ hitRecord.origin, directionToLight.Normalized()};
					rayToLight.min = 0.01f;
					//rayToLight.max = directionToLight.Magnitude();
					rayToLight.max = directionToLight.Magnitude();

					if (pScene->DoesHit(rayToLight))
					{
						finalColor *= 0.5f;
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
	}
	

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
