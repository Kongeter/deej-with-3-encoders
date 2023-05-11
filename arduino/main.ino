#include <Arduino.h>

//! you need to disconnect Pin D8 before you can flash the Chip

// How many steps of Volume/10 per Revolution Step (20 is about 2 per Step)
#define ChangeratePerKlick 20

// Set the Startval of each (virual) encoder, from 0-1023, 700 will be around 70% Volume etc.
#define Startval1 700
#define Startval2 700
#define Startval3 700
#define Startval4 700
#define Startval5 700
#define Startval6 700

//! Some Pins dont work here (pin3,4 can not be pulled down on start, and can not be used for fast input pulses)
// Pins of encoder 1
#define Enc1Pin1 D5
#define Enc1Pin2 D6
#define Enc1ButtonPin D0

// Pins of encoder 2
#define Enc2Pin1 D1
#define Enc2Pin2 D2
#define Enc2ButtonPin D3

// Pins of encoder 3
#define Enc3Pin1 D8
#define Enc3Pin2 D7
#define Enc3ButtonPin D4

const int NUM_SLIDERS = 6;

int analogSliderValues[NUM_SLIDERS] = {Startval1, Startval2, Startval3, Startval4, Startval5, Startval6};

void sendSliderValues()
{
  delay(5);
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++)
  {
    builtString += String((int)analogSliderValues[i]);

    if (i < NUM_SLIDERS - 1)
    {
      builtString += String("|");
    }
  }

  Serial.println(builtString);
}

class Encoder
{
private:
  int encodernumber;
  int Pin1;
  int Pin2;
  int PinButton;
  int maxEncoderValue;
  int minEncoderValue;
  int encoderPos;
  int encoderPosButton;
  int encoderPin1Last;
  uint8_t state = 0;

public:
  Encoder(int Encodernumber, int Pin1, int Pin2, int PinButton, int EncoderStartVal = 0, int EncoderButtonStartVal = 0, int MinEncoderValue = 0, int MaxEncoderValue = 1023)
  {
    this->encodernumber = Encodernumber;
    this->Pin1 = Pin1;
    this->Pin2 = Pin2;
    this->PinButton = PinButton;
    this->encoderPos = EncoderStartVal;
    this->encoderPosButton = EncoderButtonStartVal;
    this->minEncoderValue = MinEncoderValue;
    this->maxEncoderValue = MaxEncoderValue;

    this->encoderPin1Last = LOW;
    pinMode(Pin1, INPUT);
    pinMode(Pin2, INPUT);
    pinMode(PinButton, INPUT);
  }
  void EncoderUpdate()
  {
    bool CLKstate = digitalRead(Pin1);
    bool DTstate = digitalRead(Pin2);
    switch (state)
    {
    case 0: // Idle state, encoder not turning
      if (!CLKstate)
      { // Turn clockwise and CLK goes low first
        state = 1;
      }
      else if (!DTstate)
      { // Turn anticlockwise and DT goes low first
        state = 4;
      }
      break;
    // Clockwise rotation
    case 1:
      if (!DTstate)
      { // Continue clockwise and DT will go low after CLK
        state = 2;
      }
      break;
    case 2:
      if (CLKstate)
      { // Turn further and CLK will go high first
        state = 3;
      }
      break;
    case 3:
      if (CLKstate && DTstate)
      { // Both CLK and DT now high as the encoder completes one step clockwise
        state = 0;
        if (digitalRead(PinButton) == 1)
        {
          encoderPos += ChangeratePerKlick;
          encoderPos = min(encoderPos, maxEncoderValue);
          analogSliderValues[encodernumber] = encoderPos;
        }
        else
        {
          encoderPosButton += ChangeratePerKlick;
          encoderPosButton = min(encoderPosButton, maxEncoderValue);
          analogSliderValues[encodernumber + NUM_SLIDERS / 2] = encoderPosButton;
        }

        sendSliderValues();
      }
      break;
    // Anticlockwise rotation
    case 4: // As for clockwise but with CLK and DT reversed
      if (!CLKstate)
      {
        state = 5;
      }
      break;
    case 5:
      if (DTstate)
      {
        state = 6;
      }
      break;
    case 6:
      if (CLKstate && DTstate)
      {
        state = 0;
        if (digitalRead(PinButton) == 1)
        {
          encoderPos -= ChangeratePerKlick;
          encoderPos = max(encoderPos, minEncoderValue);
          analogSliderValues[encodernumber] = encoderPos;
        }
        else
        {
          encoderPosButton -= ChangeratePerKlick;
          encoderPosButton = max(encoderPosButton, minEncoderValue);
          analogSliderValues[encodernumber + NUM_SLIDERS / 2] = encoderPosButton;
        }
        sendSliderValues();
      }
      break;
    }
  }
  void SetEncoder(int NewVal, bool ButtonPressed)
  {
    if (ButtonPressed)
    {
      encoderPosButton = min(max(NewVal, minEncoderValue), maxEncoderValue);
    }
    else
    {
      encoderPos = min(max(NewVal, minEncoderValue), maxEncoderValue);
    }
  }
  std::tuple<int, int> GetEncoderVals()
  {
    return {encoderPos, encoderPosButton};
  }
  int GetEncoderValNoB()
  {
    return encoderPos;
  }
  int GetEncoderValB()
  {
    return encoderPosButton;
  }
};

Encoder Enc0(0, Enc1Pin1, Enc1Pin2, Enc1ButtonPin, Startval1, Startval4);
Encoder Enc1(1, Enc2Pin1, Enc2Pin2, Enc2ButtonPin, Startval2, Startval5);
Encoder Enc2(2, Enc3Pin1, Enc3Pin2, Enc3ButtonPin, Startval3, Startval6);

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  Enc0.EncoderUpdate();
  Enc1.EncoderUpdate();
  Enc2.EncoderUpdate();
}