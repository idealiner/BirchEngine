#include "Game.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

Game *game = nullptr;

static void RunFrame()
{
	if (!game || !game->running())
	{
		return;
	}

	game->handleEvents();
	game->update();
	game->render();
}

#ifdef __EMSCRIPTEN__
static void MainLoopTick()
{
	if (!game || !game->running())
	{
		if (game)
		{
			game->clean();
			delete game;
			game = nullptr;
		}
		emscripten_cancel_main_loop();
		return;
	}

	RunFrame();
}
#endif

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	game = new Game();
	game->init("GameWindow", 1024, 768, false);

#ifdef __EMSCRIPTEN__
	if (!game->running())
	{
		game->clean();
		delete game;
		game = nullptr;
		return 1;
	}

	emscripten_set_main_loop(MainLoopTick, 0, 1);
	return 0;
#else
	while (game->running())
	{
		RunFrame();
	}

	game->clean();
	delete game;
	game = nullptr;
	return 0;
#endif
}