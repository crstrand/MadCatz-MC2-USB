# MadCatz-MC2-USB
## Use an Arduino Pro Micro to repurpose the <br>Mad Catz MC2 steering wheel/pedal set to USB<br>
## Features
### Now with Cosine scaling of the steering wheel input!<br>
Even though the steering wheel seems to move +/- 135degrees, if you take the steering wheel value and cosine scale it between +/-90 degrees, you get a steering wheel that has less input in the center and increasing input closer to the ends of travel.<br>
The software now has a default of 70 degrees which is settable in the calibration menu (access with a serial terminal "h" for help menu).<br>
70 degrees further dampens the steering in the center section, with the tradeoff of not full steering output at wheel lock.
**PLEASE NOTE: If you calibrate your steering wheel in windows, it will take the lower signals as the max and will scale it back up to look like the 90 degree graph.<br>
Therefore, do *not* scale the steering wheel in windows.**<br>
The accelerator has a 1.2 multiplier on it so calibrating that would be bad also.<br>
![Linear_vs_Cosine_graph.png](Linear_vs_Cosine_graph.png)
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
