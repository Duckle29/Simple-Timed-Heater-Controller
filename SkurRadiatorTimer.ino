/*
A radiator timer, for controlling radiators in a location that doesn't need them constantly on.

This was created for controlling the heating in our workshop.
It will try to keep the shop heated to a set value (8°C standard),
and if a button is pushed, will start heating up to a different set point (18°C standard)
for a set amount of time (2 hours standard)

Written by Mikkel DB Jeppesen 2014
TODO: License it under LGPL
*/

/*------------------------------
INCLUDED FILES UNDER HERE
------------------------------*/
#include <Wire.h>
#include <LiquidTWI2.h>
#include <Encoder.h>
#include <EEPROM.h>

/*------------------------------
PROTOTYPES HERE
------------------------------*/
int deBounce(int pin, int arrIndex, bool activeState = false);

/*------------------------------
VARIABLES UNDER HERE
------------------------------*/

// Store the different pins in variables
int tempSens = A3;
int extBtn = 7;
int encBtn = 6;
int SSR = 9;
int redLed = 10;
int buzzer = 4;

// Integers used in the program
int backLightDelay = 15*1000;
int buzzMs = 30;

int doubleclickInterval = 200; // Time in milliseconds you have to click again for it to be a double click
int pressDurationLong = 1000; // Time in milliseconds to press the button for it to be read as a long press
int bounceTime = 10; // Time to wait for a button to debounce in milliseconds

int standbyWarning = 5; // How many minutes before entering standby, should the buzzer start beeping? 0 = off.
int buzzInterval = 3000; // Time between buzzes in ms when the heater is about to enter standby mode

int encVal = 0;
int lastEncVal = 0;
int tempC;
int hysterisis;
int btnStates[] = {0,0};
int buttons[] = {encBtn, extBtn};

// The set temperature (Using a byte, as I will only need two digit numbers, then it's easier to store in EEPROM)
byte setTemp = 8;

// Booleans
boolean lastEncBtnState = false;
boolean lastExtBtnState = false;
boolean heaterState = false;
bool lastPinReading[] = {true, true};

// Different millis based delay effects
unsigned long heatTimeMinutes = 3;
unsigned long heatTimeMinutesLeft = 0;
unsigned long heatStartTime = 0;
unsigned long lastDebounceTime = 0;

unsigned long btnCountMsStart[] = {0,0};
unsigned long btnCountMs[] = {100,100};
unsigned long lastBtnClickMs[] = {0,0};

unsigned long lastMenuAction = 0;
unsigned long buzzStart = 0;
unsigned long lastBuzz = 0;
// Variables related to EEPROM
int offTempAdr = 0;
int onTempAdr = 2;
int timeAdrHighByte = 4;
int timeAdrLowByte = 5;

byte offTemp = 8;
byte onTemp = 18;
byte heatTimeHighByte = heatTimeMinutes / 256;
byte heatTimeLowByte = heatTimeMinutes % 256;


// Special variables under here
LiquidTWI2 lcd(0); // Connect via i2c, default address #0 (A0-A2 not jumpered)
Encoder ENC0(2, 3);
byte tempIcon[8] =
{
  0b00100,
  0b01010,
  0b01010,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b01110
};

/*------------------------------
SETUP FUNCTION UNDER HERE
------------------------------*/
void setup()
{
  Serial.begin(9600);

  lcd.setMCPType(LTI_TYPE_MCP23008);  // set the LCD type
  lcd.begin(8, 2);  // set up the LCD's number of rows and columns:
  lcd.createChar(0, tempIcon);

  // If custom values have been written to EEPROM, use those
  if(EEPROM.read(offTempAdr) != 255)
  {
    offTemp = EEPROM.read(offTempAdr);
  }
  if(EEPROM.read(onTempAdr) != 255)
  {
    onTemp = EEPROM.read(onTempAdr);
  }
  if(EEPROM.read(timeAdrLowByte) != 255)
  {
    heatTimeHighByte = EEPROM.read(timeAdrHighByte);
    heatTimeLowByte = EEPROM.read(timeAdrLowByte);

    heatTimeMinutes = heatTimeHighByte*256 + heatTimeLowByte;
  }

  setTemp = offTemp;

  // Set up the ATMega to use it's internal 1.1V referance, for greater accuracy on small ranges.
  analogReference(INTERNAL);

  // Set up the inputs and outputs
  pinMode(tempSens, INPUT);
  pinMode(encBtn, INPUT);
  pinMode(extBtn, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(SSR, OUTPUT);
  pinMode(redLed, OUTPUT);

  digitalWrite(buzzer, LOW);

  // Turn on the internal pullups for the pins that need that
  digitalWrite(encBtn, HIGH);
  digitalWrite(extBtn, HIGH);

  lcd.setBacklight(LOW); // Turn off the backlight of the LCD (mainly for allowing low powered programmers to power the board)
}


/*------------------------------
MAIN CODE UNDER HERE
------------------------------*/
void loop()
{
  // Update the button state
  tempC = analogRead(tempSens) * 1.07421875 /10; // Get the ambient temperature
  encVal = ENC0.read();
  displayControl();
  backlightManager();
  heaterControl();
  buzzControl();
  for(int i=0; i<2; i++)
  {
    btnStates[i] = deBounce(buttons[i], i);
  }
  /*
  if(btnStates[0] != 0 || btnStates[1] != 0)
  {
    Serial.print("enc: ");Serial.print(btnStates[0]);Serial.print(" ext\t");Serial.println(btnStates[1]);
  }
  */
}
/*------------------------------
FUNCTIONS UNDER HERE
------------------------------*/

// Function that controlls heater related functions, including the timing.
void heaterControl()
{
  // If the heater button is short-pressed. Reset the on time.
  if(btnStates[1] == -1)
  {
    heatStartTime = millis();
    heatTimeMinutesLeft = heatTimeMinutes;
    //buzzStart = millis();
  }

  // If the heater button is long-pressed. Set the time to 0.
  if(btnStates[1] == -2)
  {
    heatTimeMinutesLeft = 0;
    //buzzStart = millis();
  }

  // If it's currently in the heating mode, keep track of time, and set the target temperature
  // To the desired on temp, as chosen through the menu
  if(heatTimeMinutesLeft > 0)
  {
    int minutes = (millis()-heatStartTime)/60000;
    heatTimeMinutesLeft = heatTimeMinutes - minutes;
    setTemp = onTemp;

    // If in use. Buzz when a small amount of time is left, in case you wish to extend the timer.
    if(heatTimeMinutesLeft < standbyWarning)
    {
      if(millis() - lastBuzz > buzzInterval)
      {
        buzzStart = millis();
        lastBuzz = millis();
      }
    }

  }
  else // If the time has run out, set the temperature to the standby temp.
  {
    setTemp = offTemp;
  }

  if(tempC < setTemp-hysterisis)
  {
    heaterState = true;
  }
  if(tempC > setTemp+hysterisis)
  {
    heaterState = false;
  }

  digitalWrite(SSR, heaterState);
  digitalWrite(redLed, heaterState);
}

// Function that will return the number of digits in a passed number. For it's use in this program, it will add 1 to any negative numbers
int countDigits(long number)
{
  long result;
  int digits;

  // If the number is negative,
  if(number < 0)
  {
    result = -1 * number; // Multiply it by -1 (will flip it to positive)
    digits = 1; // And start out with digits as one (as there's a minus infront of the number)
  }
  else
  {
    result = number;
    digits = 0;
  }

  // While there's still digits left, devide by 10 and increment the digits variable
  while(result > 0)
  {
    result = result / 10;
    digits++;
  }
  return digits;
}

void buzzControl()
{
  if(millis() - buzzStart < buzzMs) // If the buzz is not yet over, write the buzzer pin high
  {
    digitalWrite(buzzer, HIGH);
  }
  else // Else write it low
  {
    digitalWrite(buzzer, LOW);
  }
}

// Debounce function used for debouncing buttons, and recognizing short, double, and long presses.
// It uses negative return values, for a future planned feature, where it will return how many fast clicks was done, instead of simply doubleclick.
int deBounce(int pin, int arrIndex, bool activeState)
{
  bool pinReading = digitalRead(pin); // Reading the current state of the pin
  unsigned long count = btnCountMs[arrIndex] - btnCountMsStart[arrIndex]; // Get the time the button was pressed

  if(pinReading != lastPinReading[arrIndex]) // If the pin state has changed
  {
    lastPinReading[arrIndex] = pinReading;
    if(pinReading != activeState && count > bounceTime) // If the button has been released after a debounce period.
    {
      if(count >= pressDurationLong) // If it was held for long enough to be considered a long press, return 2
      {
        return -2;
      }
      else // If it was less than that
      {
        if(millis() - lastBtnClickMs[arrIndex] < doubleclickInterval)
        {
          lastBtnClickMs[arrIndex] = millis();
          return 2; // If the button has been clicked fast twice, count it as a double click.
        }
        else
        {
          lastBtnClickMs[arrIndex] = millis();
          return -1; // If the button has only been pressed shortly, count it as a short-press.
        }
      }
    }
    btnCountMsStart[arrIndex] = millis(); // If the button has just been pressed, save the time it was pressed for later duration calculations
  }

  if(pinReading == activeState ) // As long as the button is pressed, keep track of what the millis is.
  {
    btnCountMs[arrIndex] = millis();
  }
  else
  if(count > bounceTime)
  {
    return 0;
  }
  return 0;
}
