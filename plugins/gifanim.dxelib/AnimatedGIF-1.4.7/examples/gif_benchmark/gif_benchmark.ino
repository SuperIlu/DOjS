//
// Embedded CPU benchmark test
// tests a real-world job (GIF animation)
// on various CPU types and speeds
//

#include <AnimatedGIF.h>
#include "../test_images/badgers.h"

AnimatedGIF gif;

// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
 // don't do anything here because we're measuring CPU work
 // not display update time
} /* GIFDraw() */

void setup() {
  Serial.begin(115200);
  while (!Serial);

  gif.begin(BIG_ENDIAN_PIXELS);
}

void loop() {
long lTime;
int iFrames = 0;

  Serial.println("GIF CPU speed benchmark");
  if (gif.open((uint8_t *)ucBadgers, sizeof(ucBadgers), GIFDraw))
  {
    Serial.println("Successfully opened GIF, starting test...");
    lTime = micros();
    while (gif.playFrame(false, NULL))
    {
      iFrames++;
    }
    gif.close();
    lTime = micros() - lTime;
    Serial.print("Decoded ");
    Serial.print(iFrames, DEC);
    Serial.print(" frames in ");
    Serial.print((int)lTime, DEC);
    Serial.println(" microseconds");
  }
  delay(10000);
}
