
//////////////////////////////////////////LIBRARIES/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <RunningAverage.h>
#include <DFRobot_BME680_I2C.h>
#include <Wire.h>
#include "esp_attr.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_LIS2MDL.h>
#include <AS5600.h>
#define SERIAL_SIZE_TX  2048


////libraries stuff
Adafruit_LIS2MDL mag = Adafruit_LIS2MDL(12345);
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

/////////////////////////STEERING ANGLE///////////////////////////////////////////////////

AS5600 as5600; 

////////////////////////////////////////Libraries initialization////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RunningAverage RPM_L_RA(60);
RunningAverage RPM_R_RA(60);
RunningAverage RPM_C_RA(60);
RunningAverage RPM_CRANK_RA(60);
RunningAverage RPM_SHAFT_RA(60);

//////////////////////////////////////////////I2C sensors///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DFRobot_BME680_I2C bme(0x77);  //I2C BME680 ID

/////////////////////////////////////////////////VARIABLES///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////SPEED/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long time_l;            // Calculates the time between each sector of the wheel. Time of the magnets in the left wheel
unsigned long time_r;            // Calculates the time between each sector of the wheel. Time of the magnets in the right wheel 
unsigned long time_c;          // Calculates the time between each sector of the wheel. Time of the magnets in the centre wheel 
unsigned long time_cr;           // Calculates the time between each sector of the wheel. Time of the magnets in the crank wheel 
unsigned long time_shaft;            // Calculates the time between each sector of the wheel. Time of the magnets in the centre wheel 
unsigned long time_output;
unsigned long prev_l;            //prev_ls the time of the first previous time the magnet was passed through. Time of the magnets in the left wheel 
unsigned long prev_r;           // prev_ls the time of the first previous time the magnet was passed through. Time of the magnets in the right wheel
unsigned long prev_c;          // prev_ls the time of the first previous time the magnet was passed through. Time of the magnets in the centre wheel
unsigned long prev_cr;         // prev_ls the time of the first previous time the magnet was passed through. Time of the magnets in the crank wheel
unsigned long prev_shaft;         // prev_ls the time of the first previous time the magnet was passed through. Time of the magnets in the crank wheel
unsigned long prev_output;
float RPM_L;                     // RPM of the left wheel
float RPM_R;                    // RPM of the right wheel 
float RPM_C;                   // RPM of the centre wheel
float RPM_CRANK;              // RPM of the crank wheel
float RPM_SHAFT;              // RPM of the gear shaft
float average_rpm;            // Average rpm of 3 wheels
float total_speed;           // total speed if the bike 
float some_constant_from_gear_ratio;  // placeholder for gearratio constant
int samples = 0; 
int x = 0; 
int priority_wheel_int = 1;
struct ints_struct{
  int left;
  int right;
  int center;
  int shaft;
  int crank;
  int priority;
};


/////////////////////////////////////////////////STEERING ANGLE VARIABLES///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float steering_angle;
float steering_angle_max;
float steering_angle_min;
float steering_angle_center;

///////////////////////////////////////////////////AIR QUALITY//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float seaLevel;              // SeaLevel value 
float Temperature;          // Temperature value 
float Humidity;            // Humidity value 
float Pressure;           // Pressure value
unsigned long lastRead=0;

////////////////////////////////////////////////////////PINS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int hall_pin = 12;         // Pin relates to wheel left 
const int hall_pin2 = 13;        // Pin relates to wheel right 
const int hall_pin3 = 14;      // Pin relates to wheel centre 
const int hall_pin4 = 27;      // Pin relates to crank
const int hall_pin5 = 26;      // Pin relates to gear shaft

//////////////////////////////////////////////////// TWEAK variables///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float wheel_circumference = 2.777; //Circumference of wheel 
const int Magnet_Number = 8; // Number of magnets on the tone wheel
const int print_frequency = 5; //milliseconds between serial.print

////////////////////////////////////////////INTERRUPT FUNCTIONS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 



////////////////////////////////////////////////////SETUP//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void setup() {
  Serial.begin(115000);
  Serial.setTxBufferSize(SERIAL_SIZE_TX);
  //Serial2.begin(115200, SERIAL_8N1, 16, 17);

///////////////////////////////////////////TEMPERATURE SETUP/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
   uint8_t rslt = 1;
   while(!Serial);
   rslt = bme.begin();
   Serial.println("BME WORKING");
   bme.startConvert();
   bme.update();

///////////////////////////////////////////magnetometer/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// //magnetometer
//   mag.begin();
//   mag.enableAutoRange(true);
//   //accelerometer
//   accel.begin();

//  //////////////////////////////////////STEERING ANGLE SETUP/////////////////////////////////////////
//  ////////////////////////////////////////////////////////////////////////////////////

// }
}
///////////////////////////////////////////MAIN////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  

  
   sensors_event_t event; //declare event

   accel.getEvent(&event); //capture accelerometer data

     if(millis()-lastRead>1000){
       bme.startConvert();
       bme.update();
       Temperature=bme.readTemperature() / 100, 2;
       Humidity=bme.readHumidity() / 1000, 2;
       Pressure=bme.readPressure();
       lastRead=millis();
      }

       mag.getEvent(&event); //capture magnetometer data

    //CALCULATE HEADING from magnetometer data
     float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / 3.14159;
      if (heading < 0)
      {
      heading = 360 + heading;
    }
  
  //////////////////////////////////////////////Print to screen///////////////////////////////////////////////////////////////////////////
  // order in list should always go (left right center crank shaft)
  time_output = millis()-prev_output;
   if (time_output > print_frequency){
   prev_output = millis();
   Serial.print("'ax':");
   Serial.print(event.acceleration.x, 2); 
   Serial.print(",");
   Serial.print("'ay':");
   Serial.print(event.acceleration.y, 2); 
   Serial.print(",");
   Serial.print("'az':");
   Serial.print(event.acceleration.z, 2); 
   Serial.print(",");
   //print magnetometer
   Serial.print("'vx':");
   Serial.print(event.magnetic.x, 2);
   Serial.print(","); 
   Serial.print("'vy':");
   Serial.print(event.magnetic.y, 2); 
   Serial.print(",");
   Serial.print("'vz':");
   Serial.print(event.magnetic.z, 2);
   Serial.print(",");
   //print environment
   Serial.print("'t':");
   Serial.print(Temperature, 2);
   Serial.print(",");
   Serial.print("'h':");
   Serial.print(Humidity, 2);
   Serial.print(",");
   Serial.print("'p':");
   Serial.println(Pressure, 2);
  }
}
