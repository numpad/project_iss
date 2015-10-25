#ifndef player_h
#define player_h

#define LIMIT(v,n) if ((v) > (n)) { (v) = n; }

struct player_t;
struct bullet_t;

#include <SDL2/SDL.h>
#include "duktape.h"
#include "vector.h"
#include "map.h"
#include "atlas.h"
#include "soundatlas.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern vector_t mouse;

typedef struct bullet_t {
	vector_t pos, vel;
	float rad;
	char active;
	unsigned int ticks_alive;
	void (*behave)(struct bullet_t *, map_t *);
	duk_context *ctx;
	struct player_t *owner;
} bullet_t;

typedef struct player_t {
	/* dynamic properties */
	vector_t pos, vel;
	vector_t left_foot, right_foot, head, tocenter;
	
	/* look */
	atlas_t sprites;

	/* weapons */
	bullet_t *bullets;
	int bullet_i, bullet_max;
	
	/* aiming */
	vector_t aim_dir;
	float aim_angle, aim_dir_len;
	int aiming_dots;

	/* attributes */
	vector_t size;
	float acceleration, drag, max_speed, turn_speed;
	float max_fallspeed, jump_vel;

	/* animation */
	atlas_t skin;
	unsigned walking_frame, walking_frame_speed;
	SDL_RendererFlip animation_flip;

	/* sounds */
	soundatlas_t sounds;
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
		.tocenter = vector_new(0, -h/2),
		.size = vector_new(w, h),
		.acceleration = 0.1,
		.drag = 0.8,
		.max_speed = 0.8,
		.turn_speed = .7,
		.max_fallspeed = 13,
		.bullets = malloc(3 * sizeof(bullet_t)),
		.bullet_i = 0,
		.bullet_max = 3,
		.aim_dir = vector_new(1, 0),
		.aim_dir_len = 10,
		.aim_angle = 90,
		.aiming_dots = 4,
		.skin = atlas_new(game_load_texture("assets/Kenney/Extra_animations_and_enemies/Spritesheets/alienGreen.png"), 7),
		.walking_frame = 0,
		.walking_frame_speed = 40,
		.animation_flip = SDL_FLIP_NONE,
		.sounds = soundatlas_new(10),
		.sprites = atlas_new(game_load_texture("assets/player/sprites.png"), 1)
	};

	/* Add animation sprites */
	atlas_add_sprite(&p.skin, 70,  92, 66, 92); /* default, id=0 */
	atlas_add_sprite(&p.skin,  0, 380, 69, 71); /* duck */
	atlas_add_sprite(&p.skin,  0, 288, 69, 92); /* hurt */
	atlas_add_sprite(&p.skin, 69, 286, 67, 93); /* jump */
	atlas_add_sprite(&p.skin, 69, 379, 67, 92); /* stand */
	atlas_add_sprite(&p.skin, 69, 193, 68, 93); /* walk1 */
	atlas_add_sprite(&p.skin,  0,   0, 70, 96); /* walk2, id=6 */
	
	/* Add sprites */
	atlas_add_sprite(&p.sprites, 0, 0, 8, 8); /* bullet, id=0 */
	atlas_add_sprite(&p.sprites, 8, 0, 17, 19); /* grenade */
	
	/* Add sounds to atlas */
	soundatlas_add(&p.sounds, "assets/sounds/jump.wav", "jump_1");
	soundatlas_add(&p.sounds, "assets/sounds/jump2.wav", "jump_2");
	soundatlas_add(&p.sounds, "assets/sounds/explode.wav", "explode_1");
	soundatlas_add(&p.sounds, "assets/sounds/explode_small.wav", "explode_small_1");
	soundatlas_add(&p.sounds, "assets/sounds/shoot.wav", "shoot_1");
	soundatlas_add(&p.sounds, "assets/sounds/hurt1.wav", "hurt_1");

	return p;
}

void bullet_delete(bullet_t *b) {
	duk_destroy_heap(b->ctx);
}

void player_delete(player_t *player) {
	bullet_delete(&player->bullets[0]);
	free(player->bullets);
	soundatlas_delete(&player->sounds);
	atlas_delete(&player->skin);
	atlas_delete(&player->sprites);
}

void behave_shot(bullet_t *shot, map_t *map) {
	if (!shot->active)
		return;
	
	shot->pos = vector_add(shot->pos, shot->vel);

	float dst = 0;
	if ((dst = map_raycast(map, shot->pos, shot->vel)) < vector_len(shot->vel)) {
		map_explode(map, shot->pos.x, shot->pos.y, 10, 4, RGB(140, 80, 65));
		shot->active = 0;
		soundatlas_play(&shot->owner->sounds, "hurt_1");
		return;
	}
}

void behave_drill(bullet_t *shot, map_t *map) {
	if (!shot->active)
		return;
	
	shot->pos = vector_add(shot->pos, shot->vel);
	
	if (shot->ticks_alive == 0) {
		float dst = 0;
		if ((dst = map_raycast(map, shot->pos, shot->vel)) < vector_len(shot->vel)) {
			map_explode(map, shot->pos.x, shot->pos.y, 15, 4, RGB(140, 80, 65));
			shot->ticks_alive = 1;
		}
	} else {
		shot->vel = vector_smult(shot->vel, 0.85);
		map_explode(map, shot->pos.x, shot->pos.y, 15 - shot->ticks_alive / 2, 4, RGB(140, 80, 65));
		if (++shot->ticks_alive > 10)
			shot->active = 0;
	}
}

/*
 * WEAPON: GRENADE
 */
void bullet_grenade_create(bullet_t *grenade) {
	soundatlas_play(&grenade->owner->sounds, "shoot_1");
}
void bullet_grenade_behave(bullet_t *grenade, map_t *map) {
	if (!grenade->active) {
		return;
	}
	
	if (map_raycast(map, grenade->pos, grenade->vel) <= vector_len(grenade->vel)) {
		if (map_raycast(map, vector_add(grenade->pos, grenade->vel), vector_new(-1, 0)) < 2) {
			grenade->vel.x *= -0.8;
		}
		if (map_raycast(map, vector_add(grenade->pos, grenade->vel), vector_new(1, 0)) < 2) {
			grenade->vel.x *= -0.8;
		}
		if (map_raycast(map, vector_add(grenade->pos, grenade->vel), vector_new(0, -1)) < 2) {
			grenade->vel.y *= -0.8;
		}
		if (map_raycast(map, vector_add(grenade->pos, grenade->vel), vector_new(0, 1)) < 2) {
			grenade->vel.y *= -0.8;
		}
	}
	
	if (map_raycast(map, vector_add(grenade->pos, grenade->vel), vector_new(0, 1)) > vector_len(grenade->vel)) {
		grenade->vel.y += map->gravity;
	}
	grenade->pos = vector_add(grenade->pos, grenade->vel);
	
	if (grenade->ticks_alive > 260) {
		soundatlas_play(&grenade->owner->sounds, "explode_1");
		map_explode(map, grenade->pos.x, grenade->pos.y, 25, 4, RGB(140, 80, 65));
		grenade->active = 0;
	}

	++grenade->ticks_alive;
}

void player_grenade_new(player_t *player) {
	if (player->bullets[player->bullet_i].active)
		return;
	bullet_t grenade = (bullet_t) {
		.pos = vector_sub(vector_add(player->pos, player->tocenter), vector_new(4, 4)),
		.vel = vector_add(vector_smult(player->vel, 0.5), vector_tolen(player->aim_dir, 8)),
		.rad = 10,
		.active = 1,
		.ticks_alive = 0,
		.behave = &bullet_grenade_behave,
		.ctx = duk_create_heap_default(),
		.owner = player,
	};
	player->bullets[player->bullet_i] = grenade;
	
}

/*
 * Sets the players dimensions and updates "dependencies"
 */
void player_setsize(player_t *player, int w, int h) {
	player->left_foot = vector_new(-w / 2, 0);
	player->right_foot = vector_new(w / 2, 0);
	player->head = vector_new(0, -h);
	player->tocenter = vector_new(0, -h / 2);
	player->size = vector_new(w, h);
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
	player_setsize(player, player->size.x, player->size.y);

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
	duk_get_global_string(ctx, "jump_vel");
	player->jump_vel = (float)duk_get_number(ctx, -1);
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
		float dist_y1 = map_raycast(map, vector_add(vector_add(player->pos, vector_new(0, -2)), player->left_foot), vector_new(-1, 0));
		
		if (dist <= 1 && dist_y1 > dist) {
			player->pos.y -= 1;
		} else if (dist <= 1) {
			player->vel.x = 0;
		}
	} else if (player->vel.x > 0) {
		float dist = map_raycast(map, vector_add(vector_add(player->pos, vector_new(0, -1)), player->right_foot), vector_new(1, 0));
		float dist_y1 = map_raycast(map, vector_add(vector_add(player->pos, vector_new(0, -2)), player->right_foot), vector_new(1, 0));
		
		if (dist <= 1 && dist_y1 > dist) {
			player->pos.y -= 1;
		} else if (dist <= 1) {
			player->vel.x = 0;
		}
	}
}

/*
 * Handles jumping
 */
void player_jump(player_t *player, map_t *map) {
	if (player->vel.y == 0) {
		player->vel.y = -player->jump_vel;
		
		/* random jump sound */
		if ((int)(player->pos.x + player->pos.y + player->vel.x + player->vel.y) % 2 == 0)
			soundatlas_play(&player->sounds, "jump_1");
		else
			soundatlas_play(&player->sounds, "jump_2");
			
	}
}
/*
 * Handles collision when jumping
 */
void player_jump_collide(player_t *player, map_t *map) {
	int collides = 0;
	for (int x = 0; x < (int)player->size.x / 2; ++x) {
		if (map_get_solid(map, (int)player->pos.x + x, player->pos.y - player->size.y) || map_get_solid(map, (int)player->pos.x - x, player->pos.y - player->size.y)) {
			collides = 1;
			break;
		}
	}

	if (collides) {
		player->vel.y = 0.5;
	}
}

/*
 * Update everything player-related (collision and movement etc)
 */
void player_update(player_t *player, map_t *map) {
	/* Aiming */
	player->aim_dir = vector_sub(vector_add(mouse, map->scroll), vector_add(player->pos, player->tocenter));
	vector_setlen(&player->aim_dir, player->aim_dir_len);
	
	/* Physics */
	player_fall(player, map);
	player_jump_collide(player, map);

	/* Animation */
	player->walking_frame = (player->walking_frame + 1) % player->walking_frame_speed;
	if (player->aim_dir.x < 0) {
		player->animation_flip = SDL_FLIP_HORIZONTAL;
	}
	if (player->aim_dir.x > 0) {
		player->animation_flip = SDL_FLIP_NONE;
	}
}

/*
 * Renders `player` to renderer
 */
void player_render(player_t *player, map_t *map) {
	
	const Uint8 *key = SDL_GetKeyboardState(NULL);	
	SDL_Rect player_realdst = (SDL_Rect){
		player->pos.x - player->size.x / 2 - map->scroll.x,
		player->pos.y - player->size.y - map->scroll.y,
		player->size.x,
		player->size.y
	};
	
	/* How to display player */
	if (player->vel.y == 0) {
		if (key[SDL_SCANCODE_D]) {
			player->animation_flip = SDL_FLIP_NONE;
			atlas_render_ex(&player->skin, (player->walking_frame < (player->walking_frame_speed / 2) ? 5 : 6), &player_realdst, 0, player->animation_flip);
		} else if (key[SDL_SCANCODE_A]) {
			player->animation_flip = SDL_FLIP_HORIZONTAL;
			atlas_render_ex(&player->skin, (player->walking_frame < (player->walking_frame_speed / 2) ? 5 : 6), &player_realdst, 0, player->animation_flip);
		} else {
			atlas_render_ex(&player->skin, 4, &player_realdst, 0, player->animation_flip);
		}
	} else {
		atlas_render_ex(&player->skin, 3, &player_realdst, 0, player->animation_flip);
	}

	vector_t dot_pos = vector_add(player->pos, player->tocenter);
	for (int i = 0; i < player->aiming_dots; ++i) {
		dot_pos = vector_add(vector_add(player->pos, player->tocenter), player->aim_dir);
		dot_pos = vector_add(dot_pos, vector_smult(player->aim_dir, i));

		aacircleRGBA(renderer, dot_pos.x - map->scroll.x, dot_pos.y - map->scroll.y, 2, 255, 255, 255, 255);
	}
	
	/* draw bullets */
	int bw, bh;
	atlas_getsize(&player->sprites, 1, &bw, &bh);

	if (player->bullets[0].active) {
		SDL_Rect sprite_dst = (SDL_Rect) {
			player->bullets[0].pos.x - map->scroll.x - bw/2,
			player->bullets[0].pos.y - map->scroll.y - bh/2,
			bw, bh
		};
		atlas_render_ex(&player->sprites, 1, &sprite_dst, -player->bullets[0].vel.x, SDL_FLIP_NONE);
	}

}

#endif
