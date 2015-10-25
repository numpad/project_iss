#ifndef sound_h
#define sound_h

typedef struct {
	Mix_Chunk *chunk;
} sound_t;

/*
 * Loads a new sound effect (currently only WAV)
 */
sound_t sound_new(const char *fn) {
	sound_t snd = (sound_t) {
		.chunk = Mix_LoadWAV(fn)
	};
	
	if (snd.chunk == NULL) {
		printf("SDL_mixer error: failed to load sound effect '%s': %s\n", fn, Mix_GetError());
	}

	return snd;
}

/*
 * Cleans up `snd`
 */
void sound_delete(sound_t *snd) {
	Mix_FreeChunk(snd->chunk);
}

/*
 * Plays a sound
 */
void sound_play(sound_t *snd) {
	Mix_PlayChannel(-1, snd->chunk, 0);
}

#endif
