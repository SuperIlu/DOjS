
#include "errno.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#include "micromod.h"

/*
	Protracker MOD to WAV/IFF-8SVX converter. (c)2018 mumart@gmail.com
*/

enum {
	RAW, WAV, IFF
};

static const short str_to_key[] = { -2, 0, 1, 3, 5, 6, 8 };
static const short key_to_period[] = { 1814, /*
	 C-0   C#0   D-0   D#0   E-0   F-0   F#0   G-0   G#0   A-1  A#1  B-1 */
	1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 907,
	 856,  808,  762,  720,  678,  640,  604,  570,  538,  508, 480, 453,
	 428,  404,  381,  360,  339,  320,  302,  285,  269,  254, 240, 226,
	 214,  202,  190,  180,  170,  160,  151,  143,  135,  127, 120, 113,
	 107,  101,   95,   90,   85,   80,   75,   71,   67,   63,  60,  56,
	  53,   50,   47,   45,   42,   40,   37,   35,   33,   31,  30,  28,
	  26
};

static short mix_buf[ 8192 ];
static int filt_l, filt_r, qerror, interrupted;

static int read_file( char *file_name, void *buffer, int limit ) {
	int file_length = -1, bytes_read;
	FILE *input_file = fopen( file_name, "rb" );
	if( input_file != NULL ) {
		if( fseek( input_file, 0L, SEEK_END ) == 0 ) {
			file_length = ftell( input_file );
			if( file_length >= 0 && buffer ) {
				if( fseek( input_file, 0L, SEEK_SET ) == 0 ) {
					if( limit > 0 && file_length > limit ) {
						file_length = limit;
					}
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
		fputs( strerror( errno ), stderr );
		fputs( "\n", stderr );
	}
	return file_length;
}

static void write_shortbe( int value, char *dest ) {
	dest[ 0 ] = ( value >> 8 ) & 0xFF;
	dest[ 1 ] = value & 0xFF;
}

static void write_int32be( int value, char *dest ) {
	dest[ 0 ] = ( value >> 24 ) & 0xFF;
	dest[ 1 ] = ( value >> 16 ) & 0xFF;
	dest[ 2 ] = ( value >> 8 ) & 0xFF;
	dest[ 3 ] = value & 0xFF;
}

static void write_int32le( int value, char *dest ) {
	dest[ 0 ] = value & 0xFF;
	dest[ 1 ] = ( value >> 8 ) & 0xFF;
	dest[ 2 ] = ( value >> 16 ) & 0xFF;
	dest[ 3 ] = ( value >> 24 ) & 0xFF;
}

/*
	2:1 downsampling with simple but effective anti-aliasing.
	Count is the number of stereo samples to process, and must be even.
	input may point to the same buffer as output.
*/
static void downsample( short *input, short *output, short count ) {
	short in_idx = 0, out_idx = 0, out_l, out_r;
	while( out_idx < count ) {
		out_l = filt_l + ( input[ in_idx++ ] >> 1 );
		out_r = filt_r + ( input[ in_idx++ ] >> 1 );
		filt_l = input[ in_idx++ ] >> 2;
		filt_r = input[ in_idx++ ] >> 2;
		output[ out_idx++ ] = out_l + filt_l;
		output[ out_idx++ ] = out_r + filt_r;
	}
}

/* Convert count stereo samples in input to 8-bit mono. */
static void quantize( short *input, char *output, int gain, short count ) {
	int ampl;
	short in_idx = 0, out_idx = 0;
	while( out_idx < count ) {
		/* Convert stereo to mono and apply gain. */
		ampl = input[ in_idx++ ];
		ampl = ( ampl + input[ in_idx++ ] ) * gain >> 8;
		/* Dithering. */
		ampl -= qerror;
		qerror = ampl;
		/* Rounding. */
		ampl += ampl & 0x80;
		/* Divide by 256. */
		if( ampl < 0 ) {
			ampl = ( ampl + 255 ) >> 8;
		} else{
			ampl = ampl >> 8;
		}
		qerror = ( ampl << 8 ) - qerror;
		/* Clipping. */
		if( ampl < -128 ) ampl = -128;
		if( ampl > 127 ) ampl = 127;
		output[ out_idx++ ] = ampl;
	}
}

/* Reduce stereo-separation of count samples. */
static void crossfeed( short *audio, short count ) {
	int l, r;
	short offset = 0, end = count << 1;
	while( offset < end ) {
		l = audio[ offset ];
		r = audio[ offset + 1 ];
		audio[ offset++ ] = ( l + l + l + r ) >> 2;
		audio[ offset++ ] = ( r + r + r + l ) >> 2;
	}
}

/* Allocate and return a copy of source. */
static char* new_string( char *source ) {
	char *dest = malloc( sizeof( char ) * ( strlen( source ) + 1 ) );
	if( dest ) {
		strcpy( dest, source );
	}
	return dest;
}

/* Allocate and return a file-name with a 3-digit serial number added before the extension. */
static char* series_name( char *file_name, int idx ) {
	int len = strlen( file_name );
	char *base, *ext, *name = NULL;
	base = malloc( len + 1 );
	if( base ) {
		strcpy( base, file_name );
		ext = strrchr( base, '.' );
		if( ext ) {
			ext[ 0 ] = 0;
			ext = &ext[ 1 ];
		}
		name = malloc( len + 4 );
		if( name ) {
			if( ext ) {
				sprintf( name, "%s%.3d.%s", base, idx, ext );
			} else {
				sprintf( name, "%s%.3d", base, idx );
			}
		}
		free( base );
	}
	return name;
}

static int wav_header( char *out_buf, int sample_rate, int num_samples ) {
	strcpy( out_buf, "RIFF" );
	write_int32le( num_samples * 4 + 36, &out_buf[ 4 ] );
	strcpy( &out_buf[ 8 ], "WAVEfmt " );
	write_int32le( 16, &out_buf[ 16 ] );
	write_int32le( 0x00020001, &out_buf[ 20 ] );
	write_int32le( sample_rate, &out_buf[ 24 ] );
	write_int32le( sample_rate * 4, &out_buf[ 28 ] );
	write_int32le( 0x00100004, &out_buf[ 32 ] );
	strcpy( &out_buf[ 36 ], "data" );
	write_int32le( num_samples * 4, &out_buf[ 40 ] );
	return 44;
}

static int iff_header( char *out_buf, int sample_rate, int num_samples ) {
	strcpy( out_buf, "FORM" );
	write_int32be( num_samples + 40, &out_buf[ 4 ] );
	strcpy( &out_buf[ 8 ], "8SVXVHDR" );
	write_int32be( 20, &out_buf[ 16 ] );
	write_int32be( num_samples, &out_buf[ 20 ] );
	write_int32be( 0, &out_buf[ 24 ] );
	write_int32be( 32, &out_buf[ 28 ] );
	write_shortbe( sample_rate, &out_buf[ 32 ] );
	out_buf[ 34 ] = 1;
	out_buf[ 35 ] = 0;
	write_int32be( 65536, &out_buf[ 36 ] );
	strcpy( &out_buf[ 40 ], "BODY" );
	write_int32be( num_samples, &out_buf[ 44 ] );
	return 48;
}

static int get_audio_s16le( char *out_buf, int num_samples ) {
	int offset = 0, length = num_samples << 2;
	short idx, end, count, ampl;
	while( offset < length && !interrupted ) {
		count = 8192;
		if( count > length - offset ) {
			count = length - offset;
		}
		memset( mix_buf, 0, count * sizeof( short ) );
		micromod_get_audio( mix_buf, count >> 1 );
		downsample( mix_buf, mix_buf, count >> 1 );
		crossfeed( mix_buf, count >> 2 );
		idx = 0;
		end = count >> 1;
		while( idx < end ) {
			ampl = mix_buf[ idx++ ];
			out_buf[ offset++ ] = ampl;
			out_buf[ offset++ ] = ampl >> 8;
		}
	}
	return length;
}

static int get_audio_s8( char *out_buf, int length, int gain ) {
	int count, offset = 0;
	while( offset < length && !interrupted ) {
		count = 2048;
		if( count > length - offset ) {
			count = length - offset;
		}
		memset( mix_buf, 0, ( count << 2 ) * sizeof( short ) );
		micromod_get_audio( mix_buf, count << 1 );
		downsample( mix_buf, mix_buf, count << 1 );
		quantize( mix_buf, &out_buf[ offset ], gain, count );
		offset += count;
	}
	return length;
}

static int write_sam( char *file_name, int sample_rate, int num_samples, int gain, int type ) {
	int bytes, percent, offset = 0, count = 0, result = 0;
	FILE *out_file = fopen( file_name, "wb" );
	time_t seconds = time( NULL );
	if( out_file ) {
		char *out_buf = malloc( 65536 );
		if( out_buf ) {
			if( type == WAV ) {
				count = wav_header( out_buf, sample_rate, num_samples );
			} else if( type == IFF ) {
				count = iff_header( out_buf, sample_rate, num_samples );
			}
			if( count == 0 || fwrite( out_buf, 1, count, out_file ) == count ) {
				bytes = count + ( type == WAV ? num_samples * 4 : num_samples );
				printf( "Generating \"%s\", file size %d bytes.\n", file_name, bytes );
				percent = num_samples > 100 ? num_samples / 100 : 1;
				while( offset < num_samples && !interrupted ) {
					count = num_samples - offset;
					if( count > 16384 ) {
						count = 16384;
					}
					if( type == WAV ) {
						bytes = get_audio_s16le( out_buf, count );
					} else {
						bytes = get_audio_s8( out_buf, count, gain );
					}
					if( !interrupted ) {
						if( fwrite( out_buf, 1, bytes, out_file ) == bytes ) {
							offset += count;
							if( !interrupted ) {
								printf( "\rProgress: %d%%", offset / percent );
								fflush( stdout );
							}
						} else {
							fputs( strerror( errno ), stderr );
							fputs( "\n", stderr );
							interrupted = 1;
							result = errno;
						}
					}
				}
				if( !interrupted ) {
					printf( "\rCompleted in %d seconds!\n", ( int ) ( time( NULL ) - seconds ) );
				}
			} else {
				fputs( strerror( errno ), stderr );
				fputs( "\n", stderr );
				result = errno;
			}
			free( out_buf );
		} else {
			fputs( "Not enough memory for output buffer.\n", stderr );
			result = 1;
		}
		fclose( out_file );
	} else {
		fputs( strerror( errno ), stderr );
		fputs( "\n", stderr );
		result = errno;
	}
	return result;
}

static int mod_to_wav( signed char *module_data, char *file_name, int sample_rate, int gain, int chan, int type, int num_files ) {
	char *out_name;
	int result, num_channels, idx, duration, count, offset = 0, file = 0;
	if( type == WAV ) {
		printf( "Generating %d 16-bit stereo RIFF-WAV files.\n", num_files );
	} else {
		printf( "Generating %d 8-bit mono %s files.\n", num_files, type == IFF ? "IFF-8SVX" : "RAW" );
	}
	result = micromod_initialise( module_data, sample_rate * 2 );
	if( result == 0 ) {
		num_channels = micromod_mute_channel( -1 );
		if( chan >= 0 && chan < num_channels ) {
			printf( "Muting all but channel %d.\n", chan );
			for( idx = 0; idx < num_channels; idx++ ) {
				if( idx != chan ) {
					micromod_mute_channel( idx );
				}
			}
		}
		duration = micromod_calculate_song_duration() >> 1;
		count = num_files > 0 ? duration / num_files : duration;
		while( offset < duration && result == 0 && !interrupted ) {
			if( file >= num_files - 1 ) {
				count = duration - offset;
			}
			if( num_files > 1 ) {
				out_name = series_name( file_name, file++ );
			} else {
				out_name = new_string( file_name );
			}
			if( out_name ) {
				result = write_sam( out_name, sample_rate, count, gain, type );
				free( out_name );
			} else {
				fputs( "Out of memory.", stderr );
				result = 1;
			}
			offset += count;
		}
	} else {
		fputs( "Unsupported module or invalid sampling rate.\n", stderr );
	}
	return result;
}

static signed char* load_module( char *file_name ) {
	int length;
	signed char header[ 1084 ], *module = NULL;
	if( read_file( file_name, header, 1084 ) == 1084 ) {
		length = micromod_calculate_mod_file_len( header );
		if( length > 0 ) {
			printf( "Module Data Length: %d bytes.\n", length );
			module = calloc( length, sizeof( signed char ) );
			if( module ) {
				if( read_file( file_name, module, length ) < 0 ) {
					free( module );
					module = NULL;
				}
			} else {
				fputs( "Not enough memory to load module.\n", stderr );
			}
		} else {
			fputs( "Unrecognized module type.\n", stderr );
		}
	}
	return module;
}

static int set_pattern( signed char *module_data, int pattern ) {
	int idx, pat, max = 0;
	for( idx = 0; idx < 128; idx++ ) {
		pat = module_data[ 952 + idx ] & 0x7F;
		if( pat > max ) {
			max = pat;
		}
	}
	if( pattern < 0 || pattern > max ) {
		pattern = 0;
	}
	module_data[ 950 ] = 1;
	module_data[ 952 ] = pattern;
	module_data[ 953 ] = max;
	return pattern;
}

static int key_to_freq( int key, int c2rate ) {
	int freq = 0;
	if( key > 0 && key < 73 ) {
		freq = c2rate * 428 / key_to_period[ key ];
	}
	return freq;
}

static int str_to_freq( char *str, int c2rate ) {
	int key, freq = 0;
	int chr = str[ 0 ] >= 'a' ? str[ 0 ] - 32 : str[ 0 ];
	if( chr >= 'A' && chr <= 'G' && strlen( str ) == 3 ) {
		key = str_to_key[ chr - 'A' ];
		if( str[ 1 ] == '#' ) {
			key++;
		}
		if( "A-A#B-C-C#D-D#E-F-F#G-G#"[ key * 2 + 5 ] == str[ 1 ] ) {
			if( str[ 2 ] >= '0' && str[ 2 ] <= '9' ) {
				key += ( str[ 2 ] - '0' ) * 12;
				freq = key_to_freq( key, c2rate );
			}
		}
	} else {
		freq = atoi( str );
	}
	return freq;
}

void uppercase( char *str ) {
	int chr = str[ 0 ], idx = 0;
	while( chr ) {
		if( chr >= 'a' && chr <= 'z' ) {
			str[ idx ] = chr - 32;
		}
		chr = str[ ++idx ];
	}
}

int filetype( char *name ) {
	char ext[ 5 ];
	int type = WAV, len = strlen( name );
	if( len > 3 ) {
		strcpy( ext, &name[ len - 4 ] );
		uppercase( ext );
		if( strcmp( ext, ".IFF" ) == 0 ) type = IFF;
		if( strcmp( ext, ".RAW" ) == 0 ) type = RAW;
	}
	return type;
}

static void termination_handler( int signum ) {
	signal( signum, termination_handler );
	fprintf( stderr, "\nInterrupted...\n" );
	interrupted = 1;
}

int main( int argc, char **argv ) {
	int result = EXIT_FAILURE, idx = 1;
	int type, patt = -1, rate = -1, gain = -1, chan = -1, files = 1;
	char *arg, *in_file = NULL, *out_file = NULL;
	signed char *module;
	puts( micromod_get_version() );
	while( idx < argc ) {
		arg = argv[ idx++ ];
		if( idx < argc && strcmp( "-pat", arg ) == 0 ) {
			patt = atoi( argv[ idx++ ] );
		} else if( idx < argc && strcmp( "-rate", arg ) == 0 ) {
			rate = str_to_freq( argv[ idx++ ], 8287 );
		} else if( idx < argc && strcmp( "-gain", arg ) == 0 ) {
			gain = atoi( argv[ idx++ ] );
		} else if( idx < argc && strcmp( "-split", arg ) == 0 ) {
			files = atoi( argv[ idx++ ] );
		} else if( idx < argc && strcmp( "-chan", arg ) == 0 ) {
			chan = atoi( argv[ idx++ ] );
		} else if( !in_file ) {
			in_file = arg;
		} else if( !out_file ) {
			out_file = arg;
		}
	}
	if( out_file ) {
		printf( "Converting \"%s\".\n", in_file );
		/* Install signal handlers.*/
		signal( SIGTERM, termination_handler );
		signal( SIGINT,  termination_handler );
		/* Get output file type. */
		type = filetype( out_file );
		/* Read module file.*/
		module = load_module( in_file );
		if( module ) {
			/* Configure parameters.*/
			if( rate < 0 ) {
				rate = ( type == WAV ) ? 48000 : str_to_freq( "A-4", 8287 );
			}
			if( gain < 1 ) {
				gain = 128;
			}
			if( files < 1 ) {
				files = 1;
			}
			if( patt >= 0 ) {
				printf( "Converting pattern %d, sample rate %dhz, gain %d.\n", patt, rate, gain );
				set_pattern( module, patt );
			} else {
				printf( "Converting whole song, sample rate %dhz, gain %d.\n", rate, gain );
			}
			/* Perform conversion.*/
			if( !interrupted ) {
				result = mod_to_wav( module, out_file, rate, gain, chan, type, files );
			}
			free( module );
		}
	} else {
		fprintf( stderr, "Usage: %s in.mod out [-pat p] [-rate r] [-gain g] [-split n] [-chan n]\n\n", argv[ 0 ] );
		fprintf( stderr, "   If output ends with \".wav\", generate 16-bit stereo RIFF-WAV file.\n" );
		fprintf( stderr, "   If output ends with \".iff\", generate 8-bit mono IFF-8SVX file.\n" );
		fprintf( stderr, "   If output ends with \".raw\", generate 8-bit mono signed raw samples.\n" );
		fprintf( stderr, "   If pattern is unspecified, convert the whole song.\n" );
		fprintf( stderr, "   Rate can be specified in HZ or as a key such as \"C-2\".\n" );
		fprintf( stderr, "   Gain works only for IFF/RAW output and defaults to 128.\n" );
		fprintf( stderr, "   Chan mutes all but the specified channel, starting from 0.\n" );
		fprintf( stderr, "   Split divides the output into the specified number of files.\n\n" );
		fprintf( stderr, "Whole song to wav: %s input.mod output.wav -rate 48000\n", argv[ 0 ] );
		fprintf( stderr, "Pattern to sample: %s input.mod output.iff -pat 0 -rate A-4 -gain 128\n", argv[ 0 ] );
	}
	return result;
}
