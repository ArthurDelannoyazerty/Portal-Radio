/*
 * Rui Santos 
 * Complete Project Details http://randomnerdtutorials.com
 */

// include TFT and SPI libraries
#include <TFT.h>  
#include <SPI.h>

// pin definition for Arduino UNO
#define cs   10
#define dc   9
#define rst  8


// create an instance of the library
TFT TFTscreen = TFT(cs, dc, rst);

void setup() {

  //initialize the library
  TFTscreen.begin();

  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);
  //set the text size
  TFTscreen.setTextSize(2);
}

void loop() {

  //generate a random color
  int redRandom = random(0, 255);
  int greenRandom = random (0, 255);
  int blueRandom = random (0, 255);
  
  // set a random font color
  TFTscreen.stroke(redRandom, greenRandom, blueRandom);
  
  int width = TFTscreen.width();
  char charArray[10]; 
  itoa(width, charArray, 10);
  TFTscreen.text(charArray, 6, 57);     // 160 px

  int height = TFTscreen.height();
  char charArray2[10]; 
  itoa(height, charArray2, 10);
  TFTscreen.text(charArray2, 55, 57);   // 128 px
  
  
  // wait 200 miliseconds until change to next color
  delay(200);
}
