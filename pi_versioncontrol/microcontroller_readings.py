import serial
import ast

class SensorDataProcessor:
    """Takes data from all the sensors and packages them for either data logging, 
    screen display, or to send back to microcontrollers"""

    def __init__(self) -> None:
        # set all expected data as class instance attributes
        self.center_wheel_rpm = 0
        self.left_wheel_rpm = 0
        self.right_wheel_rpm = 0
        self.crank_rpm = 0
        self.shaft_rpm = 0
        self.total_speed = 0
        self.gear_set = 0
        self.steering_angle = 0
        self.acceleration_x = 0
        self.acceleration_y = 0
        self.acceleration_z = 0
        self.angular_velocity_x = 0
        self.angular_velocity_y = 0
        self.angular_velocity_z = 0
        self.temperature = 0
        self.air_pressure = 0
        self.humidity = 0
        self.latitude = 0
        self.longitude = 0
        self.distance_to_finish = 0

        # initialize expected data as a list for comparison
        self.expected_data = ["center_wheel_rpm","left_wheel_rpm","right_wheel_rpm","crank_rpm",
                              "shaft_rpm","total_speed","gear_set","steering_angle","acceleration_x",
                              "acceleration_y","acceleration_z","angular_velocity_x","angular_velocity_x",
                              "angular_velocity_y","angular_velocity_z","temperature","air_pressure",
                              "humidity","latitude","longitude","distance_to_finish"]

        # initialize serial read and expected ports
        try: 
            self.esphatch = serial.Serial('/dev/ttyUSB1', 115200, timeout=1)
            self.esphatch.reset_input_buffer()
        except:
            self.esphatch = "none"
        
        try:
            self.espbike = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
            self.espbike.reset_input_buffer()
        except:
            self.espbike = "none"
            
    

    def format_data(self):
        """This method reads the serials and joins them together as a string,
        before returning as a dictionary"""
        try:
            esp_bike_raw = self.espbike.readline().decode('utf-8').rstrip()
        except:
            esp_bike_raw = "'data': 'none'"
        try:
            esp_hatch_raw = self.esphatch.readline().decode('utf-8').rstrip()
        except:
            esp_hatch_raw = "'data': 'none'"
            
        data_string = ''.join(("{",esp_bike_raw,",",esp_hatch_raw,"}"))
        data_dict = ast.literal_eval(data_string)
        return data_dict
    
    def update_attributes(self, data_as_dict):
        """This method updates the class instance attributes based on the formated dictionary.
        Additionally, it registers the attribute as 'unavailable' if the sensor fails to send it
        to the pi."""
        available_data = []
        for key in data_as_dict:
            setattr(self, key, data_as_dict[key])
            available_data.append(key)
        unavailable_data = list(set(available_data).symmetric_difference(set(self.expected_data)))
        if len(unavailable_data) > 0:
            unavailable_data_as_dict= dict.fromkeys(unavailable_data, "unavailable")
            for key in unavailable_data_as_dict:
                setattr(self, key, unavailable_data_as_dict[key])
    
    def process(self):
        """This method runs the update_attributes and format_data methods in the appropriate order,
        then returns a dictionary of the current class instance attributes related to sensor data."""
        self.update_attributes(self.format_data())
        current_data = dict(self.__dict__)
        current_data.pop("expected_data")
        current_data.pop("esphatch")
        current_data.pop("espbike")

        return current_data

