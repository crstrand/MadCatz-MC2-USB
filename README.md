# MadCatz-MC2-USB
## Use an Arduino Pro Micro to repurpose an old steering wheel/pedal set to USB<br>
The wiring is included as comments at the top of main.cpp.<br>
I inverted the +5v and ground lines to the steering pot and the brake and accelerator pots to get them to provide 0V when fully left (or not depressed) and max signal when fully right (or depressed fully).
This requires **major** rewiring of your wheel.  You will remove the stock circuit board.  **Your MadCatz MC2 will no longer work with XBox/PS2/N64.**<br>
Due to IO limitations, not all buttons are functional.  The LED bar graph is not connected.<br>
The code is compiled in Visual Studio Code with PlatformIO.<br>
I use the library [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary.git) by Matthew Heironimus
