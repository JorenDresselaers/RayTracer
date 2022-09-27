#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin }
		{
			SetFOV(_fovAngle);
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fovRadians{ tan((90 * TO_RADIANS) / 2) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		void SetFOV(float newFOV)
		{
			fovAngle = newFOV;
			fovRadians = tan((fovAngle * TO_RADIANS) / 2);
		}

		Matrix CalculateCameraToWorld()
		{
			Matrix matrixToReturn;
			//todo: W2
			//assert(false && "Not Implemented Yet");
			return matrixToReturn;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//SDL_GetRelativeMouseMode(); //used for locking the mouse to center of screen
			//SDL_SetRelativeMouseMode(true);

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
