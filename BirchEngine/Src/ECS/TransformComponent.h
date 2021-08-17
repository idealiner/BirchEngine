#pragma once
#include "Components.h"
#include "../Vector2D.h"

class TransformComponents : public Component
{

public:

	Vector2D position;
	Vector2D velocity;

	int height = 150;
	int width = 150;
	int scale = 1;

	int speed = 3;

	TransformComponents()
	{
		position.x = 0.0f;
		position.y = 0.0f;
	}

	TransformComponents(int sc)
	{
		position.x = 0.0f;
		position.y = 0.0f;
		scale = sc;
	}

	TransformComponents(float x, float y)
	{
		position.x = x;
		position.y = y;
	}

	TransformComponents(float x, float y, int h, int w, int sc)
	{
		position.x = x;
		position.y = y;
		height = h;
		width = w;
		scale = sc;
	}

	void init() override
	{
		velocity.x = 0;
		velocity.y = 0;
	}

	void update() override
	{
		position.x += velocity.x * speed;
		position.y += velocity.y * speed;
	}

};