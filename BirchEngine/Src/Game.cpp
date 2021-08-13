#include "Game.h"
#include "TextureManager.h"
#include "GameObject.h"

/*SDL_Texture* playerTex;
SDL_Rect srcR, destR;*/

GameObject* player;
GameObject* enemy;

Game::Game()
{}

Game::~Game()
{}

void Game::init(const char* title, int width, int height, bool fullscreen)
{
	int flags = 0;
	
	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer)
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		}

		isRunning = true;
	}

	/*SDL_Surface* tmpSurface = IMG_Load("assets/megaman.png");
	playerTex = SDL_CreateTextureFromSurface(renderer, tmpSurface);
	SDL_FreeSurface(tmpSurface);

	playerTex = TextureManager::LoadTexture("assets/megaman.png", renderer);*/

	player = new GameObject("assets/megaman.png", renderer, 0,0);
	enemy = new GameObject("assets/enemy.png", renderer, 150, 150);
}

void Game::handleEvents()
{
	SDL_Event event;

	SDL_PollEvent(&event);

	switch (event.type)
	{
	case SDL_QUIT :
		isRunning = false;
		break;
	default:
		break;
	}
}

void Game::update()
{
	/*cnt++;
	destR.h = 128;
	destR.w = 128;
	destR.x = cnt;

	std::cout << cnt << std::endl;*/

	player->Update();
	enemy->Update();
}

void Game::render()
{
	SDL_RenderClear(renderer);
	//SDL_RenderCopy(renderer, playerTex, NULL, &destR);
	player->Render();
	enemy->Render();
	SDL_RenderPresent(renderer);
}

void Game::clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}