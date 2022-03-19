/*
AllegroPNG

Copyright (c) 2006 Michal Molhanec

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented;
     you must not claim that you wrote the original software.
     If you use this software in a product, an acknowledgment
     in the product documentation would be appreciated but
     is not required.

  2. Altered source versions must be plainly marked as such,
     and must not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any
     source distribution.
*/

/*

ex01.c -- Example of using AllegroPNG library.

*/

#include <allegro.h>
#include <alpng.h>

void init();
void deinit();

char* usage = "Usage:\n"
              "   ex01 image [color-depth [width height]]\n"
              "\n"
              "Where:\n"
              "   image is filename of image in PNG, BMP, LBM, PCX or TGA format\n"
              "   color-depth is one of 8, 15, 16, 24, 32 or letter a for 32 bits\n"
              "               with alpha\n"
              "   width and height are window dimensions, defaults to 800x600\n";

int FG, BG;

DIALOG dlg[] = {
    /* dlg_proc             x   y   w   h   fg  bg  key flags   d1  d2  dp  dp2 dp3 */
  {  d_clear_proc,          0,  0,  0,  0,  0,  0, 0,  0,      0,  0,  0,  0,  0},
  {  d_textbox_proc,        10, 10, 780,550,0,  0, 0,  0,      0,  0,  0,  0,  0},
  {  d_button_proc,         700,570,90, 20, 0,  0, 0,  D_EXIT, 0,  0,  "Exit", 0, 0},
  {  0,                     0,  0,  0,  0,  0,  0, 0,  0,      0,  0,  0,  0,  0}
};

int main(int argc, char* argv[]) {
    BITMAP* b = 0;
    PALETTE pal;
    int bpp = 0;
    int w = 800;
    int h = 600;
    int alpha = 0;
    char* param;
    DATAFILE* datafile = 0;

    allegro_init();
    alpng_init();

    if (argc < 2) {
        allegro_message(usage);
        allegro_message("Press S during viewing to save the picture to pok.png");
        return 1;
    }

    if (argc > 2) {
        param = argv[2];
        if (param[0] == 'a' || param[0] == 'A') {
            bpp = 32;
            alpha = 1;
        } else {
            bpp = atoi(param);
            if (bpp != 8 && bpp != 15 && bpp != 16 && bpp != 24 && bpp != 32) {
                allegro_message("Invalid color depth, reverting to default.");
                bpp = 0;
            }
        }
    }

    if (argc > 4) {
        w = atoi(argv[3]);
        h = atoi(argv[4]);
    }

    init(bpp, w, h);
    
    if (ustrcmp(get_extension(argv[1]), "dat") == 0) {
        datafile = load_datafile(argv[1]);
        if (!datafile) {
            allegro_message("Cannot load datafile!");
            return 0;
        }
        b = (BITMAP*) datafile[0].dat;
    } else {
        b = load_bitmap(argv[1], pal);
    }

    if (b) {
        if (alpha) {
            clear_to_color(screen, FG);
            set_alpha_blender();
            draw_trans_sprite(screen, b, 0, 0);
        } else {
            set_palette(pal);
            blit(b, screen, 0, 0, 0, 0, b->w, b->h);
        }
        while (!keypressed());
        if (key[KEY_S]) {
            if (save_bitmap("pok.png", b, pal) != 0) {
                clear(screen);
                dlg[1].dp = alpng_error_msg;
                do_dialog(dlg, 1);
            }
        }
        if (datafile) {
            unload_datafile(datafile);
        } else {
            destroy_bitmap(b);
        }
    } else {
        dlg[1].dp = alpng_error_msg;
        do_dialog(dlg, 1);
    }

    deinit();
    return 0;
}
END_OF_MAIN()

void init(int bpp, int w, int h) {
    unsigned int i;
    int res;
    if (!bpp) {
        bpp = desktop_color_depth();
        if (bpp == 0) {
            bpp = 32;
        }
    }
    set_color_depth(bpp);
    res = set_gfx_mode(GFX_AUTODETECT_WINDOWED, w, h, 0, 0);
    if (res != 0) {
        res = set_gfx_mode(GFX_AUTODETECT, w, h, 0, 0);
        if (res != 0) {
            allegro_message(allegro_error);
            exit(-1);
        }
    }

    install_timer();
    install_keyboard();
    install_mouse();
    /* add other initializations here */
    FG = makecol(255, 255, 255);
    BG = makecol(0, 0, 0);
    for (i=0; i<sizeof(dlg)/sizeof(DIALOG) - 1; i++) {
        dlg[i].fg = FG;
        dlg[i].bg = BG;
    }
}

void deinit() {
    clear_keybuf();
    /* add other deinitializations here */
}
