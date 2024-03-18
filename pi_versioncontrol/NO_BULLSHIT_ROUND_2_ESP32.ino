#include <Adafruit_GPS.h>


// what's the name of the hardware serial port?
#define GPSSerial Serial2 // fuck you peice of shit 

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO true

uint32_t timer = millis();

 double haversine(double lat1, double lon1, double lat2,double lon2) { 
    double dLat = (lat2 - lat1) * (3.1415926/180);
    double dLon = (lon2 - lon1)  * (3.1415926/180);
    double a = (sin(dLat / 2.0) * sin(dLat / 2.0)) + (cos(lat1 *   (3.1415926/180)) * cos(lat2  * (3.1415926/180)) *sin(dLon / 2.0) * sin(dLon / 2.0));
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    double R = 6371000.0; // Earth's radius in meters
    double distance = R * c; // Distance in meters
    return distance;


 }

double convertToDecimalMin(double curReading){
  double a = curReading/100; 
  int modulus = static_cast<int>(a);
  double b = modulus *100;
  double DecMinutes = curReading - b; 
  double c = DecMinutes/60;

  return modulus + c; 
  
  
}

void setup()
{
  //while (!Serial);  // uncomment to have the sketch wait until Serial is ready

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  //Serial.println("Adafruit GPS library basic parsing test!");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);
}

void loop() // run over and over again
{
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
   
    Serial.print(GPS.lastNMEA()); 
    if (!GPS.parse(GPS.lastNMEA())) 
      return; 
  }

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
      Serial.print("Antenna status: "); Serial.println((int)GPS.antenna);


/*
 * the latitide and longitude that they give to you is within the format of 4042.6142 -->40 degrees and 42.6142 decimal minutes 
 * what we want it is to go from degree and decimal minutes --> decimal degree 
 * 
 * 
 */

      
 double lat2 = 33.783516; // Destination latitude
 double lon2 = 151.125982; // Destination longitude
  
  double lat1 = convertToDecimalMin(GPS.latitude); // Current latitude
  double lon1 = convertToDecimalMin(GPS.longitude); // Current longitude

Serial.println(lat1); 

Serial.println(lon1); 


    double dist = haversine(lat1, lon1, lat2, lon2);
    Serial.print("Distance to destination: ");
    Serial.print(dist);
    Serial.println(" meters");




      
    }
  }
}
