# MadCatz-MC2-USB
## Use an Arduino Pro Micro to repurpose the <br>Mad Catz MC2 steering wheel/pedal set to USB<br>
## Wiring
The wiring is included as comments at the top of main.cpp.<br>
This requires **major** rewiring of your wheel.  You will remove the stock circuit board.<br>
**Your MadCatz MC2 will no longer work with XBox/PS2/N64.**<br>
Due to IO limitations, not all buttons are functional.  The LED bar graph is not connected.<br>
I inverted the +5v and ground lines to the steering pot and the brake and accelerator pots to get them to provide 0V when fully left (or not depressed) and max signal when fully right (or depressed fully).<br>
The signals from the A,B,X,Y buttons are much lower than expected (1.5V when it should be 5V). As such, the buttons don't *always* behave as expected. I may tear into the wheel and see what is going on later.
## Software
The code is compiled in Visual Studio Code with PlatformIO.<br>
I use the library [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary.git) by Matthew Heironimus
