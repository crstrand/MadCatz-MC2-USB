// Program used to make an old Mad Catz MC2 XBox/PS2/N64 Wheel and Pedals
// work via USB Joystick object
//------------------------------------------------------------

#include <Arduino.h>
#include "Joystick.h"
#include <EEPROM.h>

#define DEBUG 0
#define TIMEOUT_HALF_SECONDS 20
#define ENABLESERIAL 1
#define ACCELAVG 0
#define ACCELSCALING 1
#define BRAKEAVG 0
#define WHEELAVG 1
#define COSINE_SCALING 1
#define TIMESTUDY 0
#define NUMLOOPS 100
/*
The 6 wheel buttons; Cross,Circle,Square,Triangle,L2,R2 have a resistance of 17kohm not pressed or ~5kohm pressed
This causes intermittent detection when using digital IO.  Therefore, reassign those 6 buttons to use the 6 remaining
analog signals (the ones NOT used by wheel, accel, and brake)
Start, Select, D-Pad, and shifter buttons are all open or closed momentary switches so they are OK for digital
Pin   Function      Wire Color
D0    R3 (paddle push) BRN/WHT
D1    L3 (paddle push) YEL
D2    Right paddle  PNK
D3    Left paddle   GRN/WHT
D4/A6 Cross         GRY  was D16
D5    Shift Down    BLU(2)
D6/A7 Square        WHT   was D15
D7    DDN           YEL/BLK
D8/A8 DL            ORG/WHT
D9/A9 DR            RED/WHT
D10/A10 Circle      GRN   was D14 Messed this one up so I converted it to open or ~600ohms so it should work as digital
D16   Shift Up      GRN(2) was D4/A6
D14   Start         RED was D10/A10
D15   DUP           PNK/BLK  was D6/A7
D18/A0 Triangle      BLU      OK as A0
A1    Accel         BRN(2)
A2    Brake         RED(2)
A3    Steering      WHT(3)

Unused
L2  PUR
R2  BLK
CAL           ORG/WHT(J5)
*/
#define PADDLE_R_PUSH 0
#define PADDLE_L_PUSH 1
#define PADDLE_R  2
#define PADDLE_L  3
#define CROSS    A6 //D4
#define SHIFT_DN  5
#define SQUARE   A7 //D6
#define DDN       7
#define DLT       8
#define DRT       9
#define START    14
#define SHIFT_UP 16
#define CIRCLE   A10 //D10
#define DUP      15
#define TRIANGLE A0 //D18
#define ACCEL    A1
#define BRAKE    A2
#define WHEEL    A3
#define MAX_NUM_BUTTONS 11

// Default values taken from the first calibration performed
#define STEERING_LEFT_DEFAULT 0
#define STEERING_RIGHT_DEFAULT 995
#define STEERING_CENTER_DEFAULT 496
#define STEERING_DEADBAND_DEFAULT 50
#define ACCEL_MIN_DEFAULT 0
#define ACCEL_MAX_DEFAULT 758
#define BRAKE_MIN_DEFAULT 0
#define BRAKE_MAX_DEFAULT 780

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  MAX_NUM_BUTTONS, 4,                  // Button Count, Hat Switch Count
  false, false, false,   // no X and no Y, no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  true, true, true);     // accelerator, brake, and steering

// Set to true to test "Auto Send" mode or false to test "Manual Send" mode.
//const bool testAutoSendMode = true;
const bool testAutoSendMode = false;
int button_threshold = 205;

struct caltype
{
int steering_left = STEERING_LEFT_DEFAULT;
int steering_right = STEERING_RIGHT_DEFAULT;
int steering_center = STEERING_CENTER_DEFAULT;
int steering_db = STEERING_DEADBAND_DEFAULT;
int accel_min = ACCEL_MIN_DEFAULT;
int accel_max = ACCEL_MAX_DEFAULT;
int brake_min = BRAKE_MIN_DEFAULT;
int brake_max = BRAKE_MAX_DEFAULT;
} wheelcal;

int _dpad_switch[4]={DUP,DRT,DDN,DLT};
int lastButtonState[4] = {0,0,0,0};
int buttonState[MAX_NUM_BUTTONS];
int buttonGPIO[MAX_NUM_BUTTONS] = {PADDLE_L,PADDLE_R,SHIFT_UP,SHIFT_DN,START,CIRCLE};

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
  buttonState[0] = !digitalRead(PADDLE_L);
  buttonState[1] = !digitalRead(PADDLE_R);
  buttonState[2] = !digitalRead(CIRCLE);
  // The last 3 buttons need to be read analog (CIRCLE should be too, but I "fixed" it)
  // Interal pullup 20-50k, open button 17k, closed 6k
  // open voltage  = 1.48V -> 17x5/1.48 - 17 = R(pullup) = 40.4k
  // closed voltage = 6/46 x 5 = 0.652V
  // Actual measurement closed voltage ~ 0.8 (use 1.0V)
  // 10bit DAC = 1023 -> 1/5 * 1023 = 205
  buttonState[3] = (analogRead(CROSS)<button_threshold);
  buttonState[4] = (analogRead(TRIANGLE)<button_threshold);
  buttonState[5] = (analogRead(SQUARE)<button_threshold);
  buttonState[6] = !digitalRead(PADDLE_L_PUSH);
  buttonState[7] = !digitalRead(PADDLE_R_PUSH);
  buttonState[8] = !digitalRead(SHIFT_UP);
  buttonState[9] = !digitalRead(SHIFT_DN);
  buttonState[10] = !digitalRead(START);
  for(int i = 0; i< MAX_NUM_BUTTONS; i++)
  {
    Joystick.setButton(i,buttonState[i]);
  }
}

void show_menu()
{
  Serial.println(F("\n    Calibration menu"));
  Serial.println(F("1. Steering wheel min/max/center"));
  Serial.println(F("2. Accelerator min/max"));
  Serial.println(F("3. Brake min/max"));
  Serial.println(F("4. Steering wheel deadband setting"));
  Serial.println(F("5. Reset all values to defaults"));
  Serial.println(F("0. quit cal mode and save values to EEPROM"));
  Serial.println(F("q. Quit and do not save\n"));
  Serial.print(F("You have "));
  Serial.print(TIMEOUT_HALF_SECONDS/2);
  Serial.println(F(" seconds to make a selection"));
}

void steering_cal()
{
  Serial.println(F("Hold the steering wheel all the way to the LEFT then press 'm' to measure"));
  while(Serial.read()!='m') delay(500);
  wheelcal.steering_left = analogRead(WHEEL);
  Serial.print(F("steering_left = "));
  Serial.println(wheelcal.steering_left);
  
  Serial.println(F("Hold the steering wheel all the way to the RIGHT then press 'm' to measure"));
  while(Serial.read()!='m') delay(500);
  wheelcal.steering_right = analogRead(WHEEL);
  Serial.print(F("steering_right = "));
  Serial.println(wheelcal.steering_right);

  Serial.println(F("Hold the steering wheel in the CENTER then press 'm' to measure"));
  while(Serial.read()!='m') delay(500);
  wheelcal.steering_center = analogRead(WHEEL);
  Serial.print(F("steering_center = "));
  Serial.println(wheelcal.steering_center);

  Serial.println(F("Press any key to continue"));
  while(!Serial.available()) delay(500);
}

void steering_deadband()
{
  int deadband_new = 0;
  Serial.setTimeout(10*1000);
  Serial.println(F("Enter the value for the steering deadband (2-100)"));
  Serial.print(F("(full range of steering values is 0 to 1023)\nCurrent value: "));
  Serial.print(wheelcal.steering_db);
  deadband_new = Serial.readStringUntil('\n').toInt();
  if(deadband_new>=2 && deadband_new<=100)
    wheelcal.steering_db = deadband_new;
  Serial.print(F("\ndeadband = "));
  Serial.println(wheelcal.steering_db);
}

int cosine_scaling(int input_val)
{
  float input_angle,cos_val;
  float bottom_range, top_range;
  if(input_val<=wheelcal.steering_center) // we are between min and center
  {
    // input_angle = 90*input_val/(Center-Min)-90
    bottom_range = wheelcal.steering_center-wheelcal.steering_left;
    input_angle = 90*float(input_val)/bottom_range-90;
    cos_val = cos(input_angle*PI/180)*(bottom_range);
    #if DEBUG
    Serial.print(F(" input_val = "));
    Serial.print(input_val,DEC);
    Serial.print(F(" bottom_range = "));
    Serial.print(bottom_range,DEC);
    Serial.print(F(" input_angle = "));
    Serial.print(input_angle,DEC);
    Serial.print(F(" cos_val = "));
    Serial.print(cos_val,DEC);
    Serial.print(" \n");
    #endif
  }
  else // we are between center+1 and max
  {
    // input_angle = 90*(input_val-Center)/(Max-Center)
    // Cos_ratio_high = 1-COS(Input_Angle*PI()/180)
    // cos_val = Cos_ratio_high*(Max-Center)+Center
    top_range = wheelcal.steering_right-wheelcal.steering_center;
    input_angle = float(input_val-wheelcal.steering_center)*90/top_range;
    cos_val = ((1-cos(input_angle*PI/180))*top_range) + wheelcal.steering_center;
    #if DEBUG
    Serial.print(F(" input_val = "));
    Serial.print(input_val,DEC);
    Serial.print(F(" top_range = "));
    Serial.print(top_range,DEC);
    Serial.print(F(" input_angle = "));
    Serial.print(input_angle,DEC);
    Serial.print(F(" cos_val = "));
    Serial.print(cos_val,DEC);
    Serial.print(" \n");
    #endif
  }
  return int(cos_val);
}

void accel_cal()
{
  Serial.println(F("Ensure the accelerator pedal is not depressed then press 'm' to measure"));
  while(Serial.read()!='m') delay(500);
  wheelcal.accel_min = analogRead(ACCEL);
  Serial.print(F("accel_min = "));
  Serial.println(wheelcal.accel_min);
  
  Serial.println(F("Fully depress the accelerator pedal then press 'm' to measure"));
  while(Serial.read()!='m') delay(500);
  wheelcal.accel_max = analogRead(ACCEL);
  Serial.print(F("accel_max = "));
  Serial.println(wheelcal.accel_max);

  Serial.println(F("Press any key to continue"));
  while(!Serial.available()) delay(500);
}

void brake_cal()
{
  Serial.println(F("Ensure the brake pedal is not depressed then press 'm' to measure"));
  while(Serial.read()!='m') delay(500);
  wheelcal.brake_min = analogRead(BRAKE);
  Serial.print(F("brake_min = "));
  Serial.println(wheelcal.brake_min);
  
  Serial.println(F("Fully depress the brake pedal then press 'm' to measure"));
  while(Serial.read()!='m') delay(500);
  wheelcal.brake_max = analogRead(BRAKE);
  Serial.print(F("brake_max = "));
  Serial.println(wheelcal.brake_max);

  Serial.println(F("Press any key to continue"));
  while(!Serial.available()) delay(500);
}

void read_cal()
{
  // read the values out of EEPROM
  caltype temp_cal;
  EEPROM.get(0,temp_cal);
  // range check each setting. Only update the ones containing valid values
  #define ONE_THIRD_RANGE 341
  #define TWO_THIRD_RANGE 682
  #define FULL_RANGE 1023
  #define ONE_TENTH_RANGE 100

  if(temp_cal.steering_left>=0 && temp_cal.steering_left<=(ONE_THIRD_RANGE))
    wheelcal.steering_left = temp_cal.steering_left;
  if(temp_cal.steering_right>=TWO_THIRD_RANGE && temp_cal.steering_right<=FULL_RANGE)
    wheelcal.steering_right = temp_cal.steering_right;
  if(temp_cal.steering_center>=ONE_THIRD_RANGE && temp_cal.steering_center<=TWO_THIRD_RANGE)
    wheelcal.steering_center = temp_cal.steering_center;
  if(temp_cal.steering_db>=2 && temp_cal.steering_db<=ONE_TENTH_RANGE)
    wheelcal.steering_db = temp_cal.steering_db;

  if(temp_cal.accel_min>=0 && temp_cal.accel_min<=ONE_THIRD_RANGE)
    wheelcal.accel_min = temp_cal.accel_min;
  if(temp_cal.accel_max>=TWO_THIRD_RANGE && temp_cal.accel_max<=FULL_RANGE)
    wheelcal.accel_max = temp_cal.accel_max;

  if(temp_cal.brake_min>=0 && temp_cal.brake_min<=ONE_THIRD_RANGE)
    wheelcal.brake_min = temp_cal.brake_min;
  if(temp_cal.brake_max>=TWO_THIRD_RANGE && temp_cal.brake_max<=FULL_RANGE)
    wheelcal.brake_max = temp_cal.brake_max;
}

void save_cal()
{
  // save current values to EEPROM
  EEPROM.put(0,wheelcal);
}

void print_cal()
{
  Serial.println(F("\nCurrent calibration values:"));
  Serial.print(F("steering_left = "));
  Serial.println(wheelcal.steering_left);
  Serial.print(F("steering_right = "));
  Serial.println(wheelcal.steering_right);
  Serial.print(F("steering_center = "));
  Serial.println(wheelcal.steering_center);
  Serial.print(F("steering_deadband = "));
  Serial.println(wheelcal.steering_db);
  Serial.print(F("accel_min = "));
  Serial.println(wheelcal.accel_min);
  Serial.print(F("accel_max = "));
  Serial.println(wheelcal.accel_max);
  Serial.print(F("brake_min = "));
  Serial.println(wheelcal.brake_min);
  Serial.print(F("brake_max = "));
  Serial.println(wheelcal.brake_max);

}

void reset_cal()
{
  wheelcal.steering_left = STEERING_LEFT_DEFAULT;
  wheelcal.steering_right = STEERING_RIGHT_DEFAULT;
  wheelcal.steering_center = STEERING_CENTER_DEFAULT;
  wheelcal.steering_db = STEERING_DEADBAND_DEFAULT;
  wheelcal.accel_min = ACCEL_MIN_DEFAULT;
  wheelcal.accel_max = ACCEL_MAX_DEFAULT;
  wheelcal.brake_min = BRAKE_MIN_DEFAULT;
  wheelcal.brake_max = BRAKE_MAX_DEFAULT;
  Serial.println(F("Calibration values set back to defaults"));
  //save_cal();
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
          case '5':
            reset_cal();
            break;
          case '0':
            // save to EEPROM
            Serial.println(F("Done calibration. Saving values to EEPROM"));
            save_cal();
            done = true;
            break;
          case 'q':
            Serial.println(F("Exiting calibration mode.  Values NOT saved to EEPROM"));
            done = true;
            break;
        }
      }
      else // timeout
        Serial.println(F("\nTimeout.  Exiting calibration mode.  Values NOT saved to EEPROM"));

      done |= timeout;
    }
  }
}

void setup() {

  Joystick.begin(testAutoSendMode);
  Serial.begin(115200);

  pinMode(PADDLE_R_PUSH,      INPUT_PULLUP);
  pinMode(PADDLE_L_PUSH,     INPUT_PULLUP);
  pinMode(PADDLE_R, INPUT_PULLUP);
  pinMode(PADDLE_L, INPUT_PULLUP);
  pinMode(SHIFT_UP, INPUT_PULLUP);
  pinMode(SHIFT_DN, INPUT_PULLUP);
  pinMode(DUP,      INPUT_PULLUP);
  pinMode(DDN,      INPUT_PULLUP);
  pinMode(DLT,      INPUT_PULLUP);
  pinMode(DRT,      INPUT_PULLUP);
  pinMode(START,    INPUT_PULLUP);
  pinMode(CIRCLE,   INPUT_PULLUP);
  pinMode(CROSS,    INPUT_PULLUP); // analog
  pinMode(SQUARE,   INPUT_PULLUP); // analog
  pinMode(TRIANGLE, INPUT_PULLUP); // analog
  pinMode(LED_BUILTIN, OUTPUT);

  read_cal();
  if(Serial)
  {
    Serial.println(F("calibration read from EEPROM"));
    // apply calibration
    Serial.println(F("Applying calibration"));
  }
  Joystick.setAcceleratorRange(wheelcal.accel_min,wheelcal.accel_max);
  Joystick.setBrakeRange(wheelcal.brake_min,wheelcal.brake_max);
  Joystick.setSteeringRange(wheelcal.steering_left,wheelcal.steering_right);

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

int _accel, raw_accel;
int _brake, raw_brake;
int _wheel,raw_wheel, new_wheel;
int accel_samples_buff[ACCEL_FILTER_SAMPLES];
int brake_samples_buff[BRAKE_FILTER_SAMPLES];
int wheel_samples_buff[WHEEL_FILTER_SAMPLES];
float accel_scaling_value = 1.2;

bool scanmode = false;

uint8_t i;
unsigned long startmsec, elapsedmsec;
int loopcounter=0;

void loop() 
{
  #if TIMESTUDY
  if(loopcounter==0)
    startmsec = millis();
  if(++loopcounter>NUMLOOPS)
  {
    elapsedmsec = millis() - startmsec;
    if(Serial)
    {
      Serial.print(NUMLOOPS);
      Serial.print(" loops took ");
      Serial.print(elapsedmsec/NUMLOOPS);
      Serial.println(" milliseconds per loop");
    }
    loopcounter = 0;
  }
  #endif
  raw_accel = analogRead(ACCEL);
  #if ACCELAVG
  accel_samples_buff[num_accel_samples++] = raw_accel;
  // calculate a moving average for accelerator
  if( num_accel_samples >= ACCEL_FILTER_SAMPLES)
    num_accel_samples = 0;
  _accel_sum = 0;
  for( i = 0; i < ACCEL_FILTER_SAMPLES; i++ )
    _accel_sum += accel_samples_buff[i];
  _accel = _accel_sum / ACCEL_FILTER_SAMPLES;
  #else
  _accel = raw_accel;
  #endif
  #if ACCELSCALING
  _accel = int(float(_accel) * accel_scaling_value);
  #endif

  raw_brake = analogRead(BRAKE);
  #if BRAKEAVG
  brake_samples_buff[num_brake_samples++] = raw_brake;
  // calculate a moving average for brake
  if( num_brake_samples >= BRAKE_FILTER_SAMPLES)
    num_brake_samples = 0;
  _brake_sum = 0;
  for( i = 0; i < BRAKE_FILTER_SAMPLES; i++ )
    _brake_sum += brake_samples_buff[i];
  _brake = _brake_sum / BRAKE_FILTER_SAMPLES;
  #else
  _brake = raw_brake;
  #endif

  raw_wheel = analogRead(WHEEL);
  #if COSINE_SCALING
  new_wheel = cosine_scaling(raw_wheel);
  #else
  new_wheel = raw_wheel;
  #endif
  #if WHEELAVG
  wheel_samples_buff[num_wheel_samples] = new_wheel;
  num_wheel_samples++;
  // calculate a moving average for steering wheel
  if( num_wheel_samples >= WHEEL_FILTER_SAMPLES)
    num_wheel_samples = 0;
  _wheel_sum = 0;
  for( i = 0; i < WHEEL_FILTER_SAMPLES; i++ )
    _wheel_sum += wheel_samples_buff[i];
  _wheel = _wheel_sum / WHEEL_FILTER_SAMPLES;  
  #else
  _wheel = raw_wheel;
  #endif
  #if DEADBAND
  // if wheel value is in the deadband (center +/- half of deadband), set it to wheelcal.steering_center
  if(_wheel>=(wheelcal.steering_center-wheelcal.steering_db/2) && _wheel<=(wheelcal.steering_center+wheelcal.steering_db/2))
    _wheel = wheelcal.steering_center;
  #endif

  Joystick.setAccelerator(_accel);
  Joystick.setBrake(_brake);
  Joystick.setSteering(_wheel);

  read_buttons();
  read_DPAD();

  if (testAutoSendMode == false)
  {
    Joystick.sendState();
  }
  #if ENABLESERIAL
  if(Serial.available())
  {
    switch(Serial.read())
    {
      case 'c':
        do_analog_cal();
        // apply calibration
        Serial.println(F("Applying calibration"));
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
        Serial.println(F("c - calibrate\np - print cal values\ns - analog scan mode\nh - this help screen\na - about this software"));
        break;
      case 'a':
        Serial.println(F("\nMadCatz MC2 USB Conversion Firmware\nfor Arduino Pro Micro (Atmega32U4)\nCopyright 2020 Cam Strandlund\n"));
        break;
    }
  }
  if(scanmode)
  {
    Serial.print(F("Average: "));
    Serial.print(_accel);
    Serial.print(F(","));
    Serial.print(_brake);
    Serial.print(F(","));
    Serial.print(_wheel);
    #if COSINE_SCALING
    Serial.print(F(" (cosine scaled)"));
    #else
    // show what the cosine scaling "would" do
    Serial.print(F(" cosine_scaling="));
    Serial.print(cosine_scaling(wheel_samples_buff[num_wheel_samples]),DEC);
    #endif
    Serial.print(F("  Raw: "));
    Serial.print(raw_accel);
    Serial.print(F(","));
    Serial.print(raw_brake);
    Serial.print(F(","));
    Serial.print(raw_wheel);
    Serial.print(F("  "));
    for(int i = 0; i< MAX_NUM_BUTTONS; i++)
    {
      Serial.print(buttonState[i]);
    }
    Serial.print(F("                   \r"));

  }
  #endif

#if DEBUG
  delay(500);
#else
  delay(50);
#endif
}