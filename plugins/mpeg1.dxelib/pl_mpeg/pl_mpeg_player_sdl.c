/*
PL_MPEG Example - Video player using SDL2's accelerated 2d renderer
SPDX-License-Identifier: MIT

Dominic Szablewski - https://phoboslab.org
Siteswap - https://github.com/siteswapv4


-- Usage

pl_mpeg_player_sdl <video-file.mpg>

Use the arrow keys to seek forward/backward by 3 seconds. Click anywhere on the
window to seek to seek through the whole file.


-- About

This program demonstrates a simple video/audio player using plmpeg for decoding
and SDL2 with the accelerated 2d renderer.

*/

#include <stdlib.h>
#include <stdio.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

typedef struct {
	plm_t *plm;
	double last_time;
	int wants_to_quit;
	
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Rect rectangle;
	SDL_AudioDeviceID audio_device;
} app_t;

app_t * app_create(const char *filename);
void app_update(app_t *self);
void app_destroy(app_t *self);

void app_on_video(plm_t *player, plm_frame_t *frame, void *user);
void app_on_audio(plm_t *player, plm_samples_t *samples, void *user);



app_t * app_create(const char *filename) {
	app_t *self = (app_t *)malloc(sizeof(app_t));
	memset(self, 0, sizeof(app_t));
	
	// Initialize plmpeg, load the video file, install decode callbacks
	self->plm = plm_create_with_filename(filename);
	if (!self->plm) {
		SDL_Log("Couldn't open %s", filename);
		exit(1);
	}

	if (!plm_probe(self->plm, 5000 * 1024)) {
		SDL_Log("No MPEG video or audio streams found in %s", filename);
		exit(1);
	}

	int samplerate = plm_get_samplerate(self->plm);

	SDL_Log(
		"Opened %s - framerate: %f, samplerate: %d, duration: %f",
		filename, 
		plm_get_framerate(self->plm),
		plm_get_samplerate(self->plm),
		plm_get_duration(self->plm)
	);
	
	plm_set_video_decode_callback(self->plm, app_on_video, self);
	plm_set_audio_decode_callback(self->plm, app_on_audio, self);
	
	plm_set_loop(self->plm, TRUE);
	plm_set_audio_enabled(self->plm, TRUE);
	plm_set_audio_stream(self->plm, 0);

	if (plm_get_num_audio_streams(self->plm) > 0) {
		// Initialize SDL Audio
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		SDL_AudioSpec audio_spec;
		SDL_memset(&audio_spec, 0, sizeof(audio_spec));
		audio_spec.freq = samplerate;
		audio_spec.format = AUDIO_F32;
		audio_spec.channels = 2;
		audio_spec.samples = 4096;

		self->audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
		if (self->audio_device == 0) {
			SDL_Log("Failed to open audio device: %s", SDL_GetError());
		}
		SDL_PauseAudioDevice(self->audio_device, 0);

		// Adjust the audio lead time according to the audio_spec buffer size
		plm_set_audio_lead_time(self->plm, (double)audio_spec.samples / (double)samplerate);
	}
	
	// Create SDL Window
	self->window = SDL_CreateWindow(
		"pl_mpeg",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		plm_get_width(self->plm), plm_get_height(self->plm),
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	// Create SDL Renderer with vsync
	self->renderer = SDL_CreateRenderer(
		self->window,
		-1,
		SDL_RENDERER_PRESENTVSYNC
	);

	// Create SDL texture with YUV format
	self->texture = SDL_CreateTexture(
		self->renderer,
		SDL_PIXELFORMAT_IYUV,
		SDL_TEXTUREACCESS_STREAMING,
		plm_get_width(self->plm),
 		plm_get_height(self->plm)
	);

	// Adjust rectangle to video size
	self->rectangle.w = plm_get_width(self->plm);
	self->rectangle.h = plm_get_height(self->plm);

	// Renderer will keep the aspect ratio of the video and center the content if the window is resized
	SDL_RenderSetLogicalSize(self->renderer, plm_get_width(self->plm), plm_get_height(self->plm));

	// Mouse position based on the renderer logical size
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");

	// Best render scale quality
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	return self;
}

void app_destroy(app_t *self) {
	plm_destroy(self->plm);

	if (self->audio_device) {
		SDL_CloseAudioDevice(self->audio_device);
	}

	if (self->texture) {
		SDL_DestroyTexture(self->texture);
	}

	if (self->renderer) {
		SDL_DestroyRenderer(self->renderer);
	}

	if (self->window) {
		SDL_DestroyWindow(self->window);
	}

	SDL_Quit();
	
	free(self);
}

void app_update(app_t *self) {
	double seek_to = -1;

	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		if (
			ev.type == SDL_QUIT || 
			(ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_ESCAPE)
		) {
			self->wants_to_quit = TRUE;
		}
		// Seek 3sec forward/backward using arrow keys
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_RIGHT) {
			seek_to = plm_get_time(self->plm) + 3;
		}
		else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_LEFT) {
			seek_to = plm_get_time(self->plm) - 3;
		}
	}


	// Clear renderer, copy texture and present
	SDL_RenderClear(self->renderer);
        
	SDL_RenderCopy(self->renderer, self->texture, NULL, &self->rectangle);

	SDL_RenderPresent(self->renderer);

	// Compute the delta time since the last app_update(), limit max step to 
	// 1/30th of a second
	double current_time = (double)SDL_GetTicks() / 1000.0;
	double elapsed_time = current_time - self->last_time;
	if (elapsed_time > 1.0 / 30.0) {
		elapsed_time = 1.0 / 30.0;
	}
	self->last_time = current_time;

	// Seek using mouse position
	int mouse_x, mouse_y;
	if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		int sx, sy;
		SDL_GetWindowSize(self->window, &sx, &sy);
		seek_to = plm_get_duration(self->plm) * ((float)mouse_x / (float)sx);
	}
	
	// Seek or advance decode
	if (seek_to != -1) {
		SDL_ClearQueuedAudio(self->audio_device);
		plm_seek(self->plm, seek_to, FALSE);
	}
	else {
		plm_decode(self->plm, elapsed_time);
	}

	if (plm_has_ended(self->plm)) {
		self->wants_to_quit = TRUE;
	}
}

void app_on_video(plm_t *mpeg, plm_frame_t *frame, void *user) {
	app_t *self = (app_t *)user;

	SDL_UpdateYUVTexture(self->texture, NULL, frame->y.data, frame->y.width, frame->cb.data, frame->cb.width, frame->cr.data,  frame->cr.width);
}

void app_on_audio(plm_t *mpeg, plm_samples_t *samples, void *user) {
	app_t *self = (app_t *)user;

	// Hand the decoded samples over to SDL
	
	int size = sizeof(float) * samples->count * 2;
	SDL_QueueAudio(self->audio_device, samples->interleaved, size);
}



int main(int argc, char *argv[]) {
	if (argc < 2) {
		SDL_Log("Usage: pl_mpeg_player_sdl <file.mpg>");
		exit(1);
	}

	SDL_SetMainReady();
	
	app_t *app = app_create(argv[1]);
	while (!app->wants_to_quit) {
		app_update(app);
	}
	app_destroy(app);
	
	return EXIT_SUCCESS;
}
