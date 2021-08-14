#pragma once

#include "Components.h"

class PositionComponents : public Component
{
private:
	int xpos;
	int ypos;

public:
	PositionComponents()
	{
		xpos = 0;
		ypos = 0;
	}
	PositionComponents(int x, int y)
	{
		xpos = x;
		ypos = y;
	}
	void update() override
	{
		xpos++;
		ypos++;
	}

	/*void setPos(int x, int y)
	{
		xpos = x;
		ypos = y;
	}*/

	int x() { return xpos; }
	void x(int x) { xpos = x; }
	int y() { return ypos; }
	void y(int y) { ypos = y; }
	void setPos(int x, int y) { xpos = x, ypos = y; }

};