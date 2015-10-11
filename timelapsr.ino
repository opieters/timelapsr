#include <multiCameraIrControl.h>
#include <SevSeg.h>
#include <math.h>

// interval duration struct
typedef struct interval_t {
  word duration;
  boolean fast;
} interval_t;

// pin configuration
byte IRPin = A4;
byte btnUp=10, btnDown=9, btnLeft=12, btnRight=11;
int digit1=A3, digit2=A2, digit3=A1, digit4=13;
int segA=A0, segB=6, segC=1, segD=4, segE=5, segF=8, segG=0, segDP=3;
int nDigits = 4;

// display
SevSeg segDisplay;

// camera
Sony camera(IRPin);

// global properties and variables
word maxValueDisp = 9999;
byte selectedMenuItem;

void setup() {
  // setup buttons
  pinMode(btnUp, INPUT);
  pinMode(btnDown, INPUT);
  pinMode(btnLeft, INPUT);
  pinMode(btnRight, INPUT);

  // setup IR led
  pinMode(IRPin, OUTPUT);

  // setup seven segment display
  segDisplay.Begin(COMMON_ANODE, nDigits, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);
  segDisplay.SetBrightness(100);
}

void loop() {
  // get menu item
  selectedMenuItem = mainMenu();
  // execute menu item
  switch(selectedMenuItem){
    case 0:
      timelapse();
      break;
    case 1:
      bulb();
      break;
    case 2:
      fastTimelapse();
      break;
    default:
      break;
  }
}

// main menu definition
byte mainMenu(){
  byte index = 0;
  byte nOptions = 3;

  // wait for user to select menu entry
  while(digitalRead(btnRight) != HIGH){
    // display current item
    switch(index){
      case 0:
        segDisplay.DisplayString(" TL ", 0); // timelapse
        break;
      case 1:
        segDisplay.DisplayString(" LS ", 0); // long shutterspeed
        break;
      case 2:
        segDisplay.DisplayString("FTL ", 0);; // fast timelapse
        break;
    }

    // read button inputs
    if(digitalRead(btnUp) == HIGH){
      index = (index + 1) % nOptions;
      while(digitalRead(btnUp) == HIGH);
    }
    if(digitalRead(btnDown) == HIGH){
      index = (index - 1 + nOptions) % nOptions;
      while(digitalRead(btnDown) == HIGH);
    }
  }
  // wait for user to release press
  while(digitalRead(btnRight) == HIGH);

  // return selected entry
  return index;
}

// timelapse menu + camera control
void timelapse(){
  // get duration between frames
  displayMessage("DUR",2000);
  interval_t recInterval = getInterval(1,false,1,9999);

  // get timelapse duration
  displayMessage("RUN",2000);
  displayMessage("FOR",2000);
  interval_t recDuration = { 0, true }; // run loop at least once
  // duration must be at least one second
  while(recDuration.fast == true){
    recDuration = getInterval(1,false,1,9999);
    if(recDuration.fast == true){
      // display error: a value in seconds is required
      displayMessage("SEC",2000);
      displayMessage("RQD",2000);
    }
  }

  // start recording
  // default values
  boolean displayOn = true, updateDisplay = true;
  word nFrames = 0;
  unsigned long holdTime, nFramesToRec, timer;
  char nFramesString[5];

  // convert entered values to durations interpretable for arduino
  // holdTime = time between frames
  // nFramesToRec = number of frames that should be recorded before timelapse ends
  if(recInterval.fast == true){
    holdTime = 1e6 / recInterval.duration;
    nFramesToRec = recDuration.duration * recInterval.duration;
  } else {
    holdTime = 1e6 * recInterval.duration;
    nFramesToRec = recDuration.duration / recInterval.duration;
  }

  // set timer and start recording loop
  timer = micros();
  while((digitalRead(btnLeft) != HIGH) && (nFrames < nFramesToRec)){
    // turn display off if needed
    if(digitalRead(btnRight) == HIGH){
      displayOn = !displayOn;

      // wait for release
      while(digitalRead(btnRight) == HIGH);
    }

    // display number of frames if needed
    if(displayOn){
      if(updateDisplay){
        String tmpString(nFrames);
        while(tmpString.length()<4)
          tmpString = " " + tmpString;

        tmpString.toCharArray(nFramesString,5);
      }
      segDisplay.DisplayString(nFramesString, 0);
    }

    // take image
    if(timer<micros()){
      camera.shutterNow();
      timer = micros() + holdTime;
      nFrames++;
      updateDisplay = true;
    }
  }
}

// timelapse menu + camera control
void fastTimelapse(){
  // get duration between frames
  displayMessage("DUR",2000);
  interval_t recInterval = getInterval(1,false,1,9999);

  // start recording
  // default values
  boolean displayOn = true, updateDisplay = true;
  word nFrames = 0;
  unsigned long holdTime;
  char nFramesString[5];

  // convert entered values to durations interpretable for arduino
  // holdTime = time between frames
  // nFramesToRec = number of frames that should be recorded before timelapse ends
  if(recInterval.fast == true){
    holdTime = 1e3 / recInterval.duration;
  } else {
    holdTime = 1e3 * recInterval.duration;
  }

  // set timer and start recording loop
  while(digitalRead(btnLeft) != HIGH){
    // turn display off if needed
    if(digitalRead(btnRight) == HIGH){
      displayOn = !displayOn;

      // wait for release
      while(digitalRead(btnRight) == HIGH);
    }

    // display number of frames if needed
    if(displayOn){
      if(updateDisplay){
        String tmpString(nFrames);
        while(tmpString.length()<4)
          tmpString = " " + tmpString;

        tmpString.toCharArray(nFramesString,5);
      }
      segDisplay.DisplayString(nFramesString, 0);
    }

    // take image
    camera.shutterNow();
    nFrames++;
    updateDisplay = true;
    delay(holdTime);
  }
}

// display a 4-char message for the time specified
void displayMessage(String msg, unsigned long duration){
  char msg_char[5];
  unsigned long endTime = millis() + duration;

  while(msg.length()<4)
    msg = " " + msg;
  msg.toCharArray(msg_char,5);

  while((millis()<endTime) && (digitalRead(btnRight) != HIGH) && (digitalRead(btnLeft) != HIGH)){
     segDisplay.DisplayString(msg_char, 0);
  }
  while(digitalRead(btnRight) == HIGH);
  while(digitalRead(btnLeft) == HIGH);
}

// get an interval (in seconds or in Hertz)
struct interval_t getInterval(unsigned int stepSize, boolean exponential, int minValue, int maxValue){
  // variables
  char dispText[5];
  word duration = 1;
  boolean inverse = false, updateDisplay = true;

  // indicate interval
  displayMessage("INT", 1000);

  // user sets interval
  while(digitalRead(btnRight) != HIGH){
    // update display text if needed
    if(updateDisplay){
      String tmpText = String(duration);
      while(tmpText.length()<4)
        tmpText = " " + tmpText;
      tmpText.toCharArray(dispText,5);
    }
    // display text
    segDisplay.DisplayString(dispText, (inverse?1:0));

    // read buttons
    if(digitalRead(btnUp) == HIGH){
      if(duration < maxValue){
        if(exponential)
          duration *= stepSize;
        else
          duration += stepSize;
        if(duration > maxValue)
          duration = maxValue;
        updateDisplay = true;
      }
      // wait for release
      while(digitalRead(btnUp) == HIGH);
    }
    if(digitalRead(btnDown) == HIGH){
      // flip from Hertz to seconds (or vice verse)
      if(duration==1){
        inverse = !inverse;
        if(inverse)
          displayMessage("HZ",2000);
        else
          displayMessage("SEC",2000);
      }
      // decrease value
      if(duration>minValue){
        if(exponential)
          duration /= stepSize;
        else
          duration -= stepSize;
        if(duration<minValue)
          duration = minValue;
      }
      updateDisplay = true;
      // wait for release
      while(digitalRead(btnDown) == HIGH);
    }
  }

  // wait until button released
  while(digitalRead(btnRight) != LOW);
  displayMessage("GOOD", 1000);

  // return shutterspeed
  return {duration, inverse};
}

// camera in bulb mode
void bulb(){

  // get variables
  interval_t shutterOpen, recInterval, recDuration;
  // display info message
  displayMessage("SET",2000);
  displayMessage("SONY",2000);
  displayMessage("TO",2000);
  displayMessage("BULB",2000);

  // get shutter open time
  displayMessage("SHUT",2000);
  shutterOpen = getInterval(2,true,1,9999);

  // get time between frames
  displayMessage("DUR",2000);
  recInterval = getInterval(1,false,1,9999);

  // get duration of recording
  displayMessage("REC",2000);
  displayMessage("DUR",2000);
  recDuration = getInterval(1,false,0,9999);

  // start recording
  boolean displayOn = true, updateDisplay = true;
  word nFrames = 0;
  unsigned long holdTime, shutTime, nFramesToRec, timer;
  char nFramesString[5];

  // time the shutter is open
  if(shutterOpen.fast == true)
    shutTime = 1e3 / shutterOpen.duration;
  else
    shutTime = 1e3 * shutterOpen.duration;

  // time between frames and number of rames to record
  if(recInterval.duration > 0){
    if(recInterval.fast == true){
      holdTime = 1e6 / recInterval.duration;
      nFramesToRec = recDuration.duration * recInterval.duration;
    } else {
      holdTime = 1e6 * recInterval.duration;
      nFramesToRec = recDuration.duration / recInterval.duration;
    }
  }

  // init timer
  timer = micros();

  while(digitalRead(btnLeft) != HIGH){
    if(digitalRead(btnRight) == HIGH){
      displayOn = !displayOn;

      // wait for release
      while(digitalRead(btnRight) == HIGH);
    }

    // display number of frames if needed
    if(displayOn){
      if(updateDisplay){
        String tmpString(nFrames);
        while(tmpString.length()<4)
          tmpString = " " + tmpString;

        tmpString.toCharArray(nFramesString,5);
      }
      segDisplay.DisplayString(nFramesString, 0);
    }

    // take image autmatically if required
    if((timer<micros()) && (recDuration.duration > 0)){
      camera.shutterNow();
      delay(shutTime);
      camera.shutterNow();
      timer = micros() + holdTime;
      nFrames++;
      updateDisplay = true;
    }

    // take image if button pressed
    if(recDuration.duration == 0 && (digitalRead(btnUp) == HIGH || digitalRead(btnDown) == HIGH)){
      // display release message
      displayMessage("REL",2000);
      camera.shutterNow();
      delay(shutTime);
      camera.shutterNow();
      // wait for button release
      while(digitalRead(btnUp) == HIGH);
      while(digitalRead(btnDown) == HIGH);
    }

    // stop recording if needed
    if(nFrames>nFramesToRec && recDuration.duration > 0)
      break;
  }
}
