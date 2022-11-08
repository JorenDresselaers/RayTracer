#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <iostream>

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float a{ float(Vector3::Dot(ray.direction, ray.direction)) };
			const float b{ float(Vector3::Dot(2*ray.direction, ray.origin - sphere.origin)) };
			const float c{ float(Vector3::Dot(ray.origin - sphere.origin, ray.origin - sphere.origin) - sphere.radius * sphere.radius) };
			const float discriminant{ b*b - 4 * a * c };

			//const float tMax{ (-b + sqrt(discriminant)) / (2 * a) };

			if (discriminant < 0.f)
			{
				return false;
			}

			else
			{
				const float t{ (-b - sqrt(discriminant)) / (2 * a) };
				if (t > ray.min && t < ray.max)
				{
					//const float p1{ (-b + sqrt(discriminant)) / (2 * a) };
					//const float p2{ (-b - sqrt(discriminant)) / (2 * a) };

					if (!ignoreHitRecord)
					{
						hitRecord.origin = ray.origin + t * ray.direction;
						hitRecord.t = t;
						hitRecord.materialIndex = sphere.materialIndex;
						hitRecord.didHit = true;
						hitRecord.normal = Vector3{ hitRecord.origin - sphere.origin }.Normalized();
					}
					return true;
				}
			}

			//const float result{ (-b + sqrt(discriminant)) / (2 * a) };

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float t{ Vector3::Dot((plane.origin - ray.origin), plane.normal) / Vector3::Dot(ray.direction, plane.normal) };

			if (t > ray.min && t < ray.max)
			{
				const Vector3 intersectionPoint{ ray.origin + (t * ray.direction) };
				if (!ignoreHitRecord)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.normal = plane.normal;
					hitRecord.origin = intersectionPoint;
				}
				
				//std::cout << "\n" << t;
				return true;
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//checking if the ray intersects the triangle's plane
			Vector3 a{ triangle.v1 - triangle.v0 };
			Vector3 b{ triangle.v2 - triangle.v1 };
			Vector3 c{ triangle.v0 - triangle.v2 };
			Vector3 normal{ Vector3::Cross(a, b) };
			if (Vector3::Dot(normal, ray.direction) == 0) return false;

			//calculating the intersection point is within the ray's range
			Vector3 triangleCenter{ (triangle.v0 + triangle.v1 + triangle.v2) / 3 };
			Vector3 L{ triangleCenter - ray.origin };
			float t{ Vector3::Dot(L, normal) / Vector3::Dot(ray.direction, normal) };
			if (t < ray.min || t > ray.max) return false;

			//checking if the intersection is within the triangle
			Vector3 p{ ray.origin + t * ray.direction};
			Vector3 pointToSide{ p - triangle.v0 };
			Vector3 pointToSideB{ p - triangle.v1 };
			Vector3 pointToSideC{ p - triangle.v2 };
			if (Vector3::Dot(normal, Vector3::Cross(a, pointToSide)) < 0.f) return false;
			if (Vector3::Dot(normal, Vector3::Cross(b, pointToSideB)) < 0.f) return false;
			if (Vector3::Dot(normal, Vector3::Cross(c, pointToSideC)) < 0.f) return false;

			switch (triangle.cullMode)
			{
			case dae::TriangleCullMode::BackFaceCulling:
				if (Vector3::Dot(normal, ray.direction) > 0.f) return false;
				break;
			case dae::TriangleCullMode::FrontFaceCulling:
				if (Vector3::Dot(normal, ray.direction) < 0.f) return false;
				break;
			default:
				break;
			}

			hitRecord.didHit = true;
			hitRecord.normal = normal;
			hitRecord.origin = p;
			hitRecord.t = t;
			hitRecord.materialIndex = triangle.materialIndex;
			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			return Vector3{ light.origin - origin };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			return 	ColorRGB{ light.color * (light.intensity / (light.origin - target).SqrMagnitude()) };
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}