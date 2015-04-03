/*
/	This file controls the menu system of the device, which allows the user to change settings such as
/	the different temperature setpoints, the timers duration and saving these to memory.
*/

// Array of strings (well pointers bleh) to use in the menu system.
char *settingsStrings[] = {"Back    ", "Off temp", "On  temp", "Time on ", "Save    "};

// Enum for what menu is configured
enum Menu
{
	none = 0,
	statusScreen,
	settings,
	lowTempEdit,
	highTempEdit,
	activeTimeEdit
};

Menu currentMenu = none;
Menu currentConfiguredMenu = none;

// Run the different menu functions depending on the current menu
void displayControl()
{
	switch (currentMenu)
	{
    case statusScreen:
      StatusScreen();
      break;

    case settings:
    	Settings();
    	break;

    case lowTempEdit:
    	LowTempEdit();
    	break;

    case highTempEdit:
    	HighTempEdit();
    	break;

    case activeTimeEdit:
    	ActiveTimeEdit();
    	break;

    default:
      currentMenu = statusScreen;
      StatusScreen();
      break;
	}
}

// Show the statusScreen
void StatusScreen()
{
		if(btnStates[0] == -1)
	{
		currentMenu = settings;
	}

	if(currentConfiguredMenu != statusScreen)
	{
		lcd.clear();
		lcd.setCursor(3,0);
        lcd.print("C");
        lcd.setCursor(7,0);
        lcd.print("C");
        lcd.setCursor(0,1);
		currentConfiguredMenu = statusScreen;
	}
	// Write the target temp (setTemp) (add a space if the number is less that 2 digits)
  lcd.setCursor(5,0);
  if(countDigits(setTemp) < 2)
  {
    lcd.print(" ");
  }
  lcd.print(setTemp);

	// Write the current temp (tempC) (add a space if the number is less that 2 digits)
  lcd.setCursor(0,0);
  lcd.write(byte(0));
  if(countDigits(tempC) < 2)
  {
    lcd.print(" ");
  }
  lcd.print(tempC);

  if(heatTimeMinutesLeft > 0)
  {
  	displayTime(heatTimeMinutesLeft, 1);
  }
  else
  {
  	lcd.setCursor(0,1);
  	lcd.print("Waiting ");
  }

}

// Show the different settings
void Settings()
{
	if(currentConfiguredMenu != settings) // If the user just entered this menu, display it.
	{
		lcd.clear(); // Clear the display
		lcd.setCursor(0, 0); // start from 0,0
		lcd.print("Setup"); // Print the current menu
		currentConfiguredMenu = settings; // The currently configured menu is now this menu
		ENC0.write(0);
	}
	encVal = ENC0.read() / 4; // Read the current position of the encoder
	lcd.setCursor(0,1);

	// Range limiting of the encoder value
	if(encVal > 4)
	{
		ENC0.write(0);
		encVal = 0;
	}
	if(encVal < 0)
	{
		ENC0.write(4*4);
		encVal = 4;
	}

	if(btnStates[0] == -1)
	{
		switch (encVal)
		{
		  case 0:
		  	currentMenu = statusScreen;
		    break;

		  case 1:
		  	currentMenu = lowTempEdit;
		  	break;

		  case 2:
		  	currentMenu = highTempEdit;
		  	break;

		  case 3:
		  	currentMenu = activeTimeEdit;
		  	break;

		  case 4:

  			if(EEPROM.read(offTempAdr) != offTemp)
  			{
     			EEPROM.write(offTempAdr, offTemp);
     			//Serial.print("LowTempSaved\n");
 			}

   			if(EEPROM.read(onTempAdr) != onTemp)
 			{
    			EEPROM.write(onTempAdr, onTemp);
    			//Serial.print("HighTempSaved\n");
 			}

 			heatTimeHighByte = heatTimeMinutes / 256;
			heatTimeLowByte = heatTimeMinutes % 256;

			if(EEPROM.read(timeAdrHighByte) != heatTimeHighByte)
			{
				EEPROM.write(timeAdrHighByte, heatTimeHighByte);
				//Serial.print("HighByteSaved\n");
			}
			if(EEPROM.read(timeAdrLowByte) != heatTimeLowByte)
			{
				//Serial.print("lowByteSaved\n");
				EEPROM.write(timeAdrLowByte, heatTimeLowByte);
			}


    		lcd.print("        ");
    		lcd.setCursor(0,1);
	    	lcd.print("Saved");
	    	delay(1500);
	    	//lcd.print(settingsStrings[encVal]);
	    	break;
		}
	}

	lcd.print(settingsStrings[encVal]);
}

// Allow the user to edit the low temperature setpoint
void LowTempEdit()
{
	if(currentConfiguredMenu != lowTempEdit) // If the user just entered this menu, display it.
	{
		lcd.clear(); // Clear the display
		lcd.setCursor(0, 0); // start from 0,0
		lcd.print(settingsStrings[1]); // Print the current menu
		lcd.setCursor(7,1);
		lcd.print("C");
		currentConfiguredMenu = lowTempEdit; // The currently configured menu is now this menu
		ENC0.write(offTemp*4);
	}
	encVal = ENC0.read() / 4;
	lcd.setCursor(5,1);

	if(encVal > 60)
	{
		encVal = 60;
		ENC0.write(60*4);
	}
	if(encVal < 0)
	{
		encVal = 0;
		ENC0.write(0);
	}

	if(countDigits(offTemp) < 2)
  {
    lcd.print(" ");
  }
  lcd.print(offTemp);
  offTemp = encVal;

  if(btnStates[0] == -1)
  {
  	currentMenu = settings;
  }
}

// Allow the user to edit the high teperature setpoint
void HighTempEdit()
{
	if(currentConfiguredMenu != highTempEdit) // If the user just entered this menu, display it.
	{
		lcd.clear(); // Clear the display
		lcd.setCursor(0, 0); // start from 0,0
		lcd.print(settingsStrings[2]); // Print the current menu
		lcd.setCursor(7,1);
		lcd.print("C");
		currentConfiguredMenu = highTempEdit; // The currently configured menu is now mainMenu
		ENC0.write(onTemp*4);
	}
	encVal = ENC0.read() / 4;
	lcd.setCursor(5,1);

	if(encVal > 60)
	{
		encVal = 60;
		ENC0.write(60*4);
	}
	if(encVal < 0)
	{
		encVal = 0;
		ENC0.write(0);
	}

	if(countDigits(onTemp) < 2)
  {
    lcd.print(" ");
  }
  lcd.print(onTemp);
  onTemp = encVal;

  if(btnStates[0] == -1)
  {
  	currentMenu = settings;
  }
}

// Allow the user to edit the time that it will stay on the high temperature setpoint, before resetting.
void ActiveTimeEdit()
{
	if(currentConfiguredMenu != highTempEdit) // If the user just entered this menu, display it.
	{
		lcd.clear(); // Clear the display
		lcd.setCursor(0, 0); // start from 0,0
		lcd.print(settingsStrings[3]); // Print the current menu
		currentConfiguredMenu = highTempEdit; // The currently configured menu is now mainMenu
		ENC0.write(heatTimeMinutes/5*4);
	}
	encVal = ENC0.read() / 4;

	if(encVal > 6000)
	{
		encVal = 6000;
		ENC0.write(6000*4);
	}
	if(encVal < 0)
	{
		encVal = 0;
		ENC0.write(0);
	}

	displayTime(heatTimeMinutes, 1);

	heatTimeMinutes = encVal*5;

	// If the encoder has been pushed, leave the submenu.
	if(btnStates[0] == -1)
  {
  	currentMenu = settings;
  }
}

// Control the backlight, and beep the buzzer with any action the user dows.
void backlightManager()
{
	if(ENC0.read()/4 != lastEncVal/4 || btnStates[1] != 0 || btnStates[0] != 0)
	{
		lastMenuAction = millis();
		lastEncVal = ENC0.read();
		buzzStart = millis();
	}

	if(millis()-lastMenuAction > backLightDelay)
	{
		lcd.setBacklight(false);
		currentMenu = statusScreen;
	}
	else
	{
		lcd.setBacklight(true);
	}
}

// Function used to display time in a readable, hours minutes format, in 8 characters space.
void displayTime(int minutesIn, int line)
{
	int spaces = 6;
	int hours = minutesIn / 60;
	int minutes = minutesIn % 60;

	lcd.setCursor(7,line);
	lcd.print("M");
	lcd.setCursor(0,line);

	if(minutes > 9)
	{
		spaces--;
	}

	if(hours > 0)
	{
		spaces -= 2;
	}

	if(hours > 9)
	{
		spaces--;
	}

	if(hours > 99)
	{
		spaces--;
	}

	// Add spaces to the number, depending on it's length.
	for(int i=0; i<spaces-1; i++)
	{
		lcd.print(" ");
	}

	if(hours > 0)
	{
		lcd.print(hours);
		lcd.print("H");
	}
	lcd.print(" ");
	lcd.print(minutes);
}
