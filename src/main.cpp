// Program used to make an old Mad Catz MC2 XBox/PS2/N64 Wheel and Pedals
// work via USB Joystick object
//------------------------------------------------------------

#include <Arduino.h>
#include "Joystick.h"
#include <EEPROM.h>

#define DEBUG 0
#define TIMEOUT_HALF_SECONDS 20

/*
Pin	Function      Wire Color
D1  CAL           ORG/WHT(J5)
D2	Left paddle   GRN/WHT
D3	Right paddle  PNK
D4	Shift Up      GRN(2)
D5	Shift Down    BLU(2)
D6	DUP           PNK/BLK
D7	DDN           YEL/BLK
D8	DL            ORG/WHT
D9	DR            RED/WHT
D10	Start         RED
D16	Cross         GRY
D14	Circle        GRN
D15	Square        WHT
D18	Triangle      BLU
A1	Accel         BRN(2)
A2	Brake         RED(2)
A3	Steering      WHT(3)
*/
#define CAL       1
#define PADDLE_L  2
#define PADDLE_R  3
#define SHIFT_UP  4
#define SHIFT_DN  5
#define DUP 6
#define DDN 7
#define DLT 8
#define DRT 9
#define START    10
#define CROSS    16
#define CIRCLE   14
#define SQUARE   15
#define TRIANGLE 18
#define ACCEL    A1
#define BRAKE    A2
#define WHEEL    A3

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  9, 4,                  // Button Count, Hat Switch Count
  false, false, false,   // no X and no Y, no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  true, true, true);     // accelerator, brake, and steering

// Set to true to test "Auto Send" mode or false to test "Manual Send" mode.
//const bool testAutoSendMode = true;
const bool testAutoSendMode = false;


struct caltype
{
int steering_left = 0;
int steering_right = 1023;
int steering_center = 512;
int steering_db = 10;
int accel_min = 0;
int accel_max = 1023;
int brake_min = 0;
int brake_max = 1023;
} wheelcal;

int _dpad_switch[4]={DUP,DRT,DDN,DLT};
int lastButtonState[4] = {0,0,0,0};

void read_DPAD()
{
  bool valueChanged = false;

  // Read pin values
    for (int index = 0; index < 4; index++)
    {
      int currentButtonState = !digitalRead(_dpad_switch[index]);
      if (currentButtonState != lastButtonState[index])
      {
        valueChanged = true;
        lastButtonState[index] = currentButtonState;
      }
    }

    if (valueChanged) {
      
      if ((lastButtonState[0] == 0)
        && (lastButtonState[1] == 0)
        && (lastButtonState[2] == 0)
        && (lastButtonState[3] == 0)) {
          Joystick.setHatSwitch(0, -1);
      }
      if (lastButtonState[0] == 1) {
        Joystick.setHatSwitch(0, 0);
      }
      if (lastButtonState[1] == 1) {
        Joystick.setHatSwitch(0, 90);
      }
      if (lastButtonState[2] == 1) {
        Joystick.setHatSwitch(0, 180);
      }
      if (lastButtonState[3] == 1) {
        Joystick.setHatSwitch(0, 270);
      }
      
    } // if the value changed

}

void read_buttons()
{
  Joystick.setButton(0,digitalRead(PADDLE_L));
  Joystick.setButton(1,digitalRead(PADDLE_R));
  Joystick.setButton(2,digitalRead(SHIFT_UP));
  Joystick.setButton(3,digitalRead(SHIFT_DN));
  Joystick.setButton(4,digitalRead(START));
  Joystick.setButton(5,digitalRead(CROSS));
  Joystick.setButton(6,digitalRead(CIRCLE));
  Joystick.setButton(7,digitalRead(SQUARE));
  Joystick.setButton(8,digitalRead(TRIANGLE));
}

void show_menu()
{
  Serial.println("\n    Calibration menu");
  Serial.println("1. Steering wheel min/max/center");
  Serial.println("2. Accelerator min/max");
  Serial.println("3. Brake min/max");
  Serial.println("4. Steering wheel deadband setting");
  Serial.println("0. quit cal mode and save values to EEPROM");
  Serial.println("q. Quit and do not save\n");
  Serial.print("You have ");
  Serial.print(TIMEOUT_HALF_SECONDS/2);
  Serial.println(" seconds to make a selection");
}

void steering_cal()
{
  Serial.println("Hold the steering wheel all the way to the LEFT then press 'm' to measure");
  while(Serial.read()!='m') delay(500);
  wheelcal.steering_left = analogRead(WHEEL);
  Serial.print("steering_left = ");
  Serial.println(wheelcal.steering_left);
  
  Serial.println("Hold the steering wheel all the way to the RIGHT then press 'm' to measure");
  while(Serial.read()!='m') delay(500);
  wheelcal.steering_right = analogRead(WHEEL);
  Serial.print("steering_right = ");
  Serial.println(wheelcal.steering_right);

  Serial.println("Hold the steering wheel in the CENTER then press 'm' to measure");
  while(Serial.read()!='m') delay(500);
  wheelcal.steering_center = analogRead(WHEEL);
  Serial.print("steering_center = ");
  Serial.println(wheelcal.steering_center);

  Serial.println("Press any key to continue");
  while(!Serial.available()) delay(500);
}

void steering_deadband()
{
  int deadband_new = 0;
  Serial.setTimeout(10*1000);
  Serial.println("Enter the value for the steering deadband (2-100)");
  Serial.print("(full range of steering values is 0 to 1023)\nCurrent value: ");
  Serial.print(wheelcal.steering_db);
  deadband_new = Serial.readStringUntil('\n').toInt();
  if(deadband_new>=2 && deadband_new<=100)
    wheelcal.steering_db = deadband_new;
  Serial.print("\ndeadband = ");
  Serial.println(wheelcal.steering_db);
}

void accel_cal()
{
  Serial.println("Ensure the accelerator pedal is not depressed then press 'm' to measure");
  while(Serial.read()!='m') delay(500);
  wheelcal.accel_min = analogRead(ACCEL);
  Serial.print("accel_min = ");
  Serial.println(wheelcal.accel_min);
  
  Serial.println("Fully depress the accelerator pedal then press 'm' to measure");
  while(Serial.read()!='m') delay(500);
  wheelcal.accel_max = analogRead(ACCEL);
  Serial.print("accel_max = ");
  Serial.println(wheelcal.accel_max);

  Serial.println("Press any key to continue");
  while(!Serial.available()) delay(500);
}

void brake_cal()
{
  Serial.println("Ensure the brake pedal is not depressed then press 'm' to measure");
  while(Serial.read()!='m') delay(500);
  wheelcal.brake_min = analogRead(BRAKE);
  Serial.print("brake_min = ");
  Serial.println(wheelcal.brake_min);
  
  Serial.println("Fully depress the brake pedal then press 'm' to measure");
  while(Serial.read()!='m') delay(500);
  wheelcal.brake_max = analogRead(BRAKE);
  Serial.print("brake_max = ");
  Serial.println(wheelcal.brake_max);

  Serial.println("Press any key to continue");
  while(!Serial.available()) delay(500);
}

void read_cal()
{
  // read the values out of EEPROM, but make sure they make sense or just leave them at default
  caltype temp_cal;
  EEPROM.get(0,temp_cal);
  String cal_error="";
  // range check each setting
  if(temp_cal.steering_left>=0 && temp_cal.steering_left<=(1023/3))
    wheelcal.steering_left = temp_cal.steering_left;
  else
    cal_error += "invalid value for steering_left: " + temp_cal.steering_left;
  
  if(temp_cal.steering_right>=(1023*2/3) && temp_cal.steering_right<=1023)
    wheelcal.steering_right = temp_cal.steering_right;
  else
    cal_error += "invalid value for steering_right: " + temp_cal.steering_right;

  if(temp_cal.steering_center>=(1023/3) && temp_cal.steering_center<=(1023*2/3))
    wheelcal.steering_center = temp_cal.steering_center;
  else
    cal_error += "invalid value for steering_center: " + temp_cal.steering_center;

  if(temp_cal.steering_db>=2 && temp_cal.steering_db<=100)
    wheelcal.steering_db = temp_cal.steering_db;
  else
    cal_error += "invalid value for steering_db: " + temp_cal.steering_db;

  if(temp_cal.accel_min>=0 && temp_cal.accel_min<=(1023/3))
    wheelcal.accel_min = temp_cal.accel_min;
  else
    cal_error += "invalid value for accel_min: " + temp_cal.accel_min;
  if(temp_cal.accel_max>=(1023*2/3) && temp_cal.accel_max<=1023)
    wheelcal.accel_max = temp_cal.accel_max;
  else
    cal_error += "invalid value for accel_max: " + temp_cal.accel_max;

  if(temp_cal.brake_min>=0 && temp_cal.brake_min<=(1023/3))
    wheelcal.brake_min = temp_cal.brake_min;
  else
    cal_error += "invalid value for brake_min: " + temp_cal.brake_min;
  if(temp_cal.brake_max>=(1023*2/3) && temp_cal.brake_max<=1023)
    wheelcal.brake_max = temp_cal.brake_max;
  else
    cal_error += "invalid value for brake_max: " + temp_cal.brake_max;
  
  if(Serial) Serial.println(cal_error);
}

void save_cal()
{
  // save current values to EEPROM
  EEPROM.put(0,wheelcal);
}

void print_cal()
{
  Serial.println("\nCurrent calibration values:");
  Serial.print("steering_left = ");
  Serial.println(wheelcal.steering_left);
  Serial.print("steering_right = ");
  Serial.println(wheelcal.steering_right);
  Serial.print("steering_center = ");
  Serial.println(wheelcal.steering_center);
  Serial.print("steering_deadband = ");
  Serial.println(wheelcal.steering_db);
  Serial.print("accel_min = ");
  Serial.println(wheelcal.accel_min);
  Serial.print("accel_max = ");
  Serial.println(wheelcal.accel_max);
  Serial.print("brake_min = ");
  Serial.println(wheelcal.brake_min);
  Serial.print("brake_max = ");
  Serial.println(wheelcal.brake_max);
}

void do_analog_cal()
{
  bool done = false;
  bool timeout = false;
  int timeout_count = 0;

  delay(500);

  if(Serial)
  {
    while(!done)
    {
      show_menu();
      // wait for a key press, but time out in case someone accidentally pressed the cal button
      while(!Serial.available() && !timeout)
      {
        // if user hasn't made a menu selection in 10 seconds, timeout, save the values and exit cal mode
        delay(500);
        if(++timeout_count>TIMEOUT_HALF_SECONDS)
          timeout = true;
        else
        {
          if(timeout_count%2==0) Serial.print((TIMEOUT_HALF_SECONDS-timeout_count)/2);
        }
      }
      if(!timeout)
      {
        timeout_count = 0;
        Serial.println("");

        switch(Serial.read())
        {
          case '1':
            steering_cal();
            break;
          case '2':
            accel_cal();
            break;
          case '3':
            brake_cal();
            break;
          case '4':
            steering_deadband();
            break;
          case '0':
            // save to EEPROM
            Serial.println("Done calibration. Saving values to EEPROM");
            save_cal();
            done = true;
            break;
          case 'q':
            Serial.println("Exiting calibration mode.  Values NOT saved to EEPROM");
            done = true;
            break;
        }
      }
      else // timeout
        Serial.println("\nTimeout.  Exiting calibration mode.  Values NOT saved to EEPROM");

      done |= timeout;
    }
  }
}

void setup() {

  Joystick.begin(testAutoSendMode);
  
  pinMode(CAL, INPUT_PULLUP);
  pinMode(PADDLE_L, INPUT_PULLUP);
  pinMode(PADDLE_R, INPUT_PULLUP);
  pinMode(SHIFT_UP, INPUT_PULLUP);
  pinMode(SHIFT_DN, INPUT_PULLUP);
  pinMode(DUP, INPUT_PULLUP);
  pinMode(DDN, INPUT_PULLUP);
  pinMode(DLT, INPUT_PULLUP);
  pinMode(DRT, INPUT_PULLUP);
  pinMode(START, INPUT_PULLUP);
  pinMode(CROSS, INPUT_PULLUP);
  pinMode(CIRCLE, INPUT_PULLUP);
  pinMode(SQUARE, INPUT_PULLUP);
  pinMode(TRIANGLE, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  read_cal();
  // apply calibration
  Joystick.setAcceleratorRange(wheelcal.accel_min,wheelcal.accel_max);
  Joystick.setBrakeRange(wheelcal.brake_min,wheelcal.brake_max);
  Joystick.setSteeringRange(wheelcal.steering_left,wheelcal.steering_right);

  // Turn indicator light off.
  digitalWrite(LED_BUILTIN, 0);
}

bool cal_mode = false;
char temp_char;

#define ACCEL_FILTER_SAMPLES 4
#define BRAKE_FILTER_SAMPLES 4
#define WHEEL_FILTER_SAMPLES 4

int _accel_sum = 0;
int _brake_sum = 0;
int _wheel_sum = 0;
int num_accel_samples = 0;
int num_brake_samples = 0;
int num_wheel_samples = 0;

int _accel = 0;
int _brake = 0;
int _wheel = 0;

int accel_samples_buff[ACCEL_FILTER_SAMPLES];
int brake_samples_buff[BRAKE_FILTER_SAMPLES];
int wheel_samples_buff[WHEEL_FILTER_SAMPLES];
bool scanmode = false;

uint8_t i;

void loop() 
{

  accel_samples_buff[num_accel_samples] = analogRead(ACCEL);
  brake_samples_buff[num_brake_samples] = analogRead(BRAKE);
  wheel_samples_buff[num_wheel_samples] = analogRead(WHEEL);

  if(scanmode)
  {
    Serial.print("Average: ");
    Serial.print(_accel);
    Serial.print(",");
    Serial.print(_brake);
    Serial.print(",");
    Serial.print(_wheel);
    Serial.print("  Raw: ");
    Serial.print(accel_samples_buff[num_accel_samples]);
    Serial.print(",");
    Serial.print(brake_samples_buff[num_brake_samples]);
    Serial.print(",");
    Serial.print(wheel_samples_buff[num_wheel_samples]);
    Serial.print("                   \r");
  }

  num_accel_samples++;
  num_brake_samples++;
  num_wheel_samples++;

  // calculate a moving average for accelerator
  if( num_accel_samples >= ACCEL_FILTER_SAMPLES)
    num_accel_samples = 0;
  _accel_sum = 0;
  for( i = 0; i < ACCEL_FILTER_SAMPLES; i++ )
    _accel_sum += accel_samples_buff[i];
  _accel = _accel_sum / ACCEL_FILTER_SAMPLES;

  // calculate a moving average for brake
  if( num_brake_samples >= BRAKE_FILTER_SAMPLES)
    num_brake_samples = 0;
  _brake_sum = 0;
  for( i = 0; i < BRAKE_FILTER_SAMPLES; i++ )
    _brake_sum += brake_samples_buff[i];
  _brake = _brake_sum / BRAKE_FILTER_SAMPLES;

  // calculate a moving average for steering wheel
  if( num_wheel_samples >= WHEEL_FILTER_SAMPLES)
    num_wheel_samples = 0;
  _wheel_sum = 0;
  for( i = 0; i < WHEEL_FILTER_SAMPLES; i++ )
    _wheel_sum += wheel_samples_buff[i];
  _wheel = _wheel_sum / WHEEL_FILTER_SAMPLES;
  // if wheel value is in the deadband (center +/- half of deadband), set it to wheelcal.steering_center
  if(_wheel>=(wheelcal.steering_center-wheelcal.steering_db/2) && _wheel<=(wheelcal.steering_center+wheelcal.steering_db/2))
    _wheel = wheelcal.steering_center;

  Joystick.setAccelerator(1023-_accel);
  Joystick.setBrake(_brake);
  Joystick.setSteering(_wheel);

  read_buttons();
  read_DPAD();

  if (testAutoSendMode == false)
  {
    Joystick.sendState();
  }
  switch(Serial.read())
  {
    case 'c':
      do_analog_cal();
      // apply calibration
      Serial.println("Applying calibration");
      Joystick.setAcceleratorRange(wheelcal.accel_min,wheelcal.accel_max);
      Joystick.setBrakeRange(wheelcal.brake_min,wheelcal.brake_max);
      Joystick.setSteeringRange(wheelcal.steering_left,wheelcal.steering_right);
      break;
    case 'p':
      print_cal();
      break;
    case 's':
      scanmode = !scanmode;
      break;
    case 'h':
      Serial.println("c - calibrate\np - print cal values\ns - analog scan mode\nh - this help screen\na - about this software");
      break;
    case 'a':
      Serial.println("\nMadCatz MC2 USB Conversion Firmware\nfor Arduino Pro Micro (Atmega32U4)\nCopyright 2020 Cam Strandlund\n");
      break;
  }

#if DEBUG
  Serial.print("accel = ");
  Serial.print(_accel);
  Serial.print("  brake = ");
  Serial.print(_brake);
  Serial.print("  wheel = ");
  Serial.println(_wheel);
  #if 0
  Serial.println("012345678");
  Serial.println("---------");
  Serial.print(digitalRead(PADDLE_L));
  Serial.print(digitalRead(PADDLE_R));
  Serial.print(digitalRead(SHIFT_UP));
  Serial.print(digitalRead(SHIFT_DN));
  Serial.print(digitalRead(START));
  Serial.print(digitalRead(CROSS));
  Serial.print(digitalRead(CIRCLE));
  Serial.print(digitalRead(SQUARE));
  Serial.println(digitalRead(TRIANGLE));
  #endif
  delay(500);
#else
  delay(50);
#endif
}