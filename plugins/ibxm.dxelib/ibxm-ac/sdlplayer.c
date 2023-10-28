
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"

#include "SDL.h"
#include "SDL_main.h"

#include "ibxm.h"
#include "patterndisplay.h"

/*
	Simple command-line test player for ibxm-ac using SDL.
*/

#define SAMPLING_FREQ 48000 /* 48khz. */
#define REVERB_BUF_LEN 4800 /* 50ms. */
#define BUFFER_SAMPLES 2048 /* 8k buffer. */

static short reverb_buffer[ REVERB_BUF_LEN ];
static int mix_buffer[ SAMPLING_FREQ / 3 ];
static int mix_len, mix_idx, reverb_len, reverb_idx, loop, display;
static int samples_remaining, redraw_event, song_end_event;

/* Simple stereo cross delay with feedback. */
static void reverb( short *buffer, int count ) {
	int buffer_idx, buffer_end;
	if( reverb_len > 2 ) {
		buffer_idx = 0;
		buffer_end = buffer_idx + ( count << 1 );
		while( buffer_idx < buffer_end ) {
			buffer[ buffer_idx ] = ( buffer[ buffer_idx ] * 3 + reverb_buffer[ reverb_idx + 1 ] ) >> 2;
			buffer[ buffer_idx + 1 ] = ( buffer[ buffer_idx + 1 ] * 3 + reverb_buffer[ reverb_idx ] ) >> 2;
			reverb_buffer[ reverb_idx ] = buffer[ buffer_idx ];
			reverb_buffer[ reverb_idx + 1 ] = buffer[ buffer_idx + 1 ];
			reverb_idx += 2;
			if( reverb_idx >= reverb_len ) {
				reverb_idx = 0;
			}
			buffer_idx += 2;
		}
	}
}

static void clip_audio( int *input, short *output, int count ) {
	int idx, end, sam;
	for( idx = 0, end = count << 1; idx < end; idx++ ) {
		sam = input[ idx ];
		if( sam < -32768 ) {
			sam = -32768;
		}
		if( sam > 32767 ) {
			sam = 32767;
		}
		output[ idx ] = sam;
	}
}

static void get_audio( struct replay *replay, short *buffer, int count ) {
	int buf_idx = 0, remain;
	while( buf_idx < count ) {
		if( mix_idx >= mix_len ) {
			mix_len = replay_get_audio( replay, mix_buffer, pattern_display_get_mute() );
			mix_idx = 0;
		}
		remain = mix_len - mix_idx;
		if( ( buf_idx + remain ) > count ) {
			remain = count - buf_idx;
		}
		clip_audio( &mix_buffer[ mix_idx << 1 ], &buffer[ buf_idx << 1 ], remain );
		mix_idx += remain;
		buf_idx += remain;
	}
}

static void audio_callback( void *udata, Uint8 *stream, int len ) {
	struct replay *replay = ( struct replay * ) udata;
	SDL_Event event = { 0 };
	int count = len / 4;
	if( display ) {
		/* Update display. */
		event.type = redraw_event;
		event.user.code = ( replay_get_sequence_pos( replay ) << 8 ) | replay_get_row( replay );
		SDL_PushEvent( &event );
	}
	if( samples_remaining < count ) {
		/* Clear output.*/
		memset( stream, 0, len );
		count = samples_remaining;
	}
	if( count > 0 ) {
		/* Get audio from replay.*/
		get_audio( ( struct replay * ) udata, ( short * ) stream, count );
		reverb( ( short * ) stream, count );
		if( !loop ) {
			samples_remaining -= count;
		}
	} else {
		/* Notify the main thread to stop playback.*/
		event.type = song_end_event;
		SDL_PushEvent( &event );
	}
}

static void termination_handler( int signum ) {
	SDL_Event event = { 0 };
	/* Notify the main thread to stop playback. */
	event.type = song_end_event;
	SDL_PushEvent( &event );
	fprintf( stderr, "\nTerminated!\n" );
}

static long read_file( char *file_name, void *buffer ) {
	long file_length = -1, bytes_read;
	FILE *input_file = fopen( file_name, "rb" );
	if( input_file != NULL ) {
		if( fseek( input_file, 0L, SEEK_END ) == 0 ) {
			file_length = ftell( input_file );
			if( file_length >= 0 && buffer ) {
				if( fseek( input_file, 0L, SEEK_SET ) == 0 ) {
					bytes_read = fread( buffer, 1, file_length, input_file ); 
					if( bytes_read != file_length ) {
						file_length = -1;
					}
				} else {
					file_length = -1;
				}
			}
		}
		fclose( input_file );
	}
	if( file_length < 0 ) {
		fprintf( stderr, "Unable to read '%s'. %s\n", file_name, strerror( errno ) );
	}
	return file_length;
}

static void print_module_info( struct module *module ) {
	int idx = 0, end = 0;
	for( idx = 0, end = ( module->num_instruments + 1 ) / 2; idx < end; idx++ ) {
		printf( "%03i - %-32s ", idx, idx ? module->instruments[ idx ].name : module->name );
		printf( "%03i - %-32s\n", idx + end, module->instruments[ idx + end ].name );
	}
	if( idx + end <= module->num_instruments ) {
		printf( "%39s%03i - %-32s\n", "", idx + end, module->instruments[ idx + end ].name );
	}
}

static int play_module( struct module *module, int interpolation, int display ) {
	int result = 1;
	SDL_AudioSpec audiospec;
	SDL_Event event = { 0 };
	/* Initialise replay.*/
	struct replay *replay = new_replay( module, SAMPLING_FREQ, interpolation );
	if( replay ) {
		/* Calculate song length. */
		samples_remaining = replay_calculate_duration( replay );
		printf( "Song Duration: %d seconds.\n", samples_remaining / SAMPLING_FREQ );
		fflush( NULL );
		/* Initialise SDL_AudioSpec Structure. */
		memset( &audiospec, 0, sizeof( SDL_AudioSpec ) );
		audiospec.freq = SAMPLING_FREQ;
		audiospec.format = AUDIO_S16SYS;
		audiospec.channels = 2;
		audiospec.samples = BUFFER_SAMPLES;
		audiospec.callback = audio_callback;
		audiospec.userdata = replay;
		/* Initialise audio subsystem. */
		result = SDL_Init( SDL_INIT_AUDIO | ( display ? SDL_INIT_VIDEO : 0 ) );
		if( result == 0 ) {
			if( display ) {
				display = pattern_display_open();
			}
			/* Open the audio device. */
			result = SDL_OpenAudio( &audiospec, NULL );
			if( result == 0 ) {
				/* Register events. */
				redraw_event = SDL_RegisterEvents( 1 );
				song_end_event = SDL_RegisterEvents( 1 );
				/* Begin playback. */
				SDL_PauseAudio( 0 );
				/* Wait for playback to finish. */
				while( event.type != SDL_QUIT ) {
					SDL_WaitEvent( &event );
					if( event.type == song_end_event ) {
						break;
					} else if( event.type == redraw_event ) {
						pattern_display_redraw_event( &event, module );
					} else if( event.type == SDL_MOUSEBUTTONDOWN ) {
						pattern_display_button_event( &event, module );
					}
				}
				/* Shut down. */
				SDL_CloseAudio();
				pattern_display_close();
				SDL_Quit();
			} else {
				fprintf( stderr, "Unable to open audio device: %s\n", SDL_GetError() );
			}
		} else {
			fprintf( stderr, "Unable to initialise SDL: %s\n", SDL_GetError() );
		}
		dispose_replay( replay );
	} else {
		fprintf( stderr, "Unable to initialise replay.\n" );
	}
	return result;
}

int main( int argc, char **argv ) {
	int arg, length, result = EXIT_FAILURE;
	int interpolation = 0;
	char *filename = NULL, *input, message[ 64 ] = "";
	struct data data;
	struct module *module;
	for( arg = 1; arg < argc; arg++ ) {
		/* Parse arguments.*/
		if( strcmp( argv[ arg ], "-loop" ) == 0 ) {
			loop = 1;
		} else if( strcmp( argv[ arg ], "-reverb" ) == 0 ) {
			reverb_len = REVERB_BUF_LEN;
		} else if( strcmp( argv[ arg ], "-interp" ) == 0 ) {
			interpolation = 1;
		} else if( strcmp( argv[ arg ], "-display" ) == 0 ) {
			display = 1;
		} else {
			filename = argv[ arg ];
		}
	}
	if( filename == NULL ) {
		fprintf( stderr, "%s\nUsage: %s [-display] [-loop] [-reverb] [-interp] module.xm\n", IBXM_VERSION, argv[ 0 ] );
	} else {
		/* Read module file.*/
		length = read_file( filename, NULL );
		if( length >= 0 ) {
			input = calloc( length, 1 );
			if( input != NULL ) {
				if( read_file( filename, input ) >= 0 ) {
					data.buffer = input;
					data.length = length;
					module = module_load( &data, message );
					if( module ) {
						print_module_info( module );
						/* Install signal handlers.*/
						signal( SIGTERM, termination_handler );
						signal( SIGINT,  termination_handler );
						/* Play.*/
						if( play_module( module, interpolation, display ) == 0 ) {
							result = EXIT_SUCCESS;
						}
						dispose_module( module );
					} else {
						fputs( message, stderr );
						fputs( "\n", stderr );
					}
				}
				free( input );
			}
		}
	}
	return result;
}
