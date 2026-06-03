#include "Game.h"
#include "TextureManager.h"
#include "GameObject.h"

/*SDL_Texture* playerTex;
SDL_Rect srcR, destR;*/

GameObject* player;
GameObject* enemy;
int playerFlickerFrames = 0;

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
		const int groundY = 768 - 120 - 150;
		player = new GameObject("assets/cha11.png", renderer, 0, groundY, 6);
		player->SetGroundY(groundY);
		enemy = new GameObject("assets/arche.png", renderer, 1024 + 120, groundY, 6);
		enemy->SetGroundY(groundY);
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

	if (player)
	{
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
			enemy->SetPosition(1024 + 120, 768 - 120 - 150);
		}
	}

	if (player && enemy)
	{
		SDL_Rect playerRect = player->GetBounds();
		SDL_Rect enemyRect  = enemy->GetBounds();
		if (SDL_HasIntersection(&playerRect, &enemyRect) && playerFlickerFrames == 0)
		{
			playerFlickerFrames = 60;
		}
	}

	if (playerFlickerFrames > 0)
	{
		playerFlickerFrames--;
	}
}

void Game::render()
{
	// sky
	SDL_SetRenderDrawColor(renderer, 107, 185, 240, 255);
	SDL_RenderClear(renderer);

	// ground strip (bottom 120px)
	SDL_Rect ground = { 0, 768 - 120, 1024, 120 };
	SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
	SDL_RenderFillRect(renderer, &ground);

	// darker dirt edge at top of ground
	SDL_Rect dirt = { 0, 768 - 120, 1024, 12 };
	SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255);
	SDL_RenderFillRect(renderer, &dirt);

	//SDL_RenderCopy(renderer, playerTex, NULL, &destR);
	if (player)
	{
		if (playerFlickerFrames == 0 || ((playerFlickerFrames / 4) % 2 == 0))
		{
			player->Render();
		}
	}
	if (enemy) enemy->Render();
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