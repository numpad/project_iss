#ifndef map_h
#define map_h

#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern int window_width, window_height;

typedef struct {
	/* full dimensions */
	int width, height;
	vector_t scroll;
	SDL_Rect display_rect;
	
	/* Used to draw map to screen */
	SDL_Texture *texture;
	/* Color tiles */
	Uint32 *rgb_tiles;
	/* Solid tiles */
	char *solid_tiles;
	/* Background tiles */
	Uint32 *back_rgb_tiles;

	/* map properties */
	float gravity;
} map_t;

void map_update(map_t *);

/*
 * Creates a new map with `width` x `height` dimensions
 */
map_t map_new(const int width, const int height) {
	map_t map = {
		.width = width,
		.height = height,
		.scroll = vector_new(0, 0),
		.display_rect = (SDL_Rect) {0, 0, window_width, window_height},
		.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height),
		.rgb_tiles = malloc(width * height * sizeof(Uint32)),
		.solid_tiles = calloc(width * height, sizeof(char)),
		.back_rgb_tiles = calloc(width * height, sizeof(Uint32)),
		.gravity = 0.275
	};
	
	/* Writes rgb_tiles to texture */
	map_update(&map);
	return map;
}

/*
 * Frees all memory and destroys the texture used by map
 */
void map_delete(map_t *map) {
	SDL_DestroyTexture(map->texture);
	free(map->rgb_tiles);
	free(map->solid_tiles);
	free(map->back_rgb_tiles);
}

/*
 * Changes configuration
 */
void map_configure(map_t *map, float gravity) {
	map->gravity = gravity;
}

/*
 * Scroll `map` relative
 */
void map_scroll(map_t *map, vector_t dir) {
	map->scroll = vector_add(map->scroll, dir);
	
	/* limit so we wont render nonsense */
	if (map->scroll.x < 0)
		map->scroll.x = 0;
	if (map->scroll.x + map->display_rect.w >= map->width)
		map->scroll.x = map->width - map->display_rect.w;
	if (map->scroll.y < 0)
		map->scroll.y = 0;
	if (map->scroll.y + map->display_rect.h >= map->height)
		map->scroll.y = map->height - map->display_rect.h;
}

/*
 * Set `map` scroll absolute 
 */
void map_setscroll(map_t *map, vector_t pos) {
	map->scroll = pos;
	
	/* limit so we wont render nonsense */
	if (map->scroll.x < 0)
		map->scroll.x = 0;
	if (map->scroll.x + map->display_rect.w >= map->width)
		map->scroll.x = map->width - map->display_rect.w;
	if (map->scroll.y < 0)
		map->scroll.y = 0;
	if (map->scroll.y + map->display_rect.h >= map->height)
		map->scroll.y = map->height - map->display_rect.h;
}

/*
 * Sets tile at `x`,`y` to `c` without changing `solid` flag
 */
void map_set(map_t *map, const int x, const int y, Uint32 c) {
	map->rgb_tiles[x + y * map->width] = c;
}

/*
 * Sets tile at `x`,`y` to `c` and sets `solid`.
 */
void map_set_solid(map_t *map, const int x, const int y, Uint32 c, const char solid) {
	if (!solid)
		map->rgb_tiles[x + y * map->width] = map->back_rgb_tiles[x + y * map->width];
	else
		map->rgb_tiles[x + y * map->width] = c;
	map->solid_tiles[x + y * map->width] = solid;
}

/*
 * Returns color at `x`,`y`
 */
Uint32 map_get(map_t *map, const int x, const int y) {
	return map->rgb_tiles[x + y * map->width];
}

/*
 * Returns flags at `x`,`y`
 */
char map_get_solid(map_t *map, const int x, const int y) {
	return map->solid_tiles[x + y * map->width];
}

/*
 * Updates the texture by writing map::rgb_tiles to it
 */
void map_update(map_t *map) {
	SDL_UpdateTexture(map->texture, NULL, map->rgb_tiles, map->width * sizeof(Uint32));
}

/*
 * Renders `map` to the screen
 */
void map_render(map_t *map) {
	map->display_rect.x = map->scroll.x;
	map->display_rect.y = map->scroll.y;

	SDL_RenderCopy(renderer, map->texture, &map->display_rect, NULL);
}

/*
 * Checks if `vec` is colliding with a solid tile
 */
int map_collides_vector(map_t *map, const vector_t vec) {
	return (int)map_get_solid(map, (int)vec.x, (int)vec.y);
}
/*
 * Checks if `vec` + vector(`dx`,`dy`) is colliding with a solid tile
 */
int map_collides_vector_rel(map_t *map, const vector_t vec, const int dx, const int dy) {
	return (int)map_get_solid(map, (int)vec.x + dx, (int)vec.y + dy);
}

/*
 * Checks if `x`,`y` if on the map
 * x=width, y=0 leads to index *width* which would technically be an available tile (assumed height > 1),
 * this however would still return false because coordinates (width|0) are not seen as *on map*
 */
int map_in_bounds(map_t *map, const int x, const int y) {
	return !(x < 0 || x >= map->width || y < 0 || y >= map->height);
}

/*
 * Sets all tiles from `xp`,`yp` with a distance of `r` or lower to it to color `c` and sets its solid state to `solid`
 */
void map_set_circle(map_t *map, const int xp, const int yp, const int r, Uint32 c, int solid) {
	for (int x = -r; x <= r; ++x) {
		for (int y = -r; y <= r; ++y) {
			if (x*x + y*y <= r*r && map_in_bounds(map, xp + x, yp + y)) {
				map_set_solid(map, xp + x, yp + y, c, solid);
			}
		}
	}
	
}

/*
 * Sets all tiles from `xp - w/2`,`yp - h/2` in a dimension of `w`,`h` to it to color `c` and sets its solid state to `solid`
 */
void map_set_rect(map_t *map, const int xp, const int yp, const int w, const int h, Uint32 c, int solid) {
	for (int x = -w/2; x < w/2; ++x) {
		for (int y = -h/2; y < h/2; ++y) {
			map_set_solid(map, xp + x, yp + y, c, solid);
		}
	}
	
}

/*
 * Sets all tiles from `xp`,`yp` with a distance of `r` or lower to it to color `c` and sets its solid state to `solid`
 */
void map_explode(map_t *map, const int xp, const int yp, const int r, const int wd, Uint32 c) {
	for (int x = -r; x <= r; ++x) {
		for (int y = -r; y <= r; ++y) {
			if (x*x + y*y <= r*r && map_in_bounds(map, xp + x, yp + y)) {
				if (map_get_solid(map, xp + x, yp + y))
					map_set_solid(map, xp + x, yp + y, c, 1);
			}
		}
	}
	
	map_set_circle(map, xp, yp, r - wd, 0x0, 0);
}

/*
 * Returns the distance from the ray `start` moving in direction `dir` to the next solid tile.
 * out of bounds counts as collision --> returns dist to previous tile
 */
float map_raycast(map_t *map, vector_t start, vector_t dir) {
	float dist = -1;
	dir = vector_normalize(dir);
	vector_t pos = start;
	if (map_collides_vector(map, pos)) {
		dist = vector_len(vector_sub(pos, start));
		return dist;
	}

	while (1) {
		//dist += vector_len(vector_sub(pos, vector_add(pos, dir)));

		/* move to next tile and check if in bounds */
		pos = vector_add(pos, dir);
		if (!map_in_bounds(map, pos.x, pos.y)) {
			dist = vector_len(vector_sub(vector_sub(pos, dir), start));
			break;
		}
		
		if (map_collides_vector(map, pos)) {
			dist = vector_len(vector_sub(pos, start));
			break;
		}
	}

	return dist;
}

/*
 * Load map from RGBA image
 * Transparency -> not solid
 */
void map_load_rgba(map_t *map, const char *fn) {
	unsigned int w, h;
	char *solid_pixels = malloc(map->width * map->height);
	Uint32 *pixels = game_load_solid_pixels(fn, &w, &h, solid_pixels);
	for (int x = 0; x < map->width; ++x) {
		for (int y = 0; y < map->height; ++y) {
			map_set_solid(map, x, y, pixels[x + y * w], solid_pixels[x + y * w]);
		}
	}
	free(pixels);
	free(solid_pixels);
}

/*
 * Load map from 3 images
 * - one RGB png representing look of the map
 * - one MASK png representing solid tiles
 * - one BACKGROUND png representing tiles that are drawn 
 */
void map_load_mask(map_t *map, const char *fn_rgb, const char *fn_mask, const char *fn_background) {
	unsigned int w, h;
	Uint32 *pixels = game_load_pixels(fn_rgb, &w, &h);
	map->width = w;
	map->height = h;
	char *solid_pixels = malloc(map->width * map->height);
	game_load_solid_pixels(fn_mask, &w, &h, solid_pixels);
	Uint32 *background_pixels = game_load_pixels(fn_background, &w, &h);
	map->back_rgb_tiles = background_pixels;

	for (int x = 0; x < map->width; ++x) {
		for (int y = 0; y < map->height; ++y) {
			char solid = solid_pixels[x + y * w];
			if (solid)
				map_set_solid(map, x, y, pixels[x + y * w], solid);
			else
				map_set_solid(map, x, y, background_pixels[x + y * w], solid);
		
		}
	}
	free(pixels);
	free(solid_pixels);
}

/*
 * Creates a new map and loads it from `fn`
 */
map_t map_new_rgba(const char *fn) {
	unsigned width, height;
	unsigned char *img;
	unsigned error = lodepng_decode32_file(&img, &width, &height, fn);
	if (error) {
		printf("error loading '%s': %s\n", fn, lodepng_error_text(error));
		exit(1);
	}
	free(img);
	
	map_t map = map_new(width, height);
	
	map_load_rgba(&map, fn);

	/* Writes rgb_tiles to texture */
	map_update(&map);
	return map;
}

/*
 * Creates a new map and loads it from `fn`
 */
map_t map_new_mask(const char *fn, const char *fn_mask, const char *fn_bg) {
	unsigned width, height;
	unsigned char *img;
	unsigned error = lodepng_decode32_file(&img, &width, &height, fn);
	if (error) {
		printf("error loading '%s': %s\n", fn, lodepng_error_text(error));
		exit(1);
	}
	free(img);
	
	map_t map = map_new(width, height);
	
	map_load_mask(&map, fn, fn_mask, fn_bg);

	/* Writes rgb_tiles to texture */
	map_update(&map);
	return map;
}

/*
 * Loads a map from folder by using three files
 * 1. background.png
 * 2. rgb.png
 * 3. mask.png
 * only the folder needs to be specified
 */
map_t map_loadnew(const char *dir) {
	char *fn_bg = calloc(strlen(dir) + 14 + 1, 1);
	char *fn_rgb = calloc(strlen(dir) + 7 + 1, 1);
	char *fn_mask = calloc(strlen(dir) + 8 + 1, 1);
	
	/* build dir string */
	strcpy(fn_bg, dir);
	strcat(fn_bg, "background.png");
	strcpy(fn_rgb, dir);
	strcat(fn_rgb, "rgb.png");
	strcpy(fn_mask, dir);
	strcat(fn_mask, "mask.png");
	
	return map_new_mask(fn_rgb, fn_mask, fn_bg);
}

#endif
