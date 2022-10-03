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
		float testDontNoticeThis{ 1.f };

		Vector3 forward{Vector3::UnitZ};
		//Vector3 forward{0.266f, -0.453f, 0.860f}; //hardcoded to check
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		float movementSpeed{ 1.f };
		float rotationSpeed{ 0.5f };
		float mouseSensitivity{ 0.2f };

		Matrix cameraToWorld{};

		void SetFOV(float newFOV)
		{
			fovAngle = newFOV;
			fovRadians = tan((fovAngle * TO_RADIANS) / 2);
		}

		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			//Matrix matrixToReturn
			//{
			//	{right.x,	right.y,	right.z,	0},
			//	{up.x,		up.y,		up.z,		0},
			//	{forward.x, forward.y,	forward.z,	0},
			//	{origin.x,	origin.y,	origin.z,	1}
			//};


			Matrix matrixToReturn
			{
				right,
				up,
				forward,
				origin
			};

			//todo: W2
			//assert(false && "Not Implemented Yet");
			return matrixToReturn;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			float shiftModifier{ 1.f };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				shiftModifier = 4.f;
			}

			if (pKeyboardState[SDL_SCANCODE_T])
			{
				testDontNoticeThis += 0.1f;
			}

			if (pKeyboardState[SDL_SCANCODE_Y])
			{
				testDontNoticeThis -= 0.1f;
			}

			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin.x += movementSpeed * deltaTime * shiftModifier;
			}

			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin.x -= movementSpeed * deltaTime * shiftModifier;
			}

			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin.z -= movementSpeed * deltaTime * shiftModifier;
			}

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin.z += movementSpeed * deltaTime * shiftModifier;
			}

			if (pKeyboardState[SDL_SCANCODE_LEFT])
			{
				if(fovAngle > 2)
					SetFOV(fovAngle - 1);
			}

			if (pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				if (fovAngle < 178)
					SetFOV(fovAngle + 1);
			}



			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			float yawAngle{};
			float pitchAngle{};

			Matrix finalRotation{};

			if (mouseState & SDL_BUTTON_RMASK && mouseState & SDL_BUTTON_LMASK)
			{
				if (mouseX != 0)
				{
					origin.x += mouseX * mouseSensitivity * deltaTime;
				}

				if (mouseY != 0)
				{
					origin.y -= mouseY * mouseSensitivity * deltaTime;
				}
			}

			else
			{
				if (mouseState & SDL_BUTTON_RMASK)
				{
					if (mouseX != 0)
					{
						yawAngle -= mouseX * deltaTime * mouseSensitivity;
					}

					if (mouseY != 0)
					{
						pitchAngle -= mouseY * deltaTime * mouseSensitivity;
					}

					finalRotation = Matrix::CreateRotation(pitchAngle, yawAngle, 0);

					forward = finalRotation.TransformVector(forward);
					forward.Normalize();
				}

				if (mouseState & SDL_BUTTON_LMASK)
				{
					if (mouseX != 0)
					{
						yawAngle -= mouseX * deltaTime * mouseSensitivity*0.1f;
					}

					if (mouseY != 0)
					{
						origin -= mouseY * deltaTime * mouseSensitivity * forward;
					}
					finalRotation = Matrix::CreateRotation(0, yawAngle, 0);

					forward = finalRotation.TransformVector(forward);
					forward.Normalize();
				}
			}

			//SDL_GetRelativeMouseMode(); //used for locking the mouse to center of screen
			//SDL_SetRelativeMouseMode(true);
		}
	};
}
