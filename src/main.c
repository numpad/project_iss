#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "duktape.h"
#include "vector.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "weapon.h"

#define RGB(r,g,b) ( (r << 16) + (g << 8) + (b << 0) )

/* Game window and renderer */
SDL_Window *window;
SDL_Renderer *renderer;

int main(int argc, char *argv[]) {
	const int window_width  = 800,
	          window_height = 600;
	if (game_init("Project ISS", window_width, window_height, SDL_RENDERER_SOFTWARE) != 0) {
		return 1;
	}
	
	/* Init FPS Counter */
	fps_counter_t fps_counter = game_init_fps_counter();
	/* Mouse information*/
	vector_t mouse;
	Uint32 mousestate;
	/* Keyboard information */
	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

	/* Create level */
	map_t level = map_new(window_width, window_height);
	map_configure(&level, 0.245);
	for (int x = 0; x < level.width; ++x) {
		for (int y = level.height / 2 + sin(x / 100.0) * 20; y < level.height; ++y) {
			map_set_solid(&level, x, y, 0x7f5435, 1);
		}
	}
	
	/* Create player */
	int player_x = level.width / 2;
	int player_y = map_raycast(&level, vector_new(player_x, 0), vector_new(0, 1));
	player_t player = player_new(player_x, player_y, 16, 28);
	//player_configure(&player, 0.1, 0.8, 0.8, 0.7);
	player_loadconfig(&player, "settings/player.js");

	/* Enter main gameloop */
	int running = 1;
	while (running) {
		/**********\
		\* UPDATE */
		/* Handle events, update mouse information */
		game_handle_events(&running);
		game_get_mouse(&mousestate, &mouse);
			
		/* Move left/right */
		player_move(&player, &level,
			/* this calculation calculates -1 for key_left, 1 for key_right and 0 for either none or both keys down */
			(-keyboard[SDL_SCANCODE_LEFT] | keyboard[SDL_SCANCODE_RIGHT]) * (keyboard[SDL_SCANCODE_LEFT] ^ keyboard[SDL_SCANCODE_RIGHT])
			);
		
		if (keyboard[SDL_SCANCODE_UP]) {
			player_jump(&player, &level);
		}
		
		if (mousestate & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			map_set_circle(&level, mouse.x, mouse.y, 25, RGB(0, 0, 0), 0);
		}
		if (mousestate & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			map_set_circle(&level, mouse.x, mouse.y, 25, 0x7f5435, 1);
		}
		player_update(&player, &level);

		/**********\
		\* RENDER */
		/* Clear Renderer */
		SDL_SetRenderDrawColor(renderer, 120, 170, 220, 255);
		SDL_RenderClear(renderer);
		
		/* Draw pixels */
		map_update(&level);
		map_render(&level);
		player_render(&player);

		/* Display FPS & Render to screen */
		game_write_fps(&fps_counter, 10, 10);
		SDL_RenderPresent(renderer);
		SDL_Delay(5);
		
		game_calculate_fps(&fps_counter);
	}
	
	map_delete(&level);

	game_cleanup();
	return 0;
}
