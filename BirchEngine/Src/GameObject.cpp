#include "GameObject.h"
#include "TextureManager.h"

GameObject::GameObject(const char* texturesheet, SDL_Renderer* ren, int x, int y, int frames)
{
	renderer = ren;
	objTexture = TextureManager::LoadTexture(texturesheet, ren);

	xpos = x;
	ypos = y;
	xvel = 0;
	yvel = 0;

	frameCount   = frames;
	currentFrame = 0;
	frameTimer   = 0;

	jumpVel  = 0.0f;
	onGround = true;
	canDoubleJump = false;
	groundY  = 0;
	facingLeft = false;
}

GameObject::~GameObject()
{
	if (objTexture)
	{
		SDL_DestroyTexture(objTexture);
		objTexture = nullptr;
	}
}

void GameObject::Update()
{
	xpos += xvel;

	if (xvel < 0)
	{
		facingLeft = true;
	}
	else if (xvel > 0)
	{
		facingLeft = false;
	}

	// physics-based vertical when groundY is set
	if (groundY > 0)
	{
		jumpVel += 0.5f;   // gravity
		ypos    += (int)jumpVel;

		if (ypos >= groundY)
		{
			ypos = groundY;
			if (jumpVel > 3.0f)   // still bouncing
			{
				jumpVel *= -0.45f;
			}
			else
			{
				jumpVel  = 0.0f;
				onGround = true;
				canDoubleJump = false;
			}
		}
	}
	else
	{
		ypos += yvel;
	}

	// advance animation only while moving, reset to idle frame when still
	if (frameCount > 1)
	{
		if (xvel != 0 || !onGround)
		{
			frameTimer++;
			if (frameTimer >= FRAME_DELAY)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % frameCount;
			}
		}
		else
		{
			currentFrame = 0;
			frameTimer   = 0;
		}
	}

	srcRect.h = 150;
	srcRect.w = 150;
	srcRect.x = currentFrame * 150;
	srcRect.y = 0;

	destRect.x = xpos;
	destRect.y = ypos;
	destRect.w = srcRect.w;
	destRect.h = srcRect.h;
}

void GameObject::SetVelocity(int x, int y)
{
	xvel = x;
	yvel = y;
}

void GameObject::SetGroundY(int gy)
{
	groundY = gy;
}

void GameObject::Jump()
{
	if (onGround)
	{
		jumpVel       = -13.0f;
		onGround      = false;
		canDoubleJump = true;
	}
	else if (canDoubleJump)
	{
		jumpVel       = -13.0f;
		canDoubleJump = false;
	}
}

void GameObject::SetPosition(int x, int y)
{
	xpos = x;
	ypos = y;
}

SDL_Rect GameObject::GetBounds() const
{
	return destRect;
}

SDL_Rect GameObject::GetHitbox() const
{
	// 30px inset horizontally, 20px top, 10px bottom — tight body box
	SDL_Rect hb;
	hb.x = destRect.x + 30;
	hb.y = destRect.y + 20;
	hb.w = destRect.w - 60;
	hb.h = destRect.h - 30;
	return hb;
}

void GameObject::Render()
{
	if (objTexture)
	{
		SDL_RendererFlip flip = facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
		SDL_RenderCopyEx(renderer, objTexture, &srcRect, &destRect, 0.0, NULL, flip);
	}
}