#ifndef bullet_h
#define bullet_h

#include "player.h"

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


#endif
