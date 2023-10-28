
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <windows.h>
#include <mmsystem.h>

#include "micromod.h"

/*
	Simple command-line test player for micromod using the Windows MMAPI.
*/

#define SAMPLING_FREQ  48000  /* 48khz. */
#define REVERB_BUF_LEN 4800   /* 50ms. */
#define OVERSAMPLE     2      /* 2x oversampling. */
#define NUM_CHANNELS   2      /* Stereo. */
#define BUFFER_SAMPLES 16384  /* 64k per buffer. */
#define NUM_BUFFERS    4      /* 4 buffers (256k). */

static HANDLE semaphore;
static WAVEHDR wave_headers[ NUM_BUFFERS ];
static short reverb_buffer[ REVERB_BUF_LEN ];
static short mix_buffer[ BUFFER_SAMPLES * NUM_CHANNELS * OVERSAMPLE ];
static short wave_buffers[ NUM_BUFFERS ][ BUFFER_SAMPLES * NUM_CHANNELS ];
static long playing, reverb_len, reverb_idx, filt_l, filt_r;

/*
	2:1 downsampling with simple but effective anti-aliasing.
	Count is the number of stereo samples to process, and must be even.
	input may point to the same buffer as output.
*/
static void downsample( short *input, short *output, long count ) {
	long in_idx, out_idx, out_l, out_r;
	in_idx = out_idx = 0;
	while( out_idx < count ) {	
		out_l = filt_l + ( input[ in_idx++ ] >> 1 );
		out_r = filt_r + ( input[ in_idx++ ] >> 1 );
		filt_l = input[ in_idx++ ] >> 2;
		filt_r = input[ in_idx++ ] >> 2;
		output[ out_idx++ ] = out_l + filt_l;
		output[ out_idx++ ] = out_r + filt_r;
	}
}

/* Simple stereo cross delay with feedback. */
static void reverb( short *buffer, long count ) {
	long buffer_idx, buffer_end;
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

/* Reduce stereo-separation of count samples. */
static void crossfeed( short *audio, int count ) {
	int l, r, offset = 0, end = count << 1;
	while( offset < end ) {
		l = audio[ offset ];
		r = audio[ offset + 1 ];
		audio[ offset++ ] = ( l + l + l + r ) >> 2;
		audio[ offset++ ] = ( r + r + r + l ) >> 2;
	}
}

static void __stdcall wave_out_proc( HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 ) {
	/*if( uMsg == WOM_OPEN ) printf( "Device open.\n" );*/
	if( uMsg == WOM_DONE ) ReleaseSemaphore( semaphore, 1, NULL );
	/*if( uMsg == WOM_CLOSE ) printf( "Device closed.\n" );*/
}

static void termination_handler( int signum ) {
	/* Notify the main thread to stop playback. */
	playing = 0;
	fprintf( stderr, "\nTerminated!\n" );
}

static long read_file( char *filename, void *buffer, long length ) {
	FILE *file;
	long count;
	count = -1;
	file = fopen( filename, "rb" );
	if( file != NULL ) {
		count = fread( buffer, 1, length, file );
		if( count < length && !feof( file ) ) {
			fprintf( stderr, "Unable to read file '%s'.\n", filename );
			count = -1;
		}
		if( fclose( file ) != 0 ) {
			fprintf( stderr, "Unable to close file '%s'.\n", filename );
		}
	}
	return count;
}

static void print_module_info() {
	int inst;
	char string[ 23 ];
	for( inst = 0; inst < 16; inst++ ) {
		micromod_get_string( inst, string );
		printf( "%02i - %-22s ", inst, string );
		micromod_get_string( inst + 16, string );
		printf( "%02i - %-22s\n", inst + 16, string );
	}
}

static long read_module_length( char *filename ) {
	long length;
	signed char header[ 1084 ];
	length = read_file( filename, header, 1084 );
	if( length == 1084 ) {
		length = micromod_calculate_mod_file_len( header );
		if( length < 0 ) {
			fprintf( stderr, "Module file type not recognised.\n");
		}
	} else {
		fprintf( stderr, "Unable to read module file '%s'.\n", filename );
		length = -1;
	}
	return length;
}

static long check_mmresult( MMRESULT mmresult ) {
	long result;
	TCHAR string[ 64 ];
	result = 0;
	if( mmresult != MMSYSERR_NOERROR ) {
		waveOutGetErrorText( mmresult, string, 64 );
		fprintf( stderr, "%s\n", string );
		result = -1;
	}
	return result;
}

static long play_module( signed char *module ) {
	long result, idx, current_buffer, samples_remaining, count;
	WAVEFORMATEX wave_format;
	HWAVEOUT h_wave_out;
	WAVEHDR *header;
	MMRESULT mmresult;
	short *buffer;
	/* Initialise replay.*/
	result = micromod_initialise( module, SAMPLING_FREQ * OVERSAMPLE );
	if( result == 0 ) {
		print_module_info();
		/* Calculate song length. */
		samples_remaining = micromod_calculate_song_duration();
		printf( "Song Duration: %li seconds.\n", samples_remaining / ( SAMPLING_FREQ * OVERSAMPLE ) );
		fflush( NULL );
		/* Initialise Wave Format Structure. */
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nChannels = NUM_CHANNELS;
		wave_format.nSamplesPerSec = SAMPLING_FREQ;
		wave_format.nAvgBytesPerSec = SAMPLING_FREQ * NUM_CHANNELS * 2;
		wave_format.nBlockAlign = NUM_CHANNELS * 2;
		wave_format.wBitsPerSample = 16;
		/* Initialise Waveform Buffers. */
		for( idx = 0; idx < NUM_BUFFERS; idx++ ) {
			header = &wave_headers[ idx ];
			memset( header, 0, sizeof( WAVEHDR ) );
			header->lpData = ( LPSTR ) &wave_buffers[ idx ][ 0 ];
			header->dwBufferLength = BUFFER_SAMPLES * NUM_CHANNELS * sizeof( short );
		}
		/* Initialise Semaphore. */
		semaphore = CreateSemaphore( NULL, NUM_BUFFERS, NUM_BUFFERS, "" );
		/* Open Audio Device. */
		result = check_mmresult( waveOutOpen( &h_wave_out, WAVE_MAPPER, &wave_format, (DWORD_PTR) wave_out_proc, 0, CALLBACK_FUNCTION ) );
		if( result == 0 ) {
			/* Play through once. */
			playing = 1;
			current_buffer = 0;
			while( playing ) {
				/* Wait for a buffer to become available. */
				WaitForSingleObject( semaphore, INFINITE );
				header = &wave_headers[ current_buffer ];
				buffer = &wave_buffers[ current_buffer ][ 0 ];
				/* Clear mix buffer and get audio from replay. */
				count = BUFFER_SAMPLES * OVERSAMPLE;
				if( count > samples_remaining ) {
					count = samples_remaining;
				}
				memset( mix_buffer, 0, BUFFER_SAMPLES * NUM_CHANNELS * OVERSAMPLE * sizeof( short ) );
				micromod_get_audio( mix_buffer, count );
				downsample( mix_buffer, buffer, BUFFER_SAMPLES * OVERSAMPLE );
				crossfeed( buffer, BUFFER_SAMPLES );
				reverb( buffer, BUFFER_SAMPLES );
				samples_remaining -= count;
				/* Submit buffer to audio system. */
				result = check_mmresult( waveOutUnprepareHeader( h_wave_out, header, sizeof( WAVEHDR ) ) );
				if( result == 0 ) {
					result = check_mmresult( waveOutPrepareHeader( h_wave_out, header, sizeof( WAVEHDR ) ) );
					if( result == 0 ) {
						result = check_mmresult( waveOutWrite( h_wave_out, header, sizeof( WAVEHDR ) ) );
					}
				}
				if( samples_remaining <= 0 || result != 0 ) {
					playing = 0;
				}
				/* Next buffer. */
				current_buffer++;
				if( current_buffer >= NUM_BUFFERS ) {
					current_buffer = 0;
				}
			}
			if( samples_remaining > 0 ) {
				/* Playback interrupted, drop any currently playing buffers. */
				waveOutReset( h_wave_out );
			}
			while( waveOutClose( h_wave_out ) == WAVERR_STILLPLAYING ) {
				/* Keep trying until all buffers have played. */
				Sleep( 100 );
			}
		}
	} else {
		fprintf( stderr, "Unable to initialise replay.\n" );
	}
	return result;
}

int main( int argc, char **argv ) {
	int arg, result;
	long count, length;
	char *filename;
	signed char *module;
	filename = NULL;
	for( arg = 1; arg < argc; arg++ ) {
		/* Parse arguments.*/
		if( strcmp( argv[ arg ], "-reverb" ) == 0 ) {
			reverb_len = REVERB_BUF_LEN;
		} else {
			filename = argv[ arg ];
		}
	}
	result = EXIT_FAILURE;
	if( filename == NULL ) {
		fprintf( stderr, "Usage: %s [-reverb] filename\n", argv[ 0 ] );
	} else {
		/* Read module file.*/
		length = read_module_length( filename );
		if( length > 0 ) {
			printf( "Module Data Length: %li bytes.\n", length );
			module = calloc( length, 1 );
			if( module != NULL ) {
				count = read_file( filename, module, length );
				if( count < length ) {
					fprintf( stderr, "Module file is truncated. %li bytes missing.\n", length - count );
				}
				/* Install signal handlers.*/
				signal( SIGTERM, termination_handler );
				signal( SIGINT,  termination_handler );
				/* Play.*/
				if( play_module( module ) == 0 ) {
					result = EXIT_SUCCESS;
				}
				free( module );
			}
		}
	}
	return result;
}
