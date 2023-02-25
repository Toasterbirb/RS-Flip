#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "Birb2D.hpp"
#include "GuiVars.hpp"

using namespace Birb;

/* Function declarations */
static void start(Game& game);
static void input(Game& game);
static void update(Game& game);
static void render(Game& game);
static void post_render();
static void cleanup();

GuiVars* v;

int main(void)
{
	Game::WindowOpts window_options;
	window_options.title 				= "Flip";
	window_options.window_dimensions 	= { 1280, 720 };
	window_options.refresh_rate 		= 75;
	window_options.resizable 			= false;

	Game game_loop(window_options, start, input, update, render);

	/* Optional extra functions */
	game_loop.post_render = post_render;
	game_loop.cleanup = cleanup;

	/* Start the game loop */
	game_loop.Start();

	return 0;
}

/* start() is called before the game loop starts.
 * Useful for doing stuff that will only run once before
 * the game starts */
void start(Game& game)
{
	v = new GuiVars;
}

/* input() is called at the beginning of the frame
 * before update(). Behind the curtains it does input
 * polling etc. and then passes the SDL_Event into
 * this function */
void input(Game& game)
{
	if (game.window->isMouseDown())
	{
		if (game.window->CursorPosition().y < v->tabs[0]->button_dimensions.h && game.window->CursorPosition().x < v->tabs[0]->button_dimensions.w * v->tabs.size())
		{
			for (size_t i = 0; i < v->tabs.size(); ++i)
				v->tabs[i]->Deactivate();

			v->tabs[std::floor(game.window->CursorPosition().x / v->tabs[0]->button_dimensions.w)]->Activate();
		}
	}
}

/* update() is called after input has been handled and
 * before the frame gets rendered. Its useful for any game
 * logic that needs to be updated before rendering */
void update(Game& game)
{

}

/* render() is called after update() has been finished.
 * Before it gets called, the window will be cleared and
 * after the function has finished running, the rendered frame
 * will be presented */
void render(Game& game)
{
	v->game_scene.Render();
}

/* post_render() will be called after rendering has finished
 * and the timestep stalling has started. On non-windows systems
 * this function call will be done on a separate thread, so you
 * could use it to do some extra preparations for the next frame
 * while the main thread is sitting around doing nothing
 * and waiting to maintain the correct frame rate */
void post_render()
{

}

/* cleanup() gets called after the game loop has finished running
 * and the application is getting closed. This is useful for doing any
 * cleanup that is necessary, like freeing heap allocations etc. */
void cleanup()
{
	v->Cleanup();
	delete v;
}
