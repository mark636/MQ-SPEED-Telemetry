
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
const int print_frequency = 9; //milliseconds between serial.print

////////////////////////////////////////////INTERRUPT FUNCTIONS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

////////////////////////////////////////TIMING BETWEEN EACH MAGNET////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////left wheel //////////////

void IRAM_ATTR wheel_left(){
  time_l = micros() - prev_l; 
  prev_l = micros(); 
}

////////right wheel ////////////

void IRAM_ATTR wheel_right(){
  time_r = micros() - prev_r;
  prev_r = micros();
}

///////////centre wheel//////////

void IRAM_ATTR wheel_centre(){
  time_c = micros() - prev_c;
  prev_c = micros();
}

/////////crank wheel////////////

void IRAM_ATTR crank(){
  time_cr = micros() - prev_cr; 
  prev_cr = micros();
}

/////////gear shaft////////////

void IRAM_ATTR gear_shaft(){
  time_shaft = micros() - prev_shaft; 
  prev_shaft = micros();
}

// priority index: center = 1, left = 2, right = 3, all wheels broken = 4
ints_struct compare_values(float left_wheel, float right_wheel, float center_wheel, float shaft, float crank, int priority_wheel){
  int error_margin = 10;
  int left_status;
  int right_status;
  int center_status;
  int shaft_status;
  int crank_status;
  float priority_rpm;
  int new_priority_wheel;

  // choose wheel to become leader
  if(priority_wheel == 1){
    priority_rpm = center_wheel;
  }
  if(priority_wheel == 2){
    priority_rpm = left_wheel;
  }
  if(priority_wheel == 3){
    priority_rpm = right_wheel;
  }
  
  // // compare values and determine, 1 is error and 0 is working

  if(left_wheel>(priority_rpm + error_margin) || (priority_rpm - error_margin) < left_wheel){
    left_status = 1;
  } else{
    left_status = 0;
  }

  if(right_wheel>(priority_rpm + error_margin) || (priority_rpm - error_margin) < right_wheel){
    right_status = 1;
  } else{
    right_status = 0;
  }

  if(center_wheel>(priority_rpm + error_margin) || (priority_rpm - error_margin) < center_wheel){
    center_status = 1;
  } else{
    center_status = 0;
  }

  if(shaft>((priority_rpm*some_constant_from_gear_ratio) + error_margin) || (priority_rpm*some_constant_from_gear_ratio - error_margin) < shaft){
    shaft_status = 1;
  } else{
    shaft_status = 0;

  }if(crank>(priority_rpm*some_constant_from_gear_ratio + error_margin) || (priority_rpm*some_constant_from_gear_ratio - error_margin) < crank){
    crank_status = 1;
  } else{
    crank_status = 0;
  }

  if(left_status == 1 && right_status == 1 && shaft == 1 && crank == 1){
    new_priority_wheel = 2;
  } else if(center_status == 1 && right_status == 1 && shaft == 1 && crank == 1){
    new_priority_wheel = 3;
  } else if(center_status == 1 && left_status == 1 && shaft == 1 && crank == 1){
    new_priority_wheel = 4;
  } else{
    new_priority_wheel = 1;
  }

  return {left_status, right_status, center_status, shaft_status, crank_status, new_priority_wheel};
}


////////////////////////////////////////////////////SETUP//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void setup() {
  Serial.begin(115200);
  //Serial2.begin(115200, SERIAL_8N1, 16, 17);
  pinMode(hall_pin, INPUT);       //Pin initialization
  pinMode(hall_pin2, INPUT);      //Pin initialization
  pinMode(hall_pin3, INPUT);      //Pin initialization
  pinMode(hall_pin4, INPUT);      //Pin initialization
  pinMode(hall_pin5, INPUT);      //Pin initialization
  attachInterrupt(digitalPinToInterrupt(hall_pin), wheel_left, RISING); //Interrupt initialization
  attachInterrupt(digitalPinToInterrupt(hall_pin2), wheel_right, RISING); //Interrupt initialization
  attachInterrupt(digitalPinToInterrupt(hall_pin3), wheel_centre, RISING); //Interrupt initialization
  attachInterrupt(digitalPinToInterrupt(hall_pin4), crank, RISING); //Interrupt initialization
  attachInterrupt(digitalPinToInterrupt(hall_pin5), gear_shaft, RISING); //Interrupt initialization
  RPM_L_RA.clear();               // Clearing cache of the averaging for the left wheel 
  RPM_R_RA.clear();               // Clearing cache of the averaging for the right wheel
  RPM_C_RA.clear();               // Clearing cache of the averaging for the centre wheel
  RPM_CRANK_RA.clear();           // Clearing cache of the averaging for the crank wheel
  RPM_SHAFT_RA.clear();           // Clearing cache of the averaging for the crank wheel

///////////////////////////////////////////TEMPERATURE SETUP/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
  // uint8_t rslt = 1;
  // while(!Serial);
  // rslt = bme.begin();
  // Serial.println("BME WORKING");
  // bme.startConvert();
  // bme.update();

///////////////////////////////////////////magnetometer/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// //magnetometer
//   mag.begin();
//   mag.enableAutoRange(true);
//   //accelerometer
//   accel.begin();

//  //////////////////////////////////////STEERING ANGLE SETUP/////////////////////////////////////////
//  ////////////////////////////////////////////////////////////////////////////////////

//  as5600.begin(4);

// }
}
///////////////////////////////////////////MAIN////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  
  // steering_angle=as5600.rawAngle()/11.37777; //////conversion to degrees
  // if(steering_angle>180){
  //   steering_angle=-360+steering_angle;
  // }         //////setting negative angles
  // if(steering_angle>steering_angle_max){
  //   steering_angle_max=steering_angle;
  // } ///capturing max value
  // if(steering_angle<steering_angle_min){
  //   steering_angle_min=steering_angle;
  // } ///capturing min value
  // steering_angle_center=(steering_angle_max+steering_angle_min)/2; ////calculating center


  
  // sensors_event_t event; //declare event

  // accel.getEvent(&event); //capture accelerometer data
    
  
  RPM_L = (60000000.00/(time_l*Magnet_Number));          // RPM left
  RPM_R = (60000000.00/(time_r*Magnet_Number));         // RPM Right
  RPM_C = (60000000.00/(time_c*Magnet_Number));        // RPM Centre
  RPM_CRANK = (60000000.00/(time_cr*Magnet_Number));  //RPM Crank
  RPM_SHAFT = (60000000.00/(time_shaft*Magnet_Number));  //RPM Crank

  //Calculates the cadence of crank 
  //Cadence = (RPM_CRANK*wheel_circumference/16.6670)/(3.14*(diameter +(2 * tire_size)) * (chainring/cog))

  
  ///////////////////////////////////Adds value to running average/////////////////////////////////////////////////////////////////
  
  RPM_L_RA.addValue(RPM_L);             // Adds value to running average wheel left
  RPM_R_RA.addValue(RPM_R);            // Adds value to running average wheel right
  RPM_C_RA.addValue(RPM_C);           // Adds value to running average wheel centre
  RPM_CRANK_RA.addValue(RPM_CRANK);  // Adds value to runing average wheel crank
  RPM_SHAFT_RA.addValue(RPM_SHAFT); 
  average_rpm = (RPM_L_RA.getAverage() + RPM_R_RA.getAverage() + RPM_C_RA.getAverage())/3;


  ////////////////////////////////////Calculates the total Speed////////////////////////////////////////////////////////////////////////////////////////////////////////// 
  total_speed = (average_rpm*wheel_circumference)/16.6670; 

  ///////////////////////////////////Read AIR QUALITY sensor///////////////////////////////////////////////////////////////////////////////////////////////
  
    // if(millis()-lastRead>1000){
    //   bme.startConvert();
    //   bme.update();
    //   Temperature=bme.readTemperature() / 100, 2;
    //   Humidity=bme.readHumidity() / 1000, 2;
    //   Pressure=bme.readPressure();
    //   lastRead=millis();
    //  }

    //    mag.getEvent(&event); //capture magnetometer data

    // //CALCULATE HEADING from magnetometer data
    // float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / 3.14159;
    //  if (heading < 0)
    //  {
    //  heading = 360 + heading;
    // }
  
  //////////////////////////////////////////////Print to screen///////////////////////////////////////////////////////////////////////////
  // order in list should always go (left right center crank shaft)
  time_output = millis()-prev_output;
   if (time_output > print_frequency){
   prev_output = millis();
   ints_struct comparison_code = compare_values(RPM_L_RA.getAverage(), RPM_R_RA.getAverage(), RPM_C_RA.getAverage(),RPM_CRANK_RA.getAverage(),RPM_SHAFT_RA.getAverage(),priority_wheel_int);
   priority_wheel_int = comparison_code.priority;
   if(comparison_code.center==1){
    Serial.print("center wheel error");
   } else{
    Serial.print(RPM_C_RA.getAverage(),2);
   }
   Serial.print(",");
   if(comparison_code.left==1){
    Serial.print("left wheel error");
   } else{
    Serial.print(RPM_L_RA.getAverage(),2);
   }
   Serial.print(",");
   if(comparison_code.right==1){
    Serial.print("right wheel error");
   } else{
    Serial.print(RPM_R_RA.getAverage(),2);
   }
   Serial.print(",");
   if(comparison_code.crank==1){
    Serial.print("crank error");
   } else{
    Serial.print(RPM_CRANK_RA.getAverage(),2);
   }
   Serial.print(",");
   if(comparison_code.shaft==1){
    Serial.print("shaft error");
   } else{
    Serial.print(RPM_SHAFT_RA.getAverage(),2);
   }
   Serial.print(",");
   Serial.println(total_speed);




   //// update to include new compare_values() output including shaft and crank error check and priority change
  //  if(compare_values(RPM_L_RA.getAverage(), RPM_R_RA.getAverage(), RPM_C_RA.getAverage()) == 3){
  //   Serial.print("left wheel error");
  //   Serial.print(",");
  //   Serial.print("right wheel error");
  //   Serial.print(",");
  //   Serial.print(RPM_C_RA.getAverage(),2);    // Printing RPM value for centre wheel
  //   Serial.print(",");
  //   Serial.print(RPM_CRANK_RA.getAverage(),2); // Printing RPM value for crank wheel
  //   Serial.print(",");
  //   Serial.print(RPM_SHAFT_RA.getAverage(),2);     //Printing RPM value for gear shaft 
  //   Serial.print(",");
  //   Serial.println(total_speed);  // Printing total speed
  //  } else if(compare_values(RPM_L_RA.getAverage(), RPM_R_RA.getAverage(), RPM_C_RA.getAverage()) == 1){
  //   Serial.print("left wheel error");
  //   Serial.print(",");
  //   Serial.print(RPM_R_RA.getAverage(),2);     //Printing RPM value for right wheel
  //   Serial.print(",");
  //   Serial.print(RPM_C_RA.getAverage(),2);    // Printing RPM value for centre wheel
  //   Serial.print(",");
  //   Serial.print(RPM_CRANK_RA.getAverage(),2); // Printing RPM value for crank wheel
  //   Serial.print(",");
  //   Serial.print(RPM_SHAFT_RA.getAverage(),2);     //Printing RPM value for gear shaft 
  //   Serial.print(",");
  //   Serial.println(total_speed);  // Printing total speed
  //  } else{
  //   Serial.print(RPM_L_RA.getAverage(),2);     //Printing RPM value for Left wheel 
  //   Serial.print(",");
  //   Serial.print("right wheel error");     //Printing RPM value for right wheel
  //   Serial.print(",");
  //   Serial.print(RPM_C_RA.getAverage(),2);    // Printing RPM value for centre wheel
  //   Serial.print(",");
  //   Serial.print(RPM_CRANK_RA.getAverage(),2); // Printing RPM value for crank wheel
  //   Serial.print(",");
  //   Serial.print(RPM_SHAFT_RA.getAverage(),2);     //Printing RPM value for gear shaft 
  //   Serial.print(",");
  //   Serial.println(total_speed);  // Printing total speed












    // Printing total speed
    // if(RPM_L_RA.getAverage() == "inf"){
    //   Serial.print(0);
    // } else if(RPM_L_RA.getAverage()<0 || RPM_L_RA.getAverage()>10000 && ){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_L_RA.getAverage(),2);     //Printing RPM value for Left wheel 
    // }
    // Serial.print(",");
    // if(RPM_R_RA.getAverage() == "inf"){
    //   Serial.print(0);
    // } else if(RPM_R_RA.getAverage()<0 || RPM_R_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else{
    //   Serial.print(RPM_R_RA.getAverage(),2);     //Printing RPM value for right wheel
    // }
    // Serial.print(",");
    // if(RPM_C_RA.getAverage() == "inf"){
    //   Serial.print(0);
    // } else if(RPM_C_RA.getAverage()<0 || RPM_C_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_C_RA.getAverage(),2);    // Printing RPM value for centre wheel
    // }
    // Serial.print(",");
    // if(RPM_CRANK_RA.getAverage() == "inf"){
    //   Serial.print(0);
    // } else if(RPM_CRANK_RA.getAverage()<=0 || RPM_CRANK_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_CRANK_RA.getAverage(),2); // Printing RPM value for crank wheel
    // }
    // Serial.print(",");
    // if(RPM_SHAFT_RA.getAverage() == "inf"){
    //   Serial.print(0);
    // } else if(RPM_SHAFT_RA.getAverage()<0 || RPM_SHAFT_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_SHAFT_RA.getAverage(),2);     //Printing RPM value for gear shaft 
    // }
    // Serial.print(",");
    // if(total_speed == "inf"){
    //   Serial.print(0);
    // } else if(total_speed<0 || total_speed>10000){
    //   Serial.println("e");
    // } else {
    //   Serial.println(total_speed);  // Printing total speed
    // }

    // if(RPM_L_RA.getAverage()<0 || RPM_L_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_L_RA.getAverage(),2);     //Printing RPM value for Left wheel 
    // }
    // Serial.print(",");
    // if(RPM_R_RA.getAverage()<0 || RPM_R_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else{
    //   Serial.print(RPM_R_RA.getAverage(),2);     //Printing RPM value for right wheel
    // }
    // Serial.print(",");
    // if(RPM_C_RA.getAverage()<0 || RPM_C_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_C_RA.getAverage(),2);    // Printing RPM value for centre wheel
    // }
    // Serial.print(",");
    // if(RPM_CRANK_RA.getAverage()<=0 || RPM_CRANK_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_CRANK_RA.getAverage(),2); // Printing RPM value for crank wheel
    // }
    // Serial.print(",");
    // if(RPM_SHAFT_RA.getAverage()<0 || RPM_SHAFT_RA.getAverage()>10000){
    //   Serial.print("e");
    // } else {
    //   Serial.print(RPM_SHAFT_RA.getAverage(),2);     //Printing RPM value for gear shaft 
    // }
    // Serial.print(",");
    // if(total_speed<0 || total_speed>10000){
    //   Serial.println("e");
    // } else {
    //   Serial.println(total_speed);  // Printing total speed
    // }
  }
}


