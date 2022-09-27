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

	//Sphere testSphere{ Vector3{0,0,100}, 50.f, 0 };
	//Plane testPlane{ {0.f, -50.f, 0.f }, { 0.f, 1.f, 0.f }, 0 };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			float directionX = (2 * ((px + 0.5) / m_Width) - 1) * aspectRatio * camera.fovRadians;
			float directionY = (1 - 2 * ((py + .5) / m_Height)) * camera.fovRadians;

			Vector3 rayDirection{directionX, directionY, 1};
			Ray hitRay{ camera.origin, rayDirection };
			HitRecord hitRecord{};

			//ColorRGB finalColor{ gradient, gradient, gradient };
			//ColorRGB finalColor{ rayDirection.x, rayDirection.y, rayDirection.z};
			//Sphere testSphere{ {0,0,100}, 50.f, 0 };
			//GeometryUtils::HitTest_Sphere(testSphere, hitRay, hitRecord);
			
			
			//GeometryUtils::HitTest_Plane(testPlane, hitRay, hitRecord);

			pScene->GetClosestHit(hitRay, hitRecord);

			ColorRGB finalColor{};
			if (hitRecord.didHit)
			{
				//const float scaled_t = (hitRecord.t - 50.f) / 40.f;

				//t-value visualization
				//const float scaled_t = hitRecord.t / 500.f;
				//finalColor = { scaled_t, scaled_t, scaled_t };

				//hit visualization
				finalColor = materials[hitRecord.materialIndex]->Shade();
				//finalColor *= scaled_t;
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
