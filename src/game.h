#ifndef game_h
#define game_h

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "lodepng.h"
#include "vector.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;

/*
 * Initialize SDL, subsystems and window, renderer
 * Returns 0 on success, 1 on failure (prints error code)
 */
int game_init(const char *title, const int width, const int height, Uint32 rendererflags) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL failed to initialize: %s\n", SDL_GetError());
		return 1;
	}
	if (IMG_Init(IMG_INIT_PNG) < 0) {
		printf("SDL_image failed to initialize: %s\n", IMG_GetError());
		return 1;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer failed to initilaize: %s\n", Mix_GetError());
		return 1;
	}

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, rendererflags);
	
	if (!window) {
		puts("Failed to create window!");
		return 1;
	}
	if (!renderer) {
		puts("Failed to create renderer!");
		return 1;
	}
	
	return 0;
}

/*
 * Clean up all the mess SDL made
 */
int game_cleanup() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	
	return 0;
}

/*
 * Loads an image in RGB format
 */
Uint32 *game_load_pixels(const char *fn, unsigned int *w, unsigned int *h) {
	unsigned char *img_rgba;
	unsigned error = lodepng_decode32_file(&img_rgba, w, h, fn);
	if (error) {
		printf("Error %u loading image '%s': %s\n", error, fn, lodepng_error_text(error));
		return NULL;
	}

	Uint32 *pixels;
	pixels = malloc(*w * *h * sizeof(Uint32));
	for (int i = 0; i < *w * *h * 4; i += 4) {
		pixels[i / 4] = RGB(img_rgba[i], img_rgba[i + 1], img_rgba[i + 2]);
	}

	free(img_rgba);
	return pixels;
}

/*
 * Loads an image in RGB format and it's solidity-mask
 */
Uint32 *game_load_solid_pixels(const char *fn, unsigned int *w, unsigned int *h, char *solid_tiles) {
	unsigned char *img_rgba;
	unsigned error = lodepng_decode32_file(&img_rgba, w, h, fn);
	if (error) {
		printf("Error %u loading image '%s': %s\n", error, fn, lodepng_error_text(error));
		return NULL;
	}

	Uint32 *pixels = malloc(*w * *h * sizeof(Uint32));
	for (int i = 0; i < *w * *h * 4; i += 4) {
		solid_tiles[i / 4] = img_rgba[i + 3];
		/* if tile is transparent load it as black */
		if (solid_tiles[i / 4]) {
			pixels[i / 4] = RGB(img_rgba[i], img_rgba[i + 1], img_rgba[i + 2]);
		} else {
			pixels[i / 4] = RGB(0, 0, 0);
		}
		
	}

	free(img_rgba);
	return pixels;
}

/*
 * Load texture from `fn`
 */
SDL_Texture *game_load_texture(const char *fn) {
	SDL_Surface *surface = IMG_Load(fn);
	/* Check if image loading worked */
	if (surface == NULL) {
		printf("Unable to load image '%s': %s\n", fn, IMG_GetError());
		return NULL;
	}
	
	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
	/* check if surface-->texture worked */
	if (tex == NULL) {
		printf("Cannot create texture from surface ('%s'): %s\n", fn, SDL_GetError());
		return NULL;
	}
	
	/* clean up surface, we already have a texture */
	SDL_FreeSurface(surface);

	return tex;
}

/*
 * Handles events for SDL
 */
void game_handle_events(int *running) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT:
			*running = 0;
			break;
		}
	}
}

/*
 * Get mouse pos & state
 */
void game_get_mouse(Uint32 *mousestate, vector_t *mousepos) {
	int mx, my;
	*mousestate = SDL_GetMouseState(&mx, &my);
	mousepos->x = mx;
	mousepos->y = my;
}


/*
 * Hold information to count fps
 */
typedef struct {
	float interval;
	Uint32 lasttime, current, frames;
	char *fps_string;
} fps_counter_t;

/*
 * Initialize the fps counter
 */
fps_counter_t game_init_fps_counter() {
	fps_counter_t counter = (fps_counter_t) {
		.interval = 1.0,
		.lasttime = SDL_GetTicks(),
		.current = 0,
		.frames = 0,
		.fps_string = malloc(11)
	};
	
	strcpy(counter.fps_string, "FPS: __");

	return counter;
}
/*
 * Free memory taken by `counter`
 */
void game_delete_fps_counter(fps_counter_t *counter) {
	free(counter->fps_string);
}

/*
 * Calculate FPS in specified interval
 */
void game_calculate_fps(fps_counter_t *counter) {	
	counter->frames++;
	if (counter->lasttime < SDL_GetTicks() - counter->interval * 1000) {
		counter->lasttime = SDL_GetTicks();
		counter->current = counter->frames;
		counter->frames = 0;

		sprintf(counter->fps_string, "FPS: %d", counter->current);
	}
}

void game_write_fps(fps_counter_t *counter, const int x, const int y) {
	stringRGBA(renderer, x, y, counter->fps_string, 255, 155, 130, 255);
}

#endif
