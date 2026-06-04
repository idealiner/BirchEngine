#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include <iostream>

class Game
{
public:
	Game();
	~Game();

	void init(const char* title, int width, int height, bool fullscreen);

	void handleEvents();
	void update();
	bool running() { return isRunning; }
	void render();
	void clean();

private:
	struct InputState
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
		bool jumpPressed = false;
		bool pausePressed = false;
		bool restartPressed = false;
	};

	void ResetInputEdges();
	void UpdateDirectionalInput();
	void DismissSplash();

	bool isRunning = false;
	int cnt = 0;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *splashTexture = nullptr;
	InputState input;
	bool virtualLeftHeld = false;
	bool virtualRightHeld = false;
	bool virtualJumpQueued = false;
	bool splashActive = true;
	int splashFrames = 180;
};