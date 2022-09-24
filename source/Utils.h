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
			//todo W1
			//assert(false && "No Implemented Yet!");
			const float a{ float(Vector3::Dot(ray.direction, ray.direction)) };
			const float b{ float(Vector3::Dot(2*ray.direction, ray.origin - sphere.origin)) };
			const float c{ float(Vector3::Dot(ray.origin - sphere.origin, ray.origin - sphere.origin) - pow(sphere.radius, 2)) };
			const float discriminant{ b*b - 4 * a * c };

			//const float tMax{ (-b + sqrt(discriminant)) / (2 * a) };
			const float t{ (-b - sqrt(discriminant)) / (2 * a) };

			if (discriminant < 0.f)
			{
				return false;
			}

			if (t > ray.min && t < ray.max)
			{
				//const float p1{ (-b + sqrt(discriminant)) / (2 * a) };
				//const float p2{ (-b - sqrt(discriminant)) / (2 * a) };

				if (!ignoreHitRecord)
				{
					//hitRecord.origin = t * ray.origin * ray.direction;
					hitRecord.t = t;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.didHit = true;
					//hitRecord.normal = sphere.origin
				}
				return true;
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
			const float t{ Vector3::Dot((plane.origin - ray.origin), plane.normal) / Vector3::Dot(ray.direction, plane.normal)};

			if (t > ray.min && t < ray.max)
			{
				const Vector3 intersectionPoint{ ray.origin + (t * ray.direction) };
				if (!ignoreHitRecord)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.normal = plane.normal;
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
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
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
			//todo W3
			assert(false && "No Implemented Yet!");
			return {};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			assert(false && "No Implemented Yet!");
			return {};
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