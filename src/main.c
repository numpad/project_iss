#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "duktape.h"
#include "lodepng.h"
#include "vector.h"

#define RGB(r,g,b) ( (r << 16) + (g << 8) + (b << 0) )

#include "sound.h"
#include "music.h"
#include "atlas.h"
#include "game.h"
#include "map.h"
#include "player.h"

/* Game window and renderer */
SDL_Window *window;
SDL_Renderer *renderer;
int window_width  = 800,
    window_height = 600;
vector_t mouse;
map_t level;

int main(int argc, char *argv[]) {
	if (game_init("Project ISS", window_width, window_height, SDL_RENDERER_SOFTWARE) != 0) {
		return 1;
	}
	
	/* Init FPS Counter */
	fps_counter_t fps_counter = game_init_fps_counter();
	/* Mouse information*/
	Uint32 mousestate;
	/* Keyboard information */
	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

	/* Create level */
	level = map_loadnew("maps/tiled/");
	map_configure(&level, 0.245);

	/* Create player */
	player_t player = player_new(200, 200, 1, 1);
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
			(-keyboard[SDL_SCANCODE_A] | keyboard[SDL_SCANCODE_D]) * (keyboard[SDL_SCANCODE_A] ^ keyboard[SDL_SCANCODE_D])
			);
		
		if (keyboard[SDL_SCANCODE_W]) {
			player_jump(&player, &level);
		}
		
		// DEBUG: Reload settings on the fly
		if (keyboard[SDL_SCANCODE_R]) {
			player_loadconfig(&player, "settings/player.js");
		}

		if (mousestate & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			//map_explode(&level, mouse.x + level.scroll.x, mouse.y + level.scroll.y, 25, 2, 0x313574);
			player_grenade_new(&player);
		}
		if (mousestate & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			map_set_rect(&level, mouse.x + level.scroll.x, mouse.y + level.scroll.y, 30, 14, 0x313574, 1);
		}
		
		if (keyboard[SDL_SCANCODE_X]) {
			//player_grenade_new(&player);
		}

		if (player.bullets[0].rad > 1)
			player.bullets[0].behave(&player.bullets[0], &level);
		
		player_update(&player, &level);
		map_setscroll(&level, vector_sub(player.pos, vector_sdiv(vector_new(window_width, window_height), 2)));


		/**********\
		\* RENDER */
		/* Clear Renderer */
		SDL_SetRenderDrawColor(renderer, 120, 170, 220, 255);
		SDL_RenderClear(renderer);
		
		/* Draw pixels */
		map_update(&level);
		map_render(&level);
		player_render(&player, &level);

		/* Display FPS & Render to screen */
		game_write_fps(&fps_counter, 10, 10);
		SDL_RenderPresent(renderer);
		SDL_Delay(5);
		
		game_calculate_fps(&fps_counter);
	}
	map_delete(&level);
	
	player_delete(&player);
	
	game_cleanup();
	return 0;
}
