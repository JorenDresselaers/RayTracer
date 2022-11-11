#pragma once
#include "Math.h"
#include "DataTypes.h"
#include "BRDFs.h"

namespace dae
{
#pragma region Material BASE
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;

		/**
		 * \brief Function used to calculate the correct color for the specific material and its parameters
		 * \param hitRecord current hitrecord
		 * \param l light direction
		 * \param v view direction
		 * \return color
		 */
		virtual ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) = 0;
	};
#pragma endregion

#pragma region Material SOLID COLOR
	//SOLID COLOR
	//===========
	class Material_SolidColor final : public Material
	{
	public:
		Material_SolidColor(const ColorRGB& color): m_Color(color)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord, const Vector3& l, const Vector3& v) override
		{
			return m_Color;
		}

	private:
		ColorRGB m_Color{colors::White};
	};
#pragma endregion

#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final : public Material
	{
	public:
		Material_Lambert(const ColorRGB& diffuseColor, const float diffuseReflectance) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(diffuseReflectance){}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor);
		}

	private:
		ColorRGB m_DiffuseColor{colors::White};
		float m_DiffuseReflectance{1.f}; //kd
	};
#pragma endregion

#pragma region Material LAMBERT PHONG
	//LAMBERT-PHONG
	//=============
	class Material_LambertPhong final : public Material
	{
	public:
		Material_LambertPhong(const ColorRGB& diffuseColor, const float kd, const float ks, const float phongExponent):
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(kd), m_SpecularReflectance(ks),
			m_PhongExponent(phongExponent)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor) 
				+ BRDF::Phong(m_SpecularReflectance, m_PhongExponent, l, -v, hitRecord.normal);
		}

	private:
		ColorRGB m_DiffuseColor{colors::White};
		float m_DiffuseReflectance{0.5f}; //kd
		float m_SpecularReflectance{0.5f}; //ks
		float m_PhongExponent{1.f}; //Phong Exponent
	};
#pragma endregion

#pragma region Material COOK TORRENCE
	//COOK TORRENCE
	class Material_CookTorrence final : public Material
	{
	public:
		Material_CookTorrence(const ColorRGB& albedo, const bool metalness, const float roughness):
			m_Albedo(albedo), m_Metalness(metalness), m_Roughness(roughness)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			const Vector3 halfVector{ ((v + l) / (l + v).Magnitude()).Normalized()};
			const Vector3 normal{ hitRecord.normal };
			const ColorRGB f0 = (!m_Metalness) ? ColorRGB(0.04f, 0.04f, 0.04f) : m_Albedo;

			const ColorRGB f = BRDF::FresnelFunction_Schlick(halfVector, v, f0);
			const float d = BRDF::NormalDistribution_GGX(normal, halfVector, m_Roughness);
			const float g = BRDF::GeometryFunction_Smith(normal, v, l, m_Roughness);

			const float dots{ 4 * (Vector3::Dot(v, normal) * Vector3::Dot(l, normal)) };
			const auto specular{ (d * f * g) * (1 / dots) };

			const ColorRGB kd = (!m_Metalness) ? ColorRGB( 1.f, 1.f, 1.f ) - f : ColorRGB{ 0.f, 0.f, 0.f };

			const auto diffuse = BRDF::Lambert(kd, m_Albedo);

			return kd * diffuse + specular;
		}

	private:
		ColorRGB m_Albedo{0.955f, 0.637f, 0.538f}; //Copper
		bool m_Metalness{ true };
		float m_Roughness{0.1f}; // [1.0 > 0.0] >> [ROUGH > SMOOTH]
	};
#pragma endregion
}
