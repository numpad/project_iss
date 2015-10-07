#ifndef player_h
#define player_h

#define LIMIT(v,n) if ((v) > (n)) { (v) = n; }

#include "duktape.h"
#include "vector.h"
#include "map.h"
#include "weapon.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;

typedef struct {
	/* dynamic properties */
	vector_t pos, vel;
	vector_t left_foot, right_foot, head;
	
	/* weapon */
	int current_weapon, weapons_count, weapons_max_count;
	weapon_t *weapons;

	/* attributes */
	vector_t size;
	float acceleration, drag, max_speed, turn_speed;
	float max_fallspeed;
} player_t;

/*
 * Create a new player at `x`,`y`
 */
player_t player_new(float x, float y, float w, float h) {
	player_t p = (player_t) {
		.pos = vector_new(x, y),
		.vel = vector_new(0, 0),
		.left_foot = vector_new(-w / 2, 0),
		.right_foot = vector_new(w / 2, 0),
		.head = vector_new(0, -h),
		.size = vector_new(w, h),
		.acceleration = 0.1,
		.drag = 0.8,
		.max_speed = 0.8,
		.turn_speed = .7,
		.max_fallspeed = 13
		//.weapons = malloc(1),
		//.weapons_max_count = 1,
		//.current_weapon = 0,
		//.weapons_count = 0
	};
	
	return p;
}

/*
 * Change settings of `player`
 */
void player_configure(player_t *player, const float acceleration, const float drag, const float max_speed, const float turn_speed) {
	player->acceleration = acceleration;
	player->drag = drag;
	player->max_speed = max_speed;
	player->turn_speed = turn_speed;
}

/*
 * Loads configuration and applies it to `player`
 */
void player_loadconfig(player_t *player, const char *filename) {
	duk_context *ctx = duk_create_heap_default();
	
	if (duk_peval_file(ctx, filename) != 0) {
		printf("Failed to eval '%s': %s\n", filename, duk_to_string(ctx, -1));
		duk_destroy_heap(ctx);
		return;
	}
	
	duk_get_global_string(ctx, "width");
	player->size.x = (float)duk_get_number(ctx, -1);
	duk_pop(ctx);
	duk_get_global_string(ctx, "height");
	player->size.y = (float)duk_get_number(ctx, -1);
	duk_pop(ctx);
	duk_get_global_string(ctx, "acceleration");
	player->acceleration= (float)duk_get_number(ctx, -1);
	duk_pop(ctx);
	duk_get_global_string(ctx, "drag");
	player->drag = (float)duk_get_number(ctx, -1);
	duk_pop(ctx);
	duk_get_global_string(ctx, "max_speed");
	player->max_speed= (float)duk_get_number(ctx, -1);
	duk_pop(ctx);
	duk_get_global_string(ctx, "turn_speed");
	player->turn_speed = (float)duk_get_number(ctx, -1);
	duk_pop(ctx);
	duk_get_global_string(ctx, "max_fallspeed");
	player->max_fallspeed = (float)duk_get_number(ctx, -1);
	duk_pop(ctx);
	

	duk_destroy_heap(ctx);
}

/*
 * Handles gravity and falling-collision
 */
void player_fall(player_t *player, map_t *map) {
	int collides = 0;
	for (int x = 0; x < (int)player->size.x / 2; ++x) {
		if (map_get_solid(map, (int)player->pos.x + x, player->pos.y) || map_get_solid(map, (int)player->pos.x - x, player->pos.y)) {
			collides = 1;
			break;
		}
	}

	if (!collides) {
		player->vel.y += map->gravity;
		LIMIT(player->vel.y, player->max_fallspeed);
		
		float dist_to_floor;
		if ((dist_to_floor = map_raycast(map, player->pos, vector_new(0, 1))) < player->vel.y) {
			player->vel.y = 0;
			player->pos.y += dist_to_floor;
		}
	
	} else if (player->vel.y > 0){
		player->vel.y = 0;
	}

	player->pos = vector_add(player->pos, player->vel);
}

/*
 * Handles movement to right/left and collision
 */
void player_move(player_t *player, map_t *map, float dir) {
	/* if not moving in any direction, slow down */
	if (dir == 0) {
		player->vel.x *= player->drag;
		return;
	}
	
	/* move in specified directiom */
	player->vel.x += player->acceleration * dir;
	
	/* max speed (left) */
	if (dir < 0.0) {
		/* fast turn */
		if (player->vel.x > 0) {
			player->vel.x *= player->turn_speed;
		}

		if (player->vel.x < -player->max_speed)
			player->vel.x = -player->max_speed;
		

	} else { /* max speed (right) */
		/* fast turn */
		if (player->vel.x < 0) {
			player->vel.x *= player->turn_speed;
		}

		if (player->vel.x > player->max_speed)
			player->vel.x = player->max_speed;
		
	}

	if (player->vel.x < 0) {
		float dist = map_raycast(map, vector_add(vector_add(player->pos, vector_new(0, -1)), player->left_foot), vector_new(-1, 0));
		if (dist <= 1) {
			player->vel.x = 0;
		}
	} else if (player->vel.x > 0) {
		float dist = map_raycast(map, vector_add(vector_add(player->pos, vector_new(0, -1)), player->right_foot), vector_new(1, 0));
		if (dist <= 1) {
			player->vel.x = 0;
		}
	}
}

/*
 * Handles jumping and collision
 */
void player_jump(player_t *player, map_t *map) {
	if (player->vel.y == 0) {
		player->vel.y = -5.5;
	}
}

void player_update(player_t *player, map_t *map) {
	player_fall(player, map);
	
}

/*
 * Renders `player` to renderer
 */
void player_render(player_t *player) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(renderer, &(SDL_Rect){
		player->pos.x - player->size.x / 2,
		player->pos.y - player->size.y,
		player->size.x,
		player->size.y
	});
	
	SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
	SDL_RenderDrawLine(renderer, vector_add(player->pos, player->left_foot).x, vector_add(player->pos, player->left_foot).y, vector_add(player->pos, player->left_foot).x, vector_add(player->pos, player->left_foot).y + 10);
	SDL_RenderDrawLine(renderer, vector_add(player->pos, player->right_foot).x, vector_add(player->pos, player->right_foot).y, vector_add(player->pos, player->right_foot).x, vector_add(player->pos, player->right_foot).y + 10);
	
}

/*
void player_addweapon(player_t *player, weapon_t weapon) {
	player->weapons[player->weapons_count] = weapon;

}
*/

#endif
