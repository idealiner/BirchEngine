#pragma once

#include "Game.h"

class GameObject {

public:
	GameObject(const char* texturesheet, SDL_Renderer* ren, int x, int y, int frames = 1);
	~GameObject();

	void Update();
	void Render();
	void SetVelocity(int x, int y);
	void SetGroundY(int gy);
	void Jump();
	void SetPosition(int x, int y);
	SDL_Rect GetBounds() const;

private:

	int xpos;
	int ypos;
	int xvel;
	int yvel;

	float jumpVel;
	bool  onGround;
	int   groundY;
	bool  facingLeft;

	int frameCount;
	int currentFrame;
	int frameTimer;
	static const int FRAME_DELAY = 6;

	SDL_Texture* objTexture;
	SDL_Rect srcRect, destRect;
	SDL_Renderer* renderer;

};