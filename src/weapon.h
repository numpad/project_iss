#ifndef weapon_h
#define weapon_h

#include "duktape.h"

typedef struct {
	int ammo;
	
	duk_context *ctx;
} weapon_t;

weapon_t weapon_new(const char *script) {
	weapon_t weapon = (weapon_t) {
		.ammo = 0,
		.ctx = duk_create_heap_default()
	};

	return weapon;
}

void weapon_delete(weapon_t *weapon) {
	duk_destroy_heap(weapon->ctx);
}
#endif
