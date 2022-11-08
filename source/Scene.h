#pragma once
#include <string>
#include <vector>

#include "Math.h"
#include "DataTypes.h"
#include "Camera.h"

namespace dae
{
	//Forward Declarations
	class Timer;
	class Material;
	struct Plane;
	struct Sphere;
	struct Light;

	//Scene Base Class
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		Camera& GetCamera() { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
		bool DoesHit(const Ray& ray) const;

		const std::vector<Plane>& GetPlaneGeometries() const { return m_PlaneGeometries; }
		const std::vector<Sphere>& GetSphereGeometries() const { return m_SphereGeometries; }
		const std::vector<Light>& GetLights() const { return m_Lights; }
		const std::vector<Material*> GetMaterials() const { return m_Materials; }

		void MoveLight(Vector3 newOrigin);
		void AddSphereOnClick(Vector3 origin);
		void RemoveSphereOnClick(Vector3 origin);
		void DeleteBalls();
		void ToggleFunkyMode()
		{
			ResetSelectedMaterial();
			m_FunkyMode = !m_FunkyMode;
		}
		bool GetFunkyMode()
		{
			return m_FunkyMode;
		}

		void SelectSphere(const Ray& ray);
		void MoveSelectedBall(const Vector3& offset);
		void ResetSelectedMaterial();

	protected:
		std::string	sceneName;

		enum class SelectedGeometry
		{
			Null,
			Sphere,
			Plane
		};
		bool m_FunkyMode{ false };
		SelectedGeometry m_SelectedGeometry{SelectedGeometry::Null};
		int m_SelectedSphereIndex{ -1 };
		int m_OriginalMaterial{ -1 };


		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};
		std::vector<Triangle> m_TriangleGeometries{}; //temporary
		std::vector<Light> m_Lights{};
		std::vector<Material*> m_Materials{};

		Camera m_Camera{};

		Sphere* AddSphere(const Vector3& origin, float radius, unsigned char materialIndex = 0);
		Plane* AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

		Light* AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color);
		Light* AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color);
		unsigned char AddMaterial(Material* pMaterial);
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 1 Test Scene
	class Scene_W1 final : public Scene
	{
	public:
		Scene_W1() = default;
		~Scene_W1() override = default;

		Scene_W1(const Scene_W1&) = delete;
		Scene_W1(Scene_W1&&) noexcept = delete;
		Scene_W1& operator=(const Scene_W1&) = delete;
		Scene_W1& operator=(Scene_W1&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 2 Test Scene
	class Scene_W2 final : public Scene
	{
	public:
		Scene_W2() = default;
		~Scene_W2() override = default;

		Scene_W2(const Scene_W2&) = delete;
		Scene_W2(Scene_W2&&) noexcept = delete;
		Scene_W2& operator=(const Scene_W2&) = delete;
		Scene_W2& operator=(Scene_W2&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 3 Test Scene
	class Scene_W3 final : public Scene
	{
	public:
		Scene_W3() = default;
		~Scene_W3() override = default;

		Scene_W3(const Scene_W3&) = delete;
		Scene_W3(Scene_W3&&) noexcept = delete;
		Scene_W3& operator=(const Scene_W3&) = delete;
		Scene_W3& operator=(Scene_W3&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 3 Test Scene
	class Scene_W4 final : public Scene
	{
	public:
		Scene_W4() = default;
		~Scene_W4() override = default;

		Scene_W4(const Scene_W4&) = delete;
		Scene_W4(Scene_W4&&) noexcept = delete;
		Scene_W4& operator=(const Scene_W4&) = delete;
		Scene_W4& operator=(Scene_W4&&) noexcept = delete;

		void Initialize() override;
	};
}
