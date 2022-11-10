//External includes
#include "vld.h"
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;
	

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"RayTracer - Dresselaers Joren",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//const auto pScene = new Scene_W4_BunnyScene();
	const auto pScene = new Scene_W4_ReferenceScene();
	pScene->Initialize();

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		int mouseX{}, mouseY{};
		const uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if(e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;
				if(e.key.keysym.scancode == SDL_SCANCODE_F2)
					pRenderer->ToggleShadows();
				if(e.key.keysym.scancode == SDL_SCANCODE_F3)
					pRenderer->CycleLightingMode();
				if(e.key.keysym.scancode == SDL_SCANCODE_F4)
					pScene->DeleteBalls();
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					pScene->ToggleFunkyMode();
					pRenderer->ToggleFunkyMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_1)
				{
					pScene->MoveSelectedBall(Vector3(0.f, 1.f, 0.f));
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_2)
				{
					pScene->MoveSelectedBall(Vector3(0.f, -1.f, 0.f));
				}				
				if (e.key.keysym.scancode == SDL_SCANCODE_3)
				{
					pScene->MoveSelectedBall(Vector3(1.f, 0.f, 0.f));
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_4)
				{
					pScene->MoveSelectedBall(Vector3(-1.f, 0.f, 0.f));
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_5)
				{
					pScene->MoveSelectedBall(Vector3(0.f, 0.f, 1.f));
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_6)
				{
					pScene->MoveSelectedBall(Vector3(0.f, 0.f, -1.f));
				}
				break;
			}
		}

		if (mouseState & SDL_BUTTON_LMASK)
		{
			pRenderer->SelectBall(static_cast<float>(mouseX), static_cast<float>(mouseY), pScene);
		}

		if (mouseState & SDL_BUTTON_MMASK)
		{
			pRenderer->AddBall(static_cast<float>(mouseX), static_cast<float>(mouseY), pScene);
		}

		if (mouseState & SDL_BUTTON_RMASK)
		{
			pRenderer->RemoveBall(static_cast<float>(mouseX), static_cast<float>(mouseY), pScene);
		}

		//--------- Update ---------
		pScene->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render(pScene);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if (pScene->GetFunkyMode())
			{
				std::cout << "dFPS: " << pTimer->GetdFPS() + 120.f << std::endl;
			}
			else
			{
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pScene;
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}