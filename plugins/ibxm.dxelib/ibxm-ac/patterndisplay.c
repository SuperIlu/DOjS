
#include "SDL.h"

#include "ibxm.h"

/* Maximum pattern display dimensions. */
static const int MAX_WIDTH = ( 16 * 11 + 4 ) * 8, MAX_HEIGHT = 18 * 16;

/* 95 8x16 ASCII characters, 4bpp. */
extern const int CHAR_SET[];

static const int PAL_ENTRIES[] = { /*
	Blue      Green     Cyan      Red       Magenta   Yellow    White     Lime */
	0x0000C0, 0x008000, 0x008080, 0x800000, 0x800080, 0x806600, 0x808080, 0x668000,
	0x0066FF, 0x00FF00, 0x00FFFF, 0xFF0000, 0xFF00FF, 0xFFCC00, 0xFFFFFF, 0xCCFF00
};

static const char *KEY_TO_STR = "A-A#B-C-C#D-D#E-F-F#G-G#";
static const char *B36_TO_STR = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* Colours to use for effect commands (MOD/XM upper case, S3M lower case). */
static const char FX_CLR[] = {
	/* 0 1 2 3 4 5 6 7 8 9 : ; < = > ? */
	   1,1,1,1,1,7,7,5,5,4,0,0,0,0,0,0,
	/* @ A B C D E F G H I J K L M N O */
	   0,5,6,5,6,0,6,5,5,0,0,0,4,0,0,0,
	/* P Q R S T U V W X Y Z [ \ ] ^ _ */
	   5,0,4,0,5,0,0,0,1,0,0,0,0,0,0,0,
	/* ` a b c d e f g h i j k l m n o */
	   0,6,6,6,5,1,1,1,1,5,1,7,7,0,0,4,
	/* p q r s t u v w x y z { | } ~ */
	   0,4,5,0,6,1,5,0,0,0,0,0,0,0,0
};

/* Colours to use for Exy effects. */
static const char EX_CLR[] = {
	/* 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F */
	   0,1,1,1,1,1,6,5,0,4,0,0,0,0,0,0,0,5,5,4,4,4,4
};

/* Colours to use for Sxy effects.*/
static const char SX_CLR[] = {
	/* 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F */
	   5,4,1,1,5,0,0,0,5,0,0,0,0,0,0,0,0,5,4,4,4,4,4
};

/* Colours to use for volume-column effects. */
static const char VC_CLR[] = {
	/* 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F */
	   5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,1,1,5,5,5,1
};

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *target, *charset;
static int scroll, mute;

static SDL_Texture* create_texture( int width, int height, Uint32 *pixels ) {
	struct SDL_Texture *texture = SDL_CreateTexture( renderer,
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, width, height );
	if( texture ) {
		SDL_SetTextureBlendMode( texture, SDL_BLENDMODE_BLEND );
		if( SDL_UpdateTexture( texture, NULL, pixels, width * sizeof( Uint32 ) ) ) {
			fprintf( stderr, "Unable to update texture: %s\n", SDL_GetError() );
		}
	} else {
		fprintf( stderr, "Unable to create texture: %s\n", SDL_GetError() );
	}
	return texture;
}

static SDL_Texture* create_charset( const int *source, const int *pal_entries, int pal_size ) {
	int w, h, clr, chr, y, x;
	int src_off, pix_off;
	Uint32 *pixels;
	SDL_Texture *texture = NULL;
	w = 8 * 96;
	h = 16 * pal_size;
	pixels = calloc( w * h, sizeof( Uint32 ) );
	if( pixels ) {
		for( clr = 0; clr < pal_size; clr++ ) {
			src_off = 0;
			for( chr = 1; chr < 95; chr++ ) {
				pix_off = clr * w * 16 + chr * 8;
				for( y = 0; y < 16; y++ ) {
					for( x = 7; x >= 0; x-- ) {
						if( ( source[ src_off ] >> x * 4 ) & 0xF ) {
							pixels[ pix_off + x ] = ( pal_entries[ clr ] << 8 ) | 0xFF;
						}
					}
					src_off++;
					pix_off += w;
				}
			}
		}
		texture = create_texture( w, h, pixels );
		free( pixels );
	} else {
		fputs( "Out of memory.", stderr );
	}
	return texture;
}

static void fill_rect( int x, int y, int w, int h, int c ) {
	struct SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_RenderSetClipRect( renderer, NULL );
	SDL_SetRenderDrawColor( renderer, ( c >> 16 ) & 0xFF, ( c >> 8 ) & 0xFF, c & 0xFF, 0xFF );
	SDL_RenderFillRect( renderer, &rect );
}

static int blit_texture( SDL_Texture *texture, int sx, int sy, int sw, int sh, int dx, int dy ) {
	struct SDL_Rect clip, dest;
	clip.x = dx;
	clip.y = dy;
	clip.w = sw;
	clip.h = sh;
	dest.x = dx - sx;
	dest.y = dy - sy;
	SDL_QueryTexture( texture, NULL, NULL, &dest.w, &dest.h );
	SDL_RenderSetClipRect( renderer, &clip );
	return SDL_RenderCopy( renderer, texture, NULL, &dest );
}

static void draw_char( char chr, int row, int col, int clr ) {
	blit_texture( charset, ( chr - 32 ) * 8, clr * 16, 8, 16, col * 8, row * 16 );
}

static void draw_text( char *text, int len, int row, int col, int clr ) {
	int idx = 0, chr = text[ 0 ];
	while( chr ) {
		draw_char( chr, row, col + idx, clr );
		chr = text[ ++idx ];
		if( len && idx >= len ) {
			chr = 0;
		}
	}
}

static void draw_int( int val, int len, int row, int col, int clr ) {
	while( len > 0 ) {
		len = len - 1;
		draw_char( 48 + val % 10, row, col + len, clr );
		val = val / 10;
	}
}

static void get_note( struct pattern *pat, int row, int chn, char *note ) {
	int offset = ( pat->num_channels * row + chn ) * 5;
	int key = pat->data[ offset ] & 0xFF;
	int instrument = pat->data[ offset + 1 ] & 0xFF;
	int volume = pat->data[ offset + 2 ] & 0xFF;
	int effect = pat->data[ offset + 3 ] & 0xFF;
	int param = pat->data[ offset + 4 ] & 0xFF;
	note[ 0 ] = ( key > 0 && key < 118 ) ? KEY_TO_STR[ ( ( key + 2 ) % 12 ) * 2 ] : 45;
	note[ 1 ] = ( key > 0 && key < 118 ) ? KEY_TO_STR[ ( ( key + 2 ) % 12 ) * 2 + 1 ] : 45;
	note[ 2 ] = ( key > 0 && key < 118 ) ? 48 + ( key + 2 ) / 12 : 45;
	note[ 3 ] = ( instrument > 0xF && instrument < 0xFF ) ? B36_TO_STR[ ( instrument >> 4 ) & 0xF ] : 45;
	note[ 4 ] = ( instrument > 0x0 && instrument < 0xFF ) ? B36_TO_STR[ instrument & 0xF ] : 45;
	note[ 5 ] = ( volume > 0xF && volume < 0xFF ) ? B36_TO_STR[ ( volume >> 4 ) & 0xF ] : 45;
	note[ 6 ] = ( volume > 0x0 && volume < 0xFF ) ? B36_TO_STR[ volume & 0xF ] : 45;
	if( ( effect > 0 || param > 0 ) && effect < 36 ) {
		note[ 7 ] = B36_TO_STR[ effect ];
	} else if( effect > 0x80 && effect < 0x9F ) {
		note[ 7 ] = 96 + ( effect & 0x1F );
	} else {
		note[ 7 ] = 45;
	}
	note[ 8 ] = ( effect > 0 || param > 0 ) ? B36_TO_STR[ ( param >> 4 ) & 0xF ] : 45;
	note[ 9 ] = ( effect > 0 || param > 0 ) ? B36_TO_STR[ param & 0xF ] : 45;
}

static void draw_pattern( struct module *mod, int pat, int row, int channel, int width, int mute ) {
	int c, c1, y, r, x, clr, bclr, scroll_x, scroll_w;
	char note[ 10 ];
	int num_chan = mod->patterns[ pat ].num_channels;
	int num_rows = mod->patterns[ pat ].num_rows;
	int num_cols = ( width - 4 * 8 ) / ( 11 * 8 );
	if( num_cols > num_chan - channel ) {
		num_cols = num_chan - channel;
	}
	fill_rect( 0, 0, MAX_WIDTH, MAX_HEIGHT, 0 );
	draw_text( mod->name, 32, 0, 4, 0 );
	draw_int( pat, 3, 1, 0, 3 );
	for( c = 0; c < num_cols; c++ ) {
		if( mute & ( 1 << ( c + channel ) ) ) {
			draw_text( " Muted  ", 8, 1, c * 11 + 4, 3 );
			draw_int( c + channel, 2, 1, c * 11 + 12, 3 );
		} else {
			draw_text( "Channel ", 8, 1, c * 11 + 4, 0 );
			draw_int( c + channel, 2, 1, c * 11 + 12, 0 );
		}
	}
	for( y = 2; y < 17; y++ ) {
		r = row - 9 + y;
		if( r >= 0 && r < num_rows ) {
			bclr = ( y == 9 ) ? 8 : 0;
			draw_int( r, 3, y, 0, bclr );
			for( c = 0; c < num_cols; c++ ) {
				x = 4 + c * 11;
				get_note( &mod->patterns[ pat ], r, c + channel, note );
				if( mute & ( 1 << ( c + channel ) ) ) {
					draw_text( note, 10, y, x, bclr );
				} else {
					draw_text( note, 3, y, x, note[ 0 ] == 45 ? bclr : bclr + 2 );
					draw_char( note[ 3 ], y, x + 3, note[ 3 ] == 45 ? bclr : bclr + 3 );
					draw_char( note[ 4 ], y, x + 4, note[ 4 ] == 45 ? bclr : bclr + 3 );
					clr = bclr;
					if( note[ 5 ] >= 48 && note[ 5 ] <= 70 ) {
						clr = bclr + VC_CLR[ note[ 5 ] - 48 ];
					}
					draw_char( note[ 5 ], y, x + 5, clr );
					draw_char( note[ 6 ], y, x + 6, clr );
					clr = bclr;
					if( note[ 7 ] == 69 && note[ 8 ] >= 48 && note[ 8 ] <= 70 ) {
						clr = clr + EX_CLR[ note[ 8 ] - 48 ];
					} else if( note[ 7 ] == 115 && note[ 8 ] >= 48 && note[ 8 ] <= 70 ) {
						clr = clr + SX_CLR[ note[ 8 ] - 48 ];
					} else if( note[ 7 ] >= 48 && note[ 7 ] <= 126 ) {
						clr = clr + FX_CLR[ note[ 7 ] - 48 ];
					}
					if( note[ 7 ] >= 97 ) {
						note[ 7 ] -= 32;
					}
					draw_text( &note[ 7 ], 3, y, x + 7, clr );
				}
			}
		}
	}
	scroll_x = 4 + num_cols * 11 * channel / num_chan;
	scroll_w = num_cols * 11 * num_cols / num_chan;
	draw_char( 91, 17, scroll_x - 1, 0 );
	for( c = scroll_x, c1 = scroll_x + scroll_w - 1; c < c1; c++ ) {
		draw_char( 61, 17, c, 0 );
	}
	draw_char( 93, 17, scroll_x + scroll_w - 1, 0 );
}

static int scroll_click( struct module *mod, int x, int channel, int width ) {
	int num_chan = mod->num_channels;
	int num_cols = ( width - 4 * 8 ) / ( 11 * 8 );
	if( num_cols > num_chan - channel ) {
		if( channel > 0 ) {
			channel = num_chan - num_cols;
		}
		num_cols = num_chan - channel;
	}
	if( x > ( 4 + num_cols * 11 * channel / num_chan ) * 8 ) {
		channel += 4;
		if( channel > mod->num_channels - num_cols ) {
			channel = mod->num_channels - num_cols;
		}
	} else if( x > 0 ) {
		channel -= 4;
		if( channel < 0 ) {
			channel = 0;
		}
	}
	return channel;
}

static void redraw_display() {
	SDL_Rect rect = { 0 };
	if( renderer && target ) {
		SDL_SetRenderTarget( renderer, NULL );
		SDL_GetWindowSize( window, &rect.w, &rect.h );
		fill_rect( 0, 0, rect.w, rect.h, 0 );
		SDL_QueryTexture( target, NULL, NULL, &rect.w, &rect.h );
		SDL_RenderCopy( renderer, target, NULL, &rect );
		SDL_RenderPresent( renderer );
		SDL_SetRenderTarget( renderer, target );
	}
}

static int get_width( SDL_Window *window ) {
	int width, height;
	SDL_GetWindowSize( window, &width, &height );
	if( width > MAX_WIDTH ) {
		width = MAX_WIDTH;
	}
	return width;
}

void pattern_display_close() {
	if( charset ) {
		SDL_DestroyTexture( charset );
		charset = NULL;
	}
	if( target ) {
		SDL_DestroyTexture( target );
		target = NULL;
	}
	if( renderer ) {
		SDL_DestroyRenderer( renderer );
		renderer = NULL;
	}
	if( window ) {
		SDL_DestroyWindow( window );
		window = NULL;
	}
}

int pattern_display_open() {
	int result = 0;
	window = SDL_CreateWindow( IBXM_VERSION,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		( 11 * 8 + 4 ) * 8, MAX_HEIGHT, SDL_WINDOW_RESIZABLE );
	if( window ) {
		SDL_SetWindowMinimumSize( window, ( 11 * 4 + 4 ) * 8, MAX_HEIGHT );
		SDL_SetWindowMaximumSize( window, MAX_WIDTH, MAX_HEIGHT );
		renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_TARGETTEXTURE );
		if( renderer ) {
			target = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAX_WIDTH, MAX_HEIGHT );
			if( target ) { 
				SDL_SetRenderTarget( renderer, target );
				charset = create_charset( CHAR_SET, PAL_ENTRIES, 16 );
				if( charset ) {
					result = 1;
				}
			}
		}
	}
	if( !result ) {
		pattern_display_close();
	}
	return result;
}

int pattern_display_get_mute() {
	return mute;
}

void pattern_display_redraw_event( SDL_Event *event, struct module *module ) {
	int width;
	if( window ) {
		width = get_width( window );
		/*printf( "%03d %03d\n", ( int ) event->user.code >> 8, ( int ) event->user.code & 0xFF );*/
		scroll = scroll_click( module, -1, scroll, width );
		draw_pattern( module, module->sequence[ event->user.code >> 8 ], event->user.code & 0xFF, scroll, width, mute );
		redraw_display();
	}
}

void pattern_display_button_event( SDL_Event *event, struct module *module ) {
	int chan, width;
	if( window ) {
		width = get_width( window );
		if( event->button.y > 256 ) {
			scroll = scroll_click( module, event->button.x, scroll, width );
		} else {
			chan = ( event->button.x - 4 * 8 ) / ( 11 * 8 );
			if( event->button.x < 4 * 8 || chan >= module->num_channels ) {
				mute = 0;
			} else {
				chan = 1 << ( scroll + chan );
				if( mute == ~chan ) {
					/* Solo channel, unmute all. */
					mute = 0;
				} else if( mute & chan ) {
					/* Muted channel, unmute. */
					mute ^= chan;
				} else {
					/* Unmuted channel, set as solo. */
					mute = -1 ^ chan;
				}
			}
		}
	}
}
