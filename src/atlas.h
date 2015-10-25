#ifndef atlas_h
#define atlas_h

#include <SDL2/SDL.h>
#include <stdlib.h>

extern SDL_Renderer *renderer;

typedef struct {
	SDL_Texture *spritesheet;
	SDL_Rect *sprites_src;
	int allocated_sprites;
} atlas_t;

/*
 * Atlas holds information about spritesheet and its sprites
 */
atlas_t atlas_new(SDL_Texture *atlas, int sprites) {
	atlas_t new_atlas = (atlas_t) {
		.spritesheet = atlas,
		.sprites_src = malloc(sprites * sizeof(SDL_Rect)),
		.allocated_sprites = 0
	};
	
	SDL_SetTextureBlendMode(new_atlas.spritesheet, SDL_BLENDMODE_NONE);

	return new_atlas;
}

/*
 * Cleans up all the mess made by the atlas
 */
void atlas_delete(atlas_t *atl) {
	SDL_DestroyTexture(atl->spritesheet);
	free(atl->sprites_src);
}

/*
 * Add a new sprite to the atlas
 * returns `id` of sprite
 */
int atlas_add_sprite(atlas_t *atl, int x, int y, int w, int h) {
	atl->sprites_src[atl->allocated_sprites++] = (SDL_Rect) {x, y, w, h};
	return atl->allocated_sprites;
}

/*
 * Draws `sprite_id` of `atl` to the renderer
 */
void atlas_render(atlas_t *atl, int sprite_id, SDL_Rect *dst) {
	SDL_RenderCopy(renderer, atl->spritesheet, &atl->sprites_src[sprite_id], dst);
}

/*
 * Writes width and height of srcrect to `w`, `h`
 */
void atlas_getsize(atlas_t *atl, int sprite_id, int *w, int *h) {
	*w = atl->sprites_src[sprite_id].w;
	*h = atl->sprites_src[sprite_id].h;
}

/*
 * Draws `sprite_id` of `atl` to the renderer with rotation and flipping
 */
void atlas_render_ex(atlas_t *atl, int sprite_id, SDL_Rect *dst, const double angle, const SDL_RendererFlip flip) {
	SDL_RenderCopyEx(renderer, atl->spritesheet, &atl->sprites_src[sprite_id], dst, angle, NULL, flip);
}

#endif
