#ifndef button_h
#define button_h

#include <SDL2/SDL.h>
#include "atlas.h"
#include "game.h"

typedef struct {
	/* look */
	atlas_t slice_9, slice_9_pressed;
	SDL_Rect pos;

} button_t;

/*
 * Initializes a new button
 */
button_t button_new(SDL_Texture idle, SDL_Texture pressed, int x, int y, int w, int h) {
	button_t button = (button_t) {
		.slice_9 = idle,
		.slice_9_pressed = pressed,
		.pos = (SDL_Rect) {x, y, w, h}
	};
	//48x48 -> 16x16
	return button;
}

/*
 * Deletes the button
 */
void button_delete(button_t *b) {
	atlas_delete(&b->slice_9);
	atlas_delete(&b->slice_9_pressed);
}

void button_render(button_t *b) {
	SDL_
}

#endif
