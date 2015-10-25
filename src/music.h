#ifndef music_h
#define music_h

typedef struct {
	Mix_Music *music;
} music_t;

/*
 * Loads a new music effect (currently only WAV)
 */
music_t music_new(const char *fn) {
	music_t msc = (music_t) {
		.music = Mix_LoadMUS(fn)
	};
	
	if (msc.music == NULL) {
		printf("SDL_mixer error: failed to load music '%s': %s\n", fn, Mix_GetError());
	}

	return msc;
}

/*
 * Cleans up `msc`
 */
void music_delete(music_t *msc) {
	Mix_FreeMusic(msc->music);
}

/*
 * Plays music
 */
void music_play(music_t *msc) {
	Mix_PlayMusic(msc->music, -1);
}

/*
 * Resumes music
 */
void music_resume() {
	Mix_ResumeMusic();
}

/*
 * Pauses music
 */
void music_pause() {
	Mix_PauseMusic();
}

/*
 * Halts music
 */
void music_halt() {
	Mix_HaltMusic();
}


#endif
