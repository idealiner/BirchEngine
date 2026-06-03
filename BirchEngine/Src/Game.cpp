#include "Game.h"
#include "TextureManager.h"
#include "GameObject.h"
#include <cstdio>
#include <cstring>
#include <cmath>

/*SDL_Texture* playerTex;
SDL_Rect srcR, destR;*/

GameObject* player;
GameObject* enemy;
int playerFlickerFrames = 0;
int enemyFlickerFrames = 0;
SDL_Rect prevPlayerRect = { 0, 0, 0, 0 };
int scoreCounter = 0;
Uint32 gameStartTicks = 0;
float cloudParallaxX = 0.0f;
float treeParallaxX = 0.0f;
SDL_Texture* butterflyTex = nullptr;
bool butterflyActive = true;
float butterflyX = 0.0f;
float butterflyY = 0.0f;
float butterflyPhase = 0.0f;
int butterflyFrame = 0;
int butterflyFrameTick = 0;
int butterflyRespawnFrames = 0;
int butterflyCapturedCount = 0;
int butterflyFxFrames = 0;
int butterflyFxX = 0;
int butterflyFxY = 0;

static const int SCREEN_WIDTH = 1024;
static const int SCREEN_HEIGHT = 768;
static const int GROUND_HEIGHT = 120;
static const int SPRITE_SIZE = 150;
static const int LEVEL_TIME_SECONDS = 300;
static const int BUTTERFLY_SIZE = 150;
static const int BUTTERFLY_FRAMES = 6;

static float TriangleWave(float t)
{
	float f = std::fmod(t, 1.0f);
	if (f < 0.0f)
	{
		f += 1.0f;
	}

	if (f < 0.5f)
	{
		return f * 4.0f - 1.0f;
	}

	return 3.0f - f * 4.0f;
}

static void DrawButterflyIcon(SDL_Renderer* renderer, int x, int y, int p)
{
	SDL_Rect r;
	SDL_SetRenderDrawColor(renderer, 255, 196, 64, 255);
	r = { x + (0 * p), y + (1 * p), 2 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (4 * p), y + (1 * p), 2 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (1 * p), y + (0 * p), 1 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (4 * p), y + (0 * p), 1 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, 250, 120, 64, 255);
	r = { x + (2 * p), y + (1 * p), 2 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (2 * p), y + (3 * p), 1 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
}

static void DrawButterflyFx(SDL_Renderer* renderer, int cx, int cy, int frame)
{
	const SDL_Color colors[4] = {
		{ 255, 252, 112, 255 },
		{ 255, 157, 60, 255 },
		{ 255, 255, 255, 255 },
		{ 255, 84, 84, 255 }
	};
	const SDL_Color c = colors[(frame / 3) % 4];
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	SDL_Rect r;
	r = { cx - 2, cy - 38, 4, 14 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 2, cy + 24, 4, 14 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 38, cy - 2, 14, 4 }; SDL_RenderFillRect(renderer, &r);
	r = { cx + 24, cy - 2, 14, 4 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 24, cy - 24, 10, 10 }; SDL_RenderFillRect(renderer, &r);
	r = { cx + 14, cy - 24, 10, 10 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 24, cy + 14, 10, 10 }; SDL_RenderFillRect(renderer, &r);
	r = { cx + 14, cy + 14, 10, 10 }; SDL_RenderFillRect(renderer, &r);
}

static void ResetButterflyFlight()
{
	butterflyActive = true;
	butterflyX = (float)(SCREEN_WIDTH + 120);
	butterflyPhase += 0.31f;
	butterflyY = 160.0f + TriangleWave(butterflyPhase) * 72.0f;
	butterflyFrame = 0;
	butterflyFrameTick = 0;
	butterflyRespawnFrames = 0;
}

static void DrawCloud(SDL_Renderer* renderer, int x, int y, int p)
{
	SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
	SDL_Rect r;
	r = { x + (1 * p), y + (0 * p), 6 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (0 * p), y + (1 * p), 8 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (1 * p), y + (3 * p), 6 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
}

static void DrawTree(SDL_Renderer* renderer, int x, int baseY, int p)
{
	SDL_Rect r;
	SDL_SetRenderDrawColor(renderer, 94, 62, 31, 255);
	r = { x + (4 * p), baseY - (4 * p), 2 * p, 4 * p }; SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, 42, 156, 71, 255);
	r = { x + (2 * p), baseY - (9 * p), 6 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (1 * p), baseY - (7 * p), 8 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (2 * p), baseY - (5 * p), 6 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
}

static void DrawGlyph(SDL_Renderer* renderer, int x, int y, int scale, char ch)
{
	static const char* G0[7] = { "11111", "10001", "10001", "10001", "10001", "10001", "11111" };
	static const char* G1[7] = { "00100", "01100", "00100", "00100", "00100", "00100", "01110" };
	static const char* G2[7] = { "11111", "00001", "00001", "11111", "10000", "10000", "11111" };
	static const char* G3[7] = { "11111", "00001", "00001", "01111", "00001", "00001", "11111" };
	static const char* G4[7] = { "10001", "10001", "10001", "11111", "00001", "00001", "00001" };
	static const char* G5[7] = { "11111", "10000", "10000", "11111", "00001", "00001", "11111" };
	static const char* G6[7] = { "11111", "10000", "10000", "11111", "10001", "10001", "11111" };
	static const char* G7[7] = { "11111", "00001", "00010", "00100", "01000", "01000", "01000" };
	static const char* G8[7] = { "11111", "10001", "10001", "11111", "10001", "10001", "11111" };
	static const char* G9[7] = { "11111", "10001", "10001", "11111", "00001", "00001", "11111" };
	static const char* GA[7] = { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
	static const char* GC[7] = { "01111", "10000", "10000", "10000", "10000", "10000", "01111" };
	static const char* GE[7] = { "11111", "10000", "10000", "11110", "10000", "10000", "11111" };
	static const char* GI[7] = { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
	static const char* GM[7] = { "10001", "11011", "10101", "10101", "10001", "10001", "10001" };
	static const char* GO[7] = { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
	static const char* GR[7] = { "11110", "10001", "10001", "11110", "10100", "10010", "10001" };
	static const char* GS[7] = { "01111", "10000", "10000", "01110", "00001", "00001", "11110" };
	static const char* GT[7] = { "11111", "00100", "00100", "00100", "00100", "00100", "00100" };
	static const char* GX[7] = { "10001", "10001", "01010", "00100", "01010", "10001", "10001" };
	static const char* GColon[7] = { "00000", "00100", "00100", "00000", "00100", "00100", "00000" };
	static const char* GSpace[7] = { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
	const char** g = GSpace;

	switch (ch)
	{
	case '0': g = G0; break;
	case '1': g = G1; break;
	case '2': g = G2; break;
	case '3': g = G3; break;
	case '4': g = G4; break;
	case '5': g = G5; break;
	case '6': g = G6; break;
	case '7': g = G7; break;
	case '8': g = G8; break;
	case '9': g = G9; break;
	case 'A': g = GA; break;
	case 'C': g = GC; break;
	case 'E': g = GE; break;
	case 'I': g = GI; break;
	case 'M': g = GM; break;
	case 'O': g = GO; break;
	case 'R': g = GR; break;
	case 'S': g = GS; break;
	case 'T': g = GT; break;
	case 'X': g = GX; break;
	case ':': g = GColon; break;
	default: break;
	}

	SDL_Rect px;
	for (int row = 0; row < 7; ++row)
	{
		for (int col = 0; col < 5; ++col)
		{
			if (g[row][col] == '1')
			{
				px = { x + col * scale, y + row * scale, scale, scale };
				SDL_RenderFillRect(renderer, &px);
			}
		}
	}
}

static void DrawText(SDL_Renderer* renderer, int x, int y, int scale, const char* text)
{
	if (!text)
	{
		return;
	}

	for (int i = 0; text[i] != '\0'; ++i)
	{
		DrawGlyph(renderer, x + i * (6 * scale), y, scale, text[i]);
	}
}

Game::Game()
{}

Game::~Game()
{}

void Game::init(const char* title, int width, int height, bool fullscreen)
{
	int flags = 0;
	window = nullptr;
	renderer = nullptr;
	player = nullptr;
	enemy = nullptr;
	
	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		IMG_Init(IMG_INIT_PNG);
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (renderer)
		{
			SDL_SetRenderDrawColor(renderer, 107, 185, 240, 255);
			isRunning = true;
		}
	}

	/*SDL_Surface* tmpSurface = IMG_Load("assets/megaman.png");
	playerTex = SDL_CreateTextureFromSurface(renderer, tmpSurface);
	SDL_FreeSurface(tmpSurface);

	playerTex = TextureManager::LoadTexture("assets/megaman.png", renderer);*/

	if (isRunning)
	{
		butterflyTex = TextureManager::LoadTexture("assets/coins.png", renderer);
		const int groundY = SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE;
		player = new GameObject("assets/cha11.png", renderer, 0, groundY, 6);
		player->SetGroundY(groundY);
		enemy = new GameObject("assets/arche.png", renderer, SCREEN_WIDTH + 120, groundY, 6);
		enemy->SetGroundY(groundY);
		scoreCounter = 0;
		gameStartTicks = SDL_GetTicks();
		cloudParallaxX = 0.0f;
		treeParallaxX = 0.0f;
		butterflyCapturedCount = 0;
		butterflyFxFrames = 0;
		ResetButterflyFlight();
	}
}

void Game::handleEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_SPACE && player)
			{
				player->Jump();
			}
			break;
		default:
			break;
		}
	}
}

void Game::update()
{
	/*cnt++;
	destR.h = 128;
	destR.w = 128;
	destR.x = cnt;

	std::cout << cnt << std::endl;*/
	int playerVelX = 0;
	int playerVelY = 0;
	const Uint8* keyState = SDL_GetKeyboardState(NULL);

	if (keyState[SDL_SCANCODE_LEFT])
	{
		playerVelX = -1;
	}
	else if (keyState[SDL_SCANCODE_RIGHT])
	{
		playerVelX = 1;
	}

	if (keyState[SDL_SCANCODE_UP])
	{
		playerVelY = -1;
	}
	else if (keyState[SDL_SCANCODE_DOWN])
	{
		playerVelY = 1;
	}

	// move distant layers opposite to player movement for a simple parallax effect
	cloudParallaxX -= playerVelX * 0.35f;
	treeParallaxX -= playerVelX * 0.75f;

	if (cloudParallaxX <= -SCREEN_WIDTH) cloudParallaxX += SCREEN_WIDTH;
	if (cloudParallaxX >= SCREEN_WIDTH) cloudParallaxX -= SCREEN_WIDTH;
	if (treeParallaxX <= -SCREEN_WIDTH) treeParallaxX += SCREEN_WIDTH;
	if (treeParallaxX >= SCREEN_WIDTH) treeParallaxX -= SCREEN_WIDTH;

	if (player)
	{
		prevPlayerRect = player->GetHitbox();
		player->SetVelocity(playerVelX, playerVelY);
		player->Update();
	}

	if (enemy)
	{
		enemy->SetVelocity(-2, 0);
		enemy->Update();

		SDL_Rect enemyRect = enemy->GetBounds();
		if (enemyRect.x + enemyRect.w < 0)
		{
			enemy->SetPosition(SCREEN_WIDTH + 120, SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE);
			enemyFlickerFrames = 0;
		}
	}

	if (player && enemy && enemyFlickerFrames == 0)
	{
		SDL_Rect playerRect = player->GetHitbox();
		SDL_Rect enemyRect  = enemy->GetHitbox();
		if (SDL_HasIntersection(&playerRect, &enemyRect))
		{
			const bool descending = playerRect.y > prevPlayerRect.y;
			const bool fromAbove = (prevPlayerRect.y + prevPlayerRect.h) <= (enemyRect.y + 8);

			if (descending && fromAbove)
			{
				enemyFlickerFrames = 40;
				scoreCounter += 100;
				player->StompBounce();
			}
			else if (playerFlickerFrames == 0)
			{
				playerFlickerFrames = 60;
			}
		}
	}

	if (playerFlickerFrames > 0)
	{
		playerFlickerFrames--;
	}

	if (enemyFlickerFrames > 0)
	{
		enemyFlickerFrames--;
	}

	if (butterflyActive)
	{
		butterflyX -= 3.0f;
		butterflyPhase += 0.028f;
		butterflyY = 160.0f + TriangleWave(butterflyPhase) * 72.0f;

		butterflyFrameTick++;
		if (butterflyFrameTick >= 5)
		{
			butterflyFrameTick = 0;
			butterflyFrame = (butterflyFrame + 1) % BUTTERFLY_FRAMES;
		}

		if (butterflyX < -BUTTERFLY_SIZE)
		{
			ResetButterflyFlight();
		}
	}
	else
	{
		if (butterflyRespawnFrames > 0)
		{
			butterflyRespawnFrames--;
		}
		else
		{
			ResetButterflyFlight();
		}
	}

	if (player && butterflyActive)
	{
		SDL_Rect playerRect = player->GetHitbox();
		SDL_Rect butterflyRect = { (int)butterflyX + 24, (int)butterflyY + 22, 102, 98 };
		const bool playerAirborne = playerRect.y < (SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE - 4);
		if (playerAirborne && SDL_HasIntersection(&playerRect, &butterflyRect))
		{
			butterflyActive = false;
			butterflyRespawnFrames = 150;
			butterflyFxFrames = 24;
			butterflyFxX = butterflyRect.x + butterflyRect.w / 2;
			butterflyFxY = butterflyRect.y + butterflyRect.h / 2;
			butterflyCapturedCount++;
			scoreCounter += 200;
		}
	}

	if (butterflyFxFrames > 0)
	{
		butterflyFxFrames--;
	}
}

void Game::render()
{
	// sky
	SDL_SetRenderDrawColor(renderer, 107, 185, 240, 255);
	SDL_RenderClear(renderer);

	int cx = (int)cloudParallaxX;
	DrawCloud(renderer, 90 + cx, 86, 6);
	DrawCloud(renderer, 350 + cx, 118, 5);
	DrawCloud(renderer, 690 + cx, 74, 6);
	DrawCloud(renderer, 90 + cx + SCREEN_WIDTH, 86, 6);
	DrawCloud(renderer, 350 + cx + SCREEN_WIDTH, 118, 5);
	DrawCloud(renderer, 690 + cx + SCREEN_WIDTH, 74, 6);
	DrawCloud(renderer, 90 + cx - SCREEN_WIDTH, 86, 6);
	DrawCloud(renderer, 350 + cx - SCREEN_WIDTH, 118, 5);
	DrawCloud(renderer, 690 + cx - SCREEN_WIDTH, 74, 6);

	const int groundTopY = SCREEN_HEIGHT - GROUND_HEIGHT;
	int tx = (int)treeParallaxX;
	DrawTree(renderer, 130 + tx, groundTopY, 8);
	DrawTree(renderer, 420 + tx, groundTopY, 7);
	DrawTree(renderer, 790 + tx, groundTopY, 9);
	DrawTree(renderer, 130 + tx + SCREEN_WIDTH, groundTopY, 8);
	DrawTree(renderer, 420 + tx + SCREEN_WIDTH, groundTopY, 7);
	DrawTree(renderer, 790 + tx + SCREEN_WIDTH, groundTopY, 9);
	DrawTree(renderer, 130 + tx - SCREEN_WIDTH, groundTopY, 8);
	DrawTree(renderer, 420 + tx - SCREEN_WIDTH, groundTopY, 7);
	DrawTree(renderer, 790 + tx - SCREEN_WIDTH, groundTopY, 9);

	if (butterflyActive && butterflyTex)
	{
		SDL_Rect src = { butterflyFrame * BUTTERFLY_SIZE, 0, BUTTERFLY_SIZE, BUTTERFLY_SIZE };
		SDL_Rect dst = { (int)butterflyX, (int)butterflyY, BUTTERFLY_SIZE, BUTTERFLY_SIZE };
		SDL_RenderCopy(renderer, butterflyTex, &src, &dst);
	}

	if (butterflyFxFrames > 0)
	{
		DrawButterflyFx(renderer, butterflyFxX, butterflyFxY, butterflyFxFrames);
	}

	// ground strip (bottom 120px)
	SDL_Rect ground = { 0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT };
	SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
	SDL_RenderFillRect(renderer, &ground);

	// darker dirt edge at top of ground
	SDL_Rect dirt = { 0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, 12 };
	SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255);
	SDL_RenderFillRect(renderer, &dirt);

	SDL_Rect hudBar = { 0, 0, SCREEN_WIDTH, 56 };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &hudBar);

	char scoreText[32];
	char timeText[32];
	char butterflyText[32];
	int elapsedSeconds = 0;
	int timeLeft = LEVEL_TIME_SECONDS;
	int timeMinutes = 0;
	int timeSeconds = 0;
	if (gameStartTicks > 0)
	{
		elapsedSeconds = (int)((SDL_GetTicks() - gameStartTicks) / 1000);
		timeLeft = LEVEL_TIME_SECONDS - elapsedSeconds;
		if (timeLeft < 0)
		{
			timeLeft = 0;
		}
	}
	timeMinutes = timeLeft / 60;
	timeSeconds = timeLeft % 60;

	std::snprintf(scoreText, sizeof(scoreText), "SCORE %06d", scoreCounter);
	std::snprintf(timeText, sizeof(timeText), "TIME %02d:%02d", timeMinutes, timeSeconds);
	std::snprintf(butterflyText, sizeof(butterflyText), "X %03d", butterflyCapturedCount);

	// NES-like text shadow pass
	SDL_SetRenderDrawColor(renderer, 58, 94, 161, 255);
	DrawText(renderer, 26, 18, 4, scoreText);
	DrawText(renderer, SCREEN_WIDTH - 24 - (int)std::strlen(timeText) * 24 + 2, 18, 4, timeText);

	// Main HUD text pass with low-time flashing on timer
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	DrawText(renderer, 24, 16, 4, scoreText);
	if (timeLeft <= 30 && ((SDL_GetTicks() / 200) % 2 == 0))
	{
		SDL_SetRenderDrawColor(renderer, 255, 84, 84, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	}
	DrawText(renderer, SCREEN_WIDTH - 24 - (int)std::strlen(timeText) * 24, 16, 4, timeText);

	SDL_SetRenderDrawColor(renderer, 58, 94, 161, 255);
	DrawButterflyIcon(renderer, SCREEN_WIDTH / 2 - 52 + 2, 18, 4);
	DrawText(renderer, SCREEN_WIDTH / 2 - 6 + 2, 16 + 2, 4, butterflyText);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	DrawButterflyIcon(renderer, SCREEN_WIDTH / 2 - 52, 18, 4);
	DrawText(renderer, SCREEN_WIDTH / 2 - 6, 16, 4, butterflyText);

	//SDL_RenderCopy(renderer, playerTex, NULL, &destR);
	if (player)
	{
		if (playerFlickerFrames == 0 || ((playerFlickerFrames / 4) % 2 == 0))
		{
			player->Render();
		}
	}

	if (enemy)
	{
		if (enemyFlickerFrames == 0 || ((enemyFlickerFrames / 4) % 2 == 0))
		{
			enemy->Render();
		}
	}

	SDL_RenderPresent(renderer);
}

void Game::clean()
{
	if (player)
	{
		delete player;
		player = nullptr;
	}

	if (enemy)
	{
		delete enemy;
		enemy = nullptr;
	}

	if (butterflyTex)
	{
		SDL_DestroyTexture(butterflyTex);
		butterflyTex = nullptr;
	}

	if (renderer)
	{
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}

	if (window)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	IMG_Quit();
	SDL_Quit();
}
