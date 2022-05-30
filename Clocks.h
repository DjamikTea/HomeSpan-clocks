#include <Adafruit_NeoPixel.h>
#include <bits/stdc++.h>
#include "extras/PwmPin.h"   
#include "HomeSpan.h"

int R = 100;  // color clocks (global int)
int G = 100;
int B = 100;

#define PIN        14 
#define NUMPIXELS 56

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool segment_int[10][7] = {     // number to 7 segment leds
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 0, 1, 1, 0, 0, 0}, // 1
        {0, 1, 1, 0, 1, 1, 1}, // 2
        {0, 1, 1, 1, 1, 0, 1}, // 3
        {1, 0, 1, 1, 0, 0, 1}, // 4
        {1, 1, 0, 1, 1, 0, 1}, // 5
        {1, 1, 0, 1, 1, 1, 1}, // 6
        {0, 1, 1, 1, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 1, 0, 1}, // 9
}; 

bool pixels_status[NUMPIXELS] = { // this array stores the status of each LED on
          1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1,

};

void segment_on(int segmInd, int segm, bool on, int r, int g, int b) { // func change on/off on segment
    /*

    int segmInd - index 7 segment LED | input: 0-3
    int segm - segment in the indicator | input: 0-6
    bool on - on/off segment
    int r,g,b - color

    */
    if (!on) {
      r = 0; // off segment
      g = 0;
      b = 0;

      pixels_status[segm * 2 + (segmInd * 14)] = 0;     // write status false in array
      pixels_status[segm * 2 + 1 + (segmInd * 14)] = 0; // write status false in array
    } else {
      pixels_status[segm * 2 + (segmInd * 14)] = 1;     // write status true in array
      pixels_status[segm * 2 + 1 + (segmInd * 14)] = 1; // write status true in array
    }
    pixels.setPixelColor(segm * 2 + (segmInd * 14), r, g, b);
    pixels.setPixelColor(segm * 2 + 1 + (segmInd * 14), r, g, b);

    LOG2("seg on: ind " + String(segmInd) + ", segm " + String(segm) + ", pins " + String(segm * 2 + (segmInd * 14)) + ' ' + String(segm * 2 + 1 + (segmInd * 14)) + ", on " + String(on) + '\n'); // log

    pixels.show();
}

void write_num(int segmInd, int num) { // set number on segment
  /*
  int segmInd - index 7 segment LED | input: 0-3
  int num - the number to display on the 7 segment indicator | input: 0-9
  */

  for (int ind = 0; ind <= 6; ind++) {
    segment_on(segmInd, ind, segment_int[num][ind], R, G, B);
    LOG1("write num: segInd " + String(segmInd) + ", segm " + String(ind) + ", numb " + String(segment_int[num][ind]) + ", RGB " + String(R) + ' ' + String(G) + ' ' + String(B) + '\n');
  }
}

struct RGB_Clocks : Service::LightBulb {       
  
  SpanCharacteristic *power;                   // reference to the On Characteristic
  SpanCharacteristic *H;                       // reference to the Hue Characteristic
  SpanCharacteristic *S;                       // reference to the Saturation Characteristic
  SpanCharacteristic *V;                       // reference to the Brightness Characteristic
  
  RGB_Clocks() : Service::LightBulb(){       // constructor() method

    power=new Characteristic::On(1);                    
    H=new Characteristic::Hue(360);              // instantiate the Hue Characteristic with an initial value of 0 out of 360
    S=new Characteristic::Saturation(0);       // instantiate the Saturation Characteristic with an initial value of 0%
    V=new Characteristic::Brightness(50);     // instantiate the Brightness Characteristic with an initial value of 100%
    H->setRange(0,360,0.1);
    S->setRange(0,100,0.1);
    V->setRange(0,100,0.1);
  } // end constructor

  boolean update(){                         // update() method

    boolean p;
    float v, h, s, r, g, b;

    h=H->getNewVal<float>();                      // get and store all current values.  Note the use of the <float> template to properly read the values
    s=S->getNewVal<float>();
    v=V->getNewVal<float>();                      // though H and S are defined as FLOAT in HAP, V (which is brightness) is defined as INT, but will be re-cast appropriately
    p=power->getNewVal();

    char cBuf[128];

    // Here we call a static function of LedPin that converts HSV to RGB.
    // Parameters must all be floats in range of H[0,360], S[0,1], and V[0,1]
    // R, G, B, returned [0,1] range as well

    LedPin::HSVtoRGB(h,s/100.0,v/100.0,&r,&g,&b);   // since HomeKit provides S and V in percent, scale down by 100

    R=p*r*255;                                      // since LedPin uses percent, scale back up by 100, and multiple by status fo power (either 0 or 1)
    G=p*g*255;
    B=p*b*255;

    sprintf(cBuf,"RGB=(%d,%d,%d)\n",R,G,B);
    LOG1(cBuf);

    if(R == 0 && G == 0 && B == 0){ // off clocks
      pixels.clear(); 
    } else {
    for(int i=0; i<NUMPIXELS; i++){ // change the color of the LEDs that are on
        if(pixels_status[i] == 1){
          pixels.setPixelColor(i, R,G,B); // change
        } else {
          pixels.setPixelColor(i, 0,0,0); // off
        }
      }
    }
    pixels.show(); // show changes  
    return(true);                               // return true
  
  } // update
};
