// CSDK_ChromaGameLoopSample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Razer\ChromaAnimationAPI.h"
#include <array>
#include <chrono>
#include <conio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <thread>

using namespace ChromaSDK;
using namespace std;

const float MATH_PI = 3.14159f;

const char* ANIMATION_SPIRAL = "Animations\\Spiral_Keyboard.chroma";

static bool _sWaitForExit = true;

static int _sMode = 0; //loop

// Function prototypes
void Cleanup();
void GameLoop();
void HandleInput();
void Init();
int main();
void SetupAnimations();

void Init()
{
	if (ChromaAnimationAPI::InitAPI() != 0)
	{
		cerr << "Failed to load Chroma library!" << endl;
		exit(1);
	}
	RZRESULT result = ChromaAnimationAPI::Init();
	if (result != RZRESULT_SUCCESS)
	{
		cerr << "Failed to initialize Chroma!" << endl;
		exit(1);
	}
	Sleep(100); //wait for init
}

void SetupAnimations()
{
}

void GameLoop()
{
	int oldMode = -1;

	const char* baseLayer = "Animations/Spiral_Keyboard.chroma";
	const char* blankLayer = "Animations/Blank_Keyboard.chroma";
	const char* copyName = "Copy/Fade_Keyboard.chroma";

	while (_sWaitForExit)
	{
		if (oldMode != _sMode)
		{
			oldMode = _sMode; //something changed

			if (_sMode == 0) //loop
			{
				// reload from disk
				ChromaAnimationAPI::CloseAnimationName(baseLayer);

				// play animation
				ChromaAnimationAPI::PlayAnimationName(baseLayer, true);
			}
			else if (_sMode == 1) //fade
			{
				// close the old animation
				ChromaAnimationAPI::CloseAnimationName(copyName);

				// get the original frame count
				int frameCount = ChromaAnimationAPI::GetFrameCountName(baseLayer);

				// get the current frame
				int currentFrame = ChromaAnimationAPI::GetCurrentFrameName(baseLayer);

				// remaining frames
				int remaining = frameCount - currentFrame;

				// fade within 20 frames or less
				remaining = min(20, remaining);

				if (remaining > 0)
				{
					// Make a copy of the blank layer
					ChromaAnimationAPI::CopyAnimationName(blankLayer, copyName);

					// make an animation to play the remainder
					ChromaAnimationAPI::MakeBlankFramesName(copyName, remaining, 0.033f, 0);

					for (int targetFrameId = 0, frameId = currentFrame; targetFrameId < remaining; ++targetFrameId, ++frameId)
					{
						// copy frames to animation copy
						ChromaAnimationAPI::CopyNonZeroAllKeysOffsetName(baseLayer, copyName, frameId, targetFrameId - frameId);

						// fade the frames 1 to 0
						float t = (remaining - targetFrameId - 1) / (float)remaining;

						// Set the intensity
						ChromaAnimationAPI::MultiplyIntensityName(copyName, targetFrameId, t);
					}

					// play the copy without looping
					ChromaAnimationAPI::PlayAnimationName(copyName, false);
				}
			}
		}

		Sleep(33); //30 FPS
	}
}

void HandleInput()
{
	while (_sWaitForExit)
	{
		int input = _getch();
		switch (input)
		{
		case 27:
			_sWaitForExit = false;
			break;
		case 'l':
		case 'L':
			_sMode = 0; //loop
			break;
		case 'f':
		case 'F':
			_sMode = 1; //fade
			break;
		}
		Sleep(0);
	}
}

void Cleanup()
{
	ChromaAnimationAPI::StopAll();
	ChromaAnimationAPI::CloseAll();
	RZRESULT result = ChromaAnimationAPI::Uninit();
	if (result != RZRESULT_SUCCESS)
	{
		cerr << "Failed to uninitialize Chroma!" << endl;
		exit(1);
	}
}

int main()
{
	Init();
	SetupAnimations();
	thread thread(GameLoop);
	cout << "Press `ESC` to Quit." << endl;
	cout << "Press `L` for looping." << endl;
	cout << "Press `F` to fade." << endl;
	HandleInput();
	thread.join();
	Cleanup();
	return 0;
}
