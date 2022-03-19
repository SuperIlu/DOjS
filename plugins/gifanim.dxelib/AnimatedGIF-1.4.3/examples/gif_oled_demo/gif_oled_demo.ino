//
// GIF mono OLED demo
//
#include <OneBitDisplay.h>
#include <AnimatedGIF.h>
#include "../test_images/pattern.h"

OBDISP obd;
AnimatedGIF gif;
static uint8_t ucOLED[1024]; // holds current frame for 128x64 OLED

// M5Atom Matrix ESP32
#define RESET_PIN -1
#define SDA_PIN 32
#define SCL_PIN 26
#define OLED_ADDR -1
#define MY_OLED OLED_128x64
#define USE_HW_I2C 1
#define FLIP180 0
#define INVERT 0

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

//
// This doesn't have to be super efficient
// 
void DrawPixel(int x, int y, uint8_t ucColor)
{
uint8_t ucMask;
int index;

  if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
     return;
  ucMask = 1 << (y & 7);
  index = x + ((y >> 3) << 7);
  if (ucColor)
     ucOLED[index] |= ucMask;
  else
     ucOLED[index] &= ~ucMask;
}

// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    int x, y, iWidth;
    static uint8_t ucPalette[256]; // thresholded palette

    if (pDraw->y == 0) // first line, convert palette to 0/1
    {
      for (x = 0; x < 256; x++)
      {
        uint16_t usColor = pDraw->pPalette[x];
        int gray = (usColor & 0xf800) >> 8; // red
        gray += ((usColor & 0x7e0) >> 2); // plus green*2
        gray += ((usColor & 0x1f) << 3); // plus blue
        ucPalette[x] = (gray >> 9); // 0->511 = 0, 512->1023 = 1
      }
    }
    y = pDraw->iY + pDraw->y; // current line
    iWidth = pDraw->iWidth;
    if (iWidth > DISPLAY_WIDTH)
       iWidth = DISPLAY_WIDTH;
       
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t c, ucTransparent = pDraw->ucTransparent;
      int x;
      for(x=0; x < iWidth; x++)
      {
        c = *s++;
        if (c != ucTransparent) 
             DrawPixel(pDraw->iX + x, y, ucPalette[c]);
      }
    }
    else
    {
      s = pDraw->pPixels;
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      for (x=0; x<pDraw->iWidth; x++)
        DrawPixel(pDraw->iX + x, y, ucPalette[*s++]);
    }
    if (pDraw->y == pDraw->iHeight-1) // last line, render it to the display
       obdDumpBuffer(&obd, ucOLED);
} /* GIFDraw() */

void setup() {
  int rc;
  rc = obdI2CInit(&obd, MY_OLED, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 800000L); // use standard I2C bus at 400Khz
  obdFill(&obd, 0,1);
  obdWriteString(&obd,0,0,0,(char *)"GIF Demo", FONT_NORMAL, 0, 1);
  gif.begin(LITTLE_ENDIAN_PIXELS);
}

void loop() {
  if (gif.open((uint8_t *)ucPattern, sizeof(ucPattern), GIFDraw))
  {
    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    while (gif.playFrame(true, NULL))
    {
    }
    gif.close();
  }
}
