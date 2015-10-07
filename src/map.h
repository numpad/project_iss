#ifndef map_h
#define map_h

#include <stdlib.h>
#include <SDL2/SDL.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

typedef struct {
	int width, height;
	SDL_Texture *texture;
	Uint32 *rgb_tiles;
	char *solid_tiles;
	
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
		.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height),
		.rgb_tiles = malloc(width * height * sizeof(Uint32)),
		.solid_tiles = calloc(width * height, sizeof(char)),
		.gravity = 0.275
	};
	
	/* Writes rgb_tiles to texture */
	map_update(&map);
	return map;
}

/*
 * Changes configuration
 */
void map_configure(map_t *map, float gravity) {
	map->gravity = gravity;
}

/*
 * Updates the texture by writing map::rgb_tiles to it
 */
void map_update(map_t *map) {
	SDL_UpdateTexture(map->texture, NULL, map->rgb_tiles, map->width * sizeof(Uint32));
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
 * Renders `map` to the screen
 */
void map_render(map_t *map) {
	
	/*
	for (int y = 0; y < map->height; ++y) {
		for (int x = 0; x < map->width; ++x) {
			Uint32 color = map_get(map, x, y);
			
			if (map_get_solid(map, x, y)) {
				SDL_SetRenderDrawColor(renderer, color >> 16, color >> 8 & 255, color & 255, 255);
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}
	}
	*/
	
	SDL_RenderCopy(renderer, map->texture, NULL, NULL);
}

/*
 * Frees all memory and destroys the texture used by map
 */
void map_delete(map_t *map) {
	SDL_DestroyTexture(map->texture);
	free(map->rgb_tiles);
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
	for (int x = -r; x < r; ++x) {
		for (int y = -r; y < r; ++y) {
			if (x*x + y*y <= r*r && map_in_bounds(map, xp + x, yp + y)) {
				map_set_solid(map, xp + x, yp + y, c, solid);
			}
		}
	}
	
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

#endif
