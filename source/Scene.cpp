#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{

		//spheres
		for (const Sphere& currentSphere : m_SphereGeometries)
		{
			HitRecord tempRecord;
			if (GeometryUtils::HitTest_Sphere(currentSphere, ray, tempRecord))
			{
				if (tempRecord.t < closestHit.t) closestHit = tempRecord;
			}
		}

		//planes
		for (const Plane& currentPlane : m_PlaneGeometries)
		{
			HitRecord tempRecord;
			if (GeometryUtils::HitTest_Plane(currentPlane, ray, tempRecord))
			{
				if (tempRecord.t < closestHit.t) closestHit = tempRecord;
			}
		}

		//triangles
		//for (const Triangle& currentTriangle : m_TriangleGeometries)
		//{
		//	HitRecord tempRecord;
		//	if (GeometryUtils::HitTest_Triangle(currentTriangle, ray, tempRecord))
		//	{
		//		if (tempRecord.t < closestHit.t) closestHit = tempRecord;
		//	}
		//}
		
		//triangles meshes
		for (const TriangleMesh& currentMesh : m_TriangleMeshGeometries)
		{
			HitRecord tempRecord;
			if (GeometryUtils::HitTest_TriangleMesh(currentMesh, ray, tempRecord))
			{
				if (tempRecord.t < closestHit.t) closestHit = tempRecord;
			}
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		//spheres
		for (const Sphere& currentSphere : m_SphereGeometries)
		{
			if (GeometryUtils::HitTest_Sphere(currentSphere, ray))
			{
				return true;
			}
		}

		//planes
		for (const Plane& currentPlane : m_PlaneGeometries)
		{
			if (GeometryUtils::HitTest_Plane(currentPlane, ray))
			{
				return true;
			}
		}

		for (const TriangleMesh& currentMesh : m_TriangleMeshGeometries)
		{
			if (GeometryUtils::HitTest_TriangleMesh(currentMesh, ray))
			{
				return true;
			}
		}

		//for (const Triangle& currentTriangle : m_TriangleGeometries)
		//{
		//	if (GeometryUtils::HitTest_Triangle(currentTriangle, ray))
		//	{
		//		return true;
		//	}
		//}

		return false;
	}

#pragma region Level Editing
	void Scene::DeleteBalls()
	{
		m_SphereGeometries.clear();
	}

	void Scene::SelectSphere(const Ray& ray)
	{
		ResetSelectedMaterial();
		m_SelectedGeometry = SelectedGeometry::Null;
		HitRecord tempRecord, closestHit;
		for (int currentSphere{0}; currentSphere < m_SphereGeometries.size(); ++currentSphere)
		{
			if (GeometryUtils::HitTest_Sphere(m_SphereGeometries.at(currentSphere), ray))
			{
				unsigned char randomMaterial{ static_cast<unsigned char> (rand() % m_Materials.size()) };
				m_SelectedSphereIndex = currentSphere;
				m_OriginalMaterial = m_SphereGeometries.at(currentSphere).materialIndex;
				m_SphereGeometries.at(currentSphere).materialIndex = randomMaterial;
				m_SelectedGeometry = SelectedGeometry::Sphere;
				return;
			}
		}

		if (m_SelectedGeometry != SelectedGeometry::Null) return;

		int closestPlaneIndex{ -1 };
		for (int currentPlane{0}; currentPlane < m_PlaneGeometries.size(); ++currentPlane)
		{
			if (GeometryUtils::HitTest_Plane(m_PlaneGeometries.at(currentPlane), ray, tempRecord))
			{
				if (tempRecord.t < closestHit.t)
				{
					closestHit = tempRecord;
					closestPlaneIndex = currentPlane;
				}
			}
		}

		if (closestPlaneIndex != -1)
		{
			unsigned char randomMaterial{ static_cast<unsigned char> (rand() % m_Materials.size()) };
			m_SelectedSphereIndex = closestPlaneIndex;
			m_OriginalMaterial = m_PlaneGeometries.at(closestPlaneIndex).materialIndex;
			m_PlaneGeometries.at(closestPlaneIndex).materialIndex = randomMaterial;
			m_SelectedGeometry = SelectedGeometry::Plane;
		}
	}

	void Scene::MoveSelectedBall(const Vector3& offset)
	{
		switch (m_SelectedGeometry)
		{
		case dae::Scene::SelectedGeometry::Sphere:
			m_SphereGeometries.at(m_SelectedSphereIndex).origin += offset;
			break;
		case dae::Scene::SelectedGeometry::Plane:
			m_PlaneGeometries.at(m_SelectedSphereIndex).origin += offset;
			break;
		default:
			break;
		}
	}

	void Scene::ResetSelectedMaterial()
	{
		switch (m_SelectedGeometry)
		{
		case dae::Scene::SelectedGeometry::Sphere:
			if (m_OriginalMaterial != -1) m_SphereGeometries.at(m_SelectedSphereIndex).materialIndex = m_OriginalMaterial;
			break;
		case dae::Scene::SelectedGeometry::Plane:
			if (m_OriginalMaterial != -1) m_PlaneGeometries.at(m_SelectedSphereIndex).materialIndex = m_OriginalMaterial;
			break;
		default:
			break;
		}
	}

	void Scene::RemoveSphereOnClick(Vector3 origin)
	{
		for (size_t currentSphere{ 0 }; currentSphere < m_SphereGeometries.size(); ++currentSphere)
		{
			Vector3 currentOrigin = m_SphereGeometries.at(currentSphere).origin;
			if (currentOrigin.x == origin.x &&
				currentOrigin.y == origin.y &&
				currentOrigin.z == origin.z)
			{
				std::swap(m_SphereGeometries.at(currentSphere), m_SphereGeometries.back());
				m_SphereGeometries.pop_back();
			}
		}
	}

	void Scene::MoveLight(Vector3 newOrigin)
	{
		m_Lights.front().origin = newOrigin;
	}

	void Scene::AddSphereOnClick(Vector3 origin)
	{
		unsigned char randomMaterial{ static_cast<unsigned char> (rand() % m_Materials.size()) };
		AddSphere(origin, 1.f, randomMaterial);
	}
#pragma endregion

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });
		const unsigned char matId_Solid_Gray = AddMaterial(new Material_SolidColor{ colors::Gray });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);
		//AddSphere({ 0.f, 25.f, 100.f }, 50.f, matId_Solid_Blue);
		//AddSphere({ 0.f, -25.f, 100.f }, 50.f, matId_Solid_Blue);
		//AddSphere({ 0.f, 0.f, 100.f }, 50.f, matId_Solid_Gray);
		//AddSphere({ 10.f, 0.f, 100.f }, 50.f, matId_Solid_Yellow);
		//AddSphere({ -10.f, 0.f, 100.f }, 50.f, matId_Solid_Yellow);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f, 0.f}, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f, 0.f}, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f, 0.f}, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f, 0.f}, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f, -1.f}, matId_Solid_Magenta);
	}
#pragma endregion
#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.SetFOV(45.f);

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });
		const unsigned char matId_Solid_Gray = AddMaterial(new Material_SolidColor{ colors::Gray });

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f}, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f}, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f}, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f}, matId_Solid_Magenta);

		//Lights
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
		//AddPointLight({ 0.f, 5.f, -10.f }, 70.f, colors::White);
		//AddPointLight({ 5.f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.SetFOV(45.f);

		
		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, true, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, true, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .75f, .915f }, true, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//Plane
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BACKLIGHT
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //FRONT LEFT
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f }); //BACK LEFT
		

		//m_Camera.origin = { 0.f, 1.f, -5.f };
		//
		//const auto matLambert_Red = AddMaterial(new Material_Lambert({ colors::Red }, 1.f));;
		//const auto matLambert_Blue = AddMaterial(new Material_Lambert({ colors::Blue }, 1.f));
		//const auto matLambert_Yellow = AddMaterial(new Material_Lambert({ colors::Yellow }, 1.f));
		//const auto matLambert_White = AddMaterial(new Material_Lambert({ colors::White }, 1.f));
		//const auto matLambert_Magenta = AddMaterial(new Material_Lambert({ colors::Magenta }, 1.f));
		//const auto matLambert_Cyan = AddMaterial(new Material_Lambert({ colors::Cyan }, 1.f));
		//const auto matLambert_Gray = AddMaterial(new Material_Lambert({ colors::Gray }, 1.f));
		//const auto matLambert_Green = AddMaterial(new Material_Lambert({ colors::Green }, 1.f));
		//const auto matLambertPhong_Red = AddMaterial(new Material_LambertPhong({ colors::Red }, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Blue = AddMaterial(new Material_LambertPhong({ colors::Blue }, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Yellow = AddMaterial(new Material_LambertPhong({ colors::Yellow }, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_White = AddMaterial(new Material_LambertPhong({ colors::White }, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Magenta = AddMaterial(new Material_LambertPhong({ colors::Magenta }, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Cyan = AddMaterial(new Material_LambertPhong({ colors::Cyan }, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Gray = AddMaterial(new Material_LambertPhong({ colors::Gray }, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Green = AddMaterial(new Material_LambertPhong({ colors::Green }, 1.f, 1.f, 60.f));
		//
		//AddSphere({ -.75f, 1.f, 0.f }, 1.f, matLambert_Red);
		//AddSphere({ .75f, 1.f, 0.f }, 1.f, matLambertPhong_Blue);
		////AddSphere({ 0.f, 5.f, 5.f }, .5f, matLambertPhong_Gray);
		////AddSphere({ 0.f, 2.5f, -5.f }, 5.f, matLambertPhong_Magenta);
		//
		//AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_Yellow);
		//
		//AddPointLight({ 0.f, 5.f, 5.f }, 25.f, colors::White);
		//AddPointLight({ 0.f, 2.5f, -5.f }, 25.f, colors::White);
	}
	void Scene_W4::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.SetFOV(45.f);
		m_Camera.totalYaw = 0;


		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, true, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, true, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .75f, .915f }, true, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));

		const auto matLambert_Red = AddMaterial(new Material_Lambert({ colors::Red }, 1.f));;
		const auto matLambert_Blue = AddMaterial(new Material_Lambert({ colors::Blue }, 1.f));
		const auto matLambert_Yellow = AddMaterial(new Material_Lambert({ colors::Yellow }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert({ colors::White }, 1.f));
		const auto matLambert_Magenta = AddMaterial(new Material_Lambert({ colors::Magenta }, 1.f));
		const auto matLambert_Cyan = AddMaterial(new Material_Lambert({ colors::Cyan }, 1.f));
		const auto matLambert_Gray = AddMaterial(new Material_Lambert({ colors::Gray }, 1.f));
		const auto matLambert_Green = AddMaterial(new Material_Lambert({ colors::Green }, 1.f));
		const auto matLambertPhong_Red = AddMaterial(new Material_LambertPhong({ colors::Red }, 1.f, 1.f, 60.f));
		const auto matLambertPhong_Blue = AddMaterial(new Material_LambertPhong({ colors::Blue }, 1.f, 1.f, 60.f));
		const auto matLambertPhong_Yellow = AddMaterial(new Material_LambertPhong({ colors::Yellow }, 1.f, 1.f, 60.f));
		const auto matLambertPhong_White = AddMaterial(new Material_LambertPhong({ colors::White }, 1.f, 1.f, 60.f));
		const auto matLambertPhong_Magenta = AddMaterial(new Material_LambertPhong({ colors::Magenta }, 1.f, 1.f, 60.f));
		const auto matLambertPhong_Cyan = AddMaterial(new Material_LambertPhong({ colors::Cyan }, 1.f, 1.f, 60.f));
		const auto matLambertPhong_Gray = AddMaterial(new Material_LambertPhong({ colors::Gray }, 1.f, 1.f, 60.f));
		const auto matLambertPhong_Green = AddMaterial(new Material_LambertPhong({ colors::Green }, 1.f, 1.f, 60.f));

		//const auto triangleMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_Gray);
		//triangleMesh->positions = { {-.75f, -1.f, .0f}, {-.75f, 1.f, 0.f}, {.75f, 1.f, 1.f}, {.75f, -1.f, 0.f} };
		//triangleMesh->indices = {
		//	0, 1, 2,
		//	0, 2, 3
		//};
		//m_pMeshesVector.emplace_back(triangleMesh);
		//
		//triangleMesh->CalculateNormals();
		//
		//triangleMesh->Translate({ 0.f, 1.5f, 0.f });
		//triangleMesh->RotateY(45.f);
		//
		//triangleMesh->UpdateTransforms();

		const auto cubeMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_Gray);
		Utils::ParseOBJ("Resources/simple_cube.obj",
			cubeMesh->positions,
			cubeMesh->normals,
			cubeMesh->indices);

		cubeMesh->Scale({ .7f, .7f, .7f });
		cubeMesh->Translate({ .0f, 1.f, 0.f });
		cubeMesh->UpdateTransforms();

		m_pMeshesVector.emplace_back(cubeMesh);

		//AddSphere({ -.75f, .5f, .0f }, 0.1f, matLambert_Red);
		//AddSphere({ -.75f, 2.f, .0f }, 0.1f, matLambert_Blue);
		//AddSphere({ .75f, .5f, .0f }, 0.1f, matLambert_Green);

		//Plane
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BACKLIGHT
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //FRONT LEFT
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f }); //BACK LEFT
	}
	void Scene_W4::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);
		for (TriangleMesh* currentMesh : m_pMeshesVector)
		{
			if (currentMesh)
			{
				currentMesh->RotateY(PI_DIV_2 * pTimer->GetTotal());
				currentMesh->UpdateTransforms();
			}
		}
	}
}
