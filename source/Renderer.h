#pragma once

#include <cstdint>
#include <vector>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;
	struct Camera;
	struct Light;
	class Material;
	struct Matrix;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;
		void RenderPixel(const Scene* pScene, int pixelIndex, float aspectRatio, const Camera& camera,
		                 Matrix cameraToWorld, const std::vector<Light>& lights, const std::vector<Material*>& materials) const;

		void CycleLightingMode();
		void ToggleShadows()
		{
			m_ShadowsEnabled = !m_ShadowsEnabled;
		}

		void ToggleEditMode()
		{
			m_EditMode = !m_EditMode;
		}

		void AddSphere(float x, float y, Scene* pScene) const;
		void SelectGeometry(float x, float y, Scene* pScene) const;

	private:
		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true }, m_EditMode{ false };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};
	};
}
