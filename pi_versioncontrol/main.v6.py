#//////////////////////////////IMPORTING LIBRARIES////////////////////
#////////////////////////////////////////////////////////////////////

import RPi.GPIO as GPIO
from picamera import PiCamera
import time  
from datetime import datetime
import datetime as dt
import csv
from microcontroller_readings import SensorDataProcessor

#///////////////////////////////////SETUP////////////////////////////////////////////
#//////////////////////////////////////////////////////////////////////////////

GPIO.setmode(GPIO.BOARD) # Use physical pin numbering
GPIO.setup(7, GPIO.IN, pull_up_down=GPIO.PUD_DOWN) # Set pin 10 to be an input pin and set initial value to be pulled low (off)

powerstate = False
f=open('/home/pi/Desktop/Saves/Sensors/Test_' + str(datetime.now().strftime('%Y_%m_%d_%H_%M_%S')) + '.csv', 'w')
writer = csv.writer(f)

sensor_data_processor = SensorDataProcessor()

#////////////////////////////VARIABLES/////////////////////////////////////
#//////////////////////////////////////////////////////////////////////////////

t = 1800
temp_speed = 0
max_speed = 0
distance = 8000
cadance = 0
timeout=0;

#///////////////////////CAMERA SETUP/////////////////////////////////
#///////////////////////////////////////////////////////////////////

camera = PiCamera()
camera.rotation = 180

#///////////////////////////////////MAIN////////////////////////////////////
#///////////////////////////////////////////////////////////////////////////

while True:
    if GPIO.input(7) == GPIO.HIGH and powerstate == False :
        
        #/////////////////////////////FILE SETUP/////////////////////////////////////////////
        #/////////////////////////////////////////////////////////////////////////////////
        
        f=open('/home/pi/Desktop/Saves/Sensors/Test_' + str(datetime.now().strftime('%Y_%m_%d_%H_%M_%S')) + '.csv', 'w')
        camera.start_recording('/home/pi/Desktop/Camera Videos /Vid_ ' + str(datetime.now().strftime('%Y_%m_%d_%H_%M_%S')) + '.h264')
        writer = csv.writer(f)
        time_start = time.time()
        
        #/////////////////////////Starting Camera and time/////////////////////////////// 
        #///////////////////////////////////////////////////////////////////////////////
        
        camera.start_preview()
        camera.annotate_background = False
        camera.annotate_text_size = 32
        start = dt.datetime.now()
        powerstate = True
        
    if GPIO.input(7) == GPIO.HIGH:
        
        #/////////////////////////////DATALOGING////////////////////////////////////
        #///////////////////////////////////////////////////////////////////////////
        
        now = datetime.now()
        current_time = now.strftime("%H:%M:%S")
        writer.writerow([str(sensor_data_processor.process())])
       
       #///////////////////////////MAIN BIKE CALCULATIONS///////////////////////////
       #////////////////////////////////////////////////////////////////////////////
            
        gearratio = [0.1354,0.1185,0.1058,0.0931,0.0847,0.0804,0.0677]
        try:
            cadance = float(sensor_data_processor.crank_rpm)  * gearratio[sensor_data_processor.gear_set-1]
        except ValueError:
            cadance = "unavailable"
        #///////////////////////////////////OVERLAY///////////////////////////////////////
        #///////////////////////////////////////////////////////////////////////////////

        Cam_Speed = str(sensor_data_processor.total_speed)
        Cam_Gear = str(sensor_data_processor.gear_set)
        Cam_distance = str(distance)
        
        #//////////////////////MODES FOR THE HUD//////////////////////////////////////////////////
        #////////////////////////////////////////////////////////////////////////////////////////
        
        # i = ' Speed: ' + Cam_Speed + ' Gear: '+ Cam_Gear +'                            '+ ' Temp: '+Cam_Temp+'                                               '+' Humidity: '+Cam_Humd+'                                          '+' Pressure: '+Cam_Press
        # j = ' Gear: '+ Cam_Gear +'                                     '+ ' Temp: '+Cam_Temp+'                                               '+' Humidity: '+Cam_Humd+'                                          '+' Max Speed: '+Cam_MaxSpD 
        z = 'Cadance: ' +str(sensor_data_processor.crank_rpm) + '     ' +    ' RPM: ' + str(sensor_data_processor.total_speed) +'     '+ 'Distance: '+str(sensor_data_processor.distance_to_finish)
        # x = ' Cadance: ' +str(arduino_readings.main.crank) +     ' RPM: ' + str(arduino_readings.main.wheel_centre) + '           '+arduino_readings.main.error_response +'            '+ str(arduino_readings.main.lat)
        #DO NOT TOUCH 28  or 48 spaces for test to be at the left hand and right hand corner of the screeen 
        
        #///////////////////////SETTING THE MODE FOR THE HUD///////////////////////////////////////
        #/////////////////////////////////////////////////////////////////////////////////////////
        
        camera.annotate_text = z
        
        
    if GPIO.input(7) == GPIO.LOW and powerstate == True :
        
        #/////////////////////////SAVING AND CLOSING FILES/////////////////////////////////
        #/////////////////////////////////////////////////////////////////////////////////
        
        end_time = time.time()
        f.close()
        camera.stop_recording()        
        camera.stop_preview()
        powerstate = False
        exit()
