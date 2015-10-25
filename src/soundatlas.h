#ifndef sound_atlas_h
#define sound_atlas_h

#include <stdlib.h>
#include <string.h>
#include "sound.h"

typedef struct {
	sound_t *sounds;
	char **sound_names;
	int sounds_allocated, max_sounds;

} soundatlas_t;

/*
 * Creates a new soundatlas
 */
soundatlas_t soundatlas_new(int sounds_count) {
	soundatlas_t sndatlas = (soundatlas_t) {
		.sounds = malloc(sounds_count * sizeof(sound_t)),
		.sound_names = malloc(sounds_count * sizeof(char*)),
		.sounds_allocated = 0,
		.max_sounds = sounds_count
	};
	
	return sndatlas;
}

/*
 * Clean up soundatlas
 */
void soundatlas_delete(soundatlas_t *sa) {
	free(sa->sounds);
	free(sa->sound_names);
}

/*
 * Add a sound to the collection
 */
void soundatlas_add(soundatlas_t *sa, const char *fn, char *name) {
	/* allocate new memory if we dont have enough */
	if (sa->sounds_allocated == sa->max_sounds - 1) {
		sa->max_sounds += 1;
		sa->sounds = realloc(sa->sounds, sa->max_sounds * sizeof(sound_t));
		sa->sound_names = realloc(sa->sound_names, sa->max_sounds * sizeof(char*));
	}
	
	/* add sound to soundatlas */
	sa->sounds[sa->sounds_allocated] = sound_new(fn);
	sa->sound_names[sa->sounds_allocated] = name;
	
	sa->sounds_allocated += 1;
}

/*
 * Plays a sound
 */
void soundatlas_play(soundatlas_t *sa, const char *name) {
	int sound_name_idx = -1;
	for (int idx = 0; idx < sa->sounds_allocated; ++idx) {
		if (!strcmp(sa->sound_names[idx], name)) {
			sound_name_idx = idx;
			break;
		}
	}
	
	
	if (sound_name_idx > -1)
		sound_play(&sa->sounds[sound_name_idx]);
}

#endif
