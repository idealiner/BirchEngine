#include "Game.h"
#include "TextureManager.h"
#include "GameObject.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

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

int playerInvulnFrames = 0;
int comboChain = 0;
int freezeFrames = 0;
int shakeFrames = 0;
int floatingScoreFrames = 0;
int floatingScoreX = 0;
int floatingScoreY = 0;
int floatingScoreValue = 0;
bool isPaused = false;
bool nearMissAwarded = false;
int prevEnemyX = 0;
float butterflySpeed = 3.0f;
int enemySpeed = 2;
Uint32 lastUpdateTicks = 0;
float gameClockSeconds = 0.0f;
int elapsedSeconds = 0;
int currentTimeLeft = 0;

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

static int CalcButterflyRespawnFrames(int elapsed)
{
	int stage30 = elapsed / 30;
	return std::max(60, 150 - stage30 * 8);
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
		playerInvulnFrames = 0;
		comboChain = 0;
		freezeFrames = 0;
		shakeFrames = 0;
		floatingScoreFrames = 0;
		isPaused = false;
		nearMissAwarded = false;
		lastUpdateTicks = SDL_GetTicks();
		gameClockSeconds = 0.0f;
		elapsedSeconds = 0;
		currentTimeLeft = LEVEL_TIME_SECONDS;
		ResetButterflyFlight();
		prevEnemyX = SCREEN_WIDTH + 120;
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
			if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_RETURN)
			{
				isPaused = !isPaused;
			}
			if (!isPaused && event.key.keysym.sym == SDLK_SPACE && player)
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
	int playerVelX = 0;
	int playerVelY = 0;
	Uint32 nowTicks = SDL_GetTicks();
	if (lastUpdateTicks == 0)
	{
		lastUpdateTicks = nowTicks;
	}

	float dt = (float)(nowTicks - lastUpdateTicks) / 1000.0f;
	if (dt > 0.05f)
	{
		dt = 0.05f;
	}
	lastUpdateTicks = nowTicks;

	if (isPaused)
	{
		return;
	}

	if (freezeFrames > 0)
	{
		freezeFrames--;
		return;
	}

	gameClockSeconds += dt;
	elapsedSeconds = (int)gameClockSeconds;
	currentTimeLeft = LEVEL_TIME_SECONDS - elapsedSeconds;
	if (currentTimeLeft < 0)
	{
		currentTimeLeft = 0;
	}

	float progress = 1.0f - ((float)currentTimeLeft / (float)LEVEL_TIME_SECONDS);
	enemySpeed = 2 + (int)std::floor(progress * 2.5f);
	if (enemySpeed > 5)
	{
		enemySpeed = 5;
	}
	butterflySpeed = 3.0f + 3.0f * progress;

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
		prevEnemyX = enemy->GetBounds().x;
		enemy->SetVelocity(-enemySpeed, 0);
		enemy->Update();
		SDL_Rect enemyRect = enemy->GetBounds();
		if (enemyRect.x + enemyRect.w < 0)
		{
			enemy->SetPosition(SCREEN_WIDTH + 120, SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE);
			enemyFlickerFrames = 0;
			nearMissAwarded = false;
		}
	}

	if (player && enemy && enemyFlickerFrames == 0)
	{
		SDL_Rect playerRect = player->GetHitbox();
		SDL_Rect enemyRect = enemy->GetHitbox();
		if (SDL_HasIntersection(&playerRect, &enemyRect))
		{
			const bool descending = playerRect.y > prevPlayerRect.y;
			const bool fromAbove = (prevPlayerRect.y + prevPlayerRect.h) <= (enemyRect.y + 8);
			if (descending && fromAbove)
			{
				enemyFlickerFrames = 40;
				int stompBonus = (currentTimeLeft <= 30) ? 200 : 100;
				scoreCounter += stompBonus;
				player->StompBounce();
				freezeFrames = 3;
				floatingScoreValue = stompBonus;
				floatingScoreFrames = 40;
				floatingScoreX = enemyRect.x + enemyRect.w / 2;
				floatingScoreY = enemyRect.y - 20;
			}
			else if (playerFlickerFrames == 0 && playerInvulnFrames == 0)
			{
				playerFlickerFrames = 60;
				playerInvulnFrames = 60;
				scoreCounter = std::max(0, scoreCounter - 100);
				butterflyCapturedCount = std::max(0, butterflyCapturedCount - 1);
				comboChain = 0;
				shakeFrames = 14;
			}
		}
	}

	if (player && enemy && !nearMissAwarded)
	{
		SDL_Rect playerRect = player->GetHitbox();
		SDL_Rect enemyRect = enemy->GetHitbox();
		bool crossed = (prevEnemyX >= playerRect.x + playerRect.w) && (enemyRect.x + enemyRect.w < playerRect.x);
		bool playerAirborne = playerRect.y < (SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE - 4);
		int verticalGap = std::abs((playerRect.y + playerRect.h) - enemyRect.y);
		if (crossed && playerAirborne && verticalGap < 80)
		{
			scoreCounter += 50;
			nearMissAwarded = true;
			floatingScoreValue = 50;
			floatingScoreFrames = 30;
			floatingScoreX = playerRect.x + playerRect.w / 2;
			floatingScoreY = playerRect.y - 14;
		}
	}

	if (playerFlickerFrames > 0) playerFlickerFrames--;
	if (enemyFlickerFrames > 0) enemyFlickerFrames--;
	if (playerInvulnFrames > 0) playerInvulnFrames--;

	if (butterflyActive)
	{
		int minuteStage = elapsedSeconds / 60;
		float centerY = (minuteStage % 2 == 0) ? 160.0f : 210.0f;
		float amplitudeY = (minuteStage % 2 == 0) ? 72.0f : 92.0f;
		float phaseSpeed = (minuteStage % 2 == 0) ? 0.028f : 0.038f;
		butterflyX -= butterflySpeed;
		butterflyPhase += phaseSpeed;
		butterflyY = centerY + TriangleWave(butterflyPhase) * amplitudeY;
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
		bool playerAirborne = playerRect.y < (SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE - 4);
		if (playerAirborne && SDL_HasIntersection(&playerRect, &butterflyRect))
		{
			butterflyActive = false;
			butterflyRespawnFrames = CalcButterflyRespawnFrames(elapsedSeconds);
			butterflyFxFrames = 24;
			butterflyFxX = butterflyRect.x + butterflyRect.w / 2;
			butterflyFxY = butterflyRect.y + butterflyRect.h / 2;
			butterflyCapturedCount++;
			comboChain++;
			if (comboChain > 5) comboChain = 5;
			int capturePoints = 200 * comboChain;
			scoreCounter += capturePoints;
			floatingScoreValue = capturePoints;
			floatingScoreFrames = 40;
			floatingScoreX = butterflyRect.x + butterflyRect.w / 2;
			floatingScoreY = butterflyRect.y - 12;
			freezeFrames = 2;
		}
	}

	if (butterflyFxFrames > 0) butterflyFxFrames--;
	if (floatingScoreFrames > 0)
	{
		floatingScoreFrames--;
		if ((floatingScoreFrames % 3) == 0) floatingScoreY--;
	}
}

void Game::render()
{
	int shakeX = 0;
	int shakeY = 0;
	if (shakeFrames > 0)
	{
		shakeFrames--;
		shakeX = ((SDL_GetTicks() / 17) % 3) - 1;
		shakeY = ((SDL_GetTicks() / 23) % 3) - 1;
	}

	if (currentTimeLeft <= 30)
	{
		Uint32 pulse = (SDL_GetTicks() / 120) % 3;
		if (pulse == 0) SDL_SetRenderDrawColor(renderer, 120, 12, 12, 255);
		else if (pulse == 1) SDL_SetRenderDrawColor(renderer, 80, 8, 8, 255);
		else SDL_SetRenderDrawColor(renderer, 18, 6, 6, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 107, 185, 240, 255);
	}
	SDL_RenderClear(renderer);

	int cx = (int)cloudParallaxX;
	DrawCloud(renderer, 90 + cx + shakeX, 86 + shakeY, 6);
	DrawCloud(renderer, 350 + cx + shakeX, 118 + shakeY, 5);
	DrawCloud(renderer, 690 + cx + shakeX, 74 + shakeY, 6);
	DrawCloud(renderer, 90 + cx + SCREEN_WIDTH + shakeX, 86 + shakeY, 6);
	DrawCloud(renderer, 350 + cx + SCREEN_WIDTH + shakeX, 118 + shakeY, 5);
	DrawCloud(renderer, 690 + cx + SCREEN_WIDTH + shakeX, 74 + shakeY, 6);
	DrawCloud(renderer, 90 + cx - SCREEN_WIDTH + shakeX, 86 + shakeY, 6);
	DrawCloud(renderer, 350 + cx - SCREEN_WIDTH + shakeX, 118 + shakeY, 5);
	DrawCloud(renderer, 690 + cx - SCREEN_WIDTH + shakeX, 74 + shakeY, 6);

	const int groundTopY = SCREEN_HEIGHT - GROUND_HEIGHT;
	int tx = (int)treeParallaxX;
	DrawTree(renderer, 130 + tx + shakeX, groundTopY + shakeY, 8);
	DrawTree(renderer, 420 + tx + shakeX, groundTopY + shakeY, 7);
	DrawTree(renderer, 790 + tx + shakeX, groundTopY + shakeY, 9);
	DrawTree(renderer, 130 + tx + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 8);
	DrawTree(renderer, 420 + tx + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 7);
	DrawTree(renderer, 790 + tx + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 9);
	DrawTree(renderer, 130 + tx - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 8);
	DrawTree(renderer, 420 + tx - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 7);
	DrawTree(renderer, 790 + tx - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 9);

	if (butterflyActive && butterflyTex)
	{
		SDL_Rect src = { butterflyFrame * BUTTERFLY_SIZE, 0, BUTTERFLY_SIZE, BUTTERFLY_SIZE };
		SDL_Rect dst = { (int)butterflyX + shakeX, (int)butterflyY + shakeY, BUTTERFLY_SIZE, BUTTERFLY_SIZE };
		SDL_RenderCopy(renderer, butterflyTex, &src, &dst);
	}

	if (butterflyFxFrames > 0)
	{
		DrawButterflyFx(renderer, butterflyFxX + shakeX, butterflyFxY + shakeY, butterflyFxFrames);
	}

	SDL_Rect ground = { 0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT };
	SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
	SDL_RenderFillRect(renderer, &ground);
	SDL_Rect dirt = { 0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, 12 };
	SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255);
	SDL_RenderFillRect(renderer, &dirt);

	SDL_Rect hudBar = { 0, 0, SCREEN_WIDTH, 56 };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &hudBar);

	char scoreText[32];
	char timeText[32];
	char butterflyText[32];
	int timeLeft = currentTimeLeft;
	int timeMinutes = timeLeft / 60;
	int timeSeconds = timeLeft % 60;
	std::snprintf(scoreText, sizeof(scoreText), "SCORE %06d", scoreCounter);
	std::snprintf(timeText, sizeof(timeText), "TIME %02d:%02d", timeMinutes, timeSeconds);
	std::snprintf(butterflyText, sizeof(butterflyText), "X %03d", butterflyCapturedCount);

	SDL_SetRenderDrawColor(renderer, 58, 94, 161, 255);
	DrawText(renderer, 26, 18, 4, scoreText);
	DrawText(renderer, SCREEN_WIDTH - 24 - (int)std::strlen(timeText) * 24 + 2, 18, 4, timeText);

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
	DrawText(renderer, SCREEN_WIDTH / 2 - 6 + 2, 18, 4, butterflyText);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	DrawButterflyIcon(renderer, SCREEN_WIDTH / 2 - 52, 18, 4);
	DrawText(renderer, SCREEN_WIDTH / 2 - 6, 16, 4, butterflyText);

	if (floatingScoreFrames > 0)
	{
		char floatText[24];
		std::snprintf(floatText, sizeof(floatText), "%d", floatingScoreValue);
		SDL_SetRenderDrawColor(renderer, 58, 94, 161, 255);
		DrawText(renderer, floatingScoreX + 1, floatingScoreY + 1, 3, floatText);
		SDL_SetRenderDrawColor(renderer, 255, 252, 112, 255);
		DrawText(renderer, floatingScoreX, floatingScoreY, 3, floatText);
	}

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

	if (isPaused)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
		SDL_Rect pauseShade = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderFillRect(renderer, &pauseShade);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_Rect bar1 = { SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 34, 14, 68 };
		SDL_Rect bar2 = { SCREEN_WIDTH / 2 + 6, SCREEN_HEIGHT / 2 - 34, 14, 68 };
		SDL_RenderFillRect(renderer, &bar1);
		SDL_RenderFillRect(renderer, &bar2);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
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
