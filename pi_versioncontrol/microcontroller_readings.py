import serial
import ast
import time

class SensorDataProcessor:
    """Takes data from all the sensors and packages them for either data logging, 
    screen display, or to send back to microcontrollers"""

    def __init__(self) -> None:
        # set all expected data as class instance attributes
        self.c = 0
        self.l = 0
        self.r = 0
        self.cr = 0
        self.s = 0
        self.ts = 0
        self.g = 0
        self.sa = 0
        self.ax = 0
        self.ay = 0
        self.az = 0
        self.vx = 0
        self.vy = 0
        self.vz = 0
        self.t = 0
        self.p = 0
        self.h = 0
        self.la = 0
        self.lo = 0
        self.df = 0
        self.last_log = 0

        # initialize expected data as a list for comparison
        self.expected_data = ["c","l","r","cr",
                              "s","ts","g","sa","ax",
                              "ay","az","vx",
                              "vy","vz","t","h",
                              "p","la","lo","df"]
        # legend in order : central wheel=c, left wheel=l, right wheel=r, crank=cr, shaft=s, total speed=ts, gear set=g,
        # steering angle=sa, acceleration x=ax, acceleration y=ay, acceleration z=az, angular velocity x=vx, angular velocity y=vy
        # angular velocity z=vz, temperature=t, humidity=h, air pressure=p, latitude=la, longitude=lo, distance to finish=df

        # initialize serial read and expected ports
        try: 
            self.esphatch = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
            self.esphatch.reset_input_buffer()
        except:
            self.esphatch = "none"
        
        try:
            self.espbike = serial.Serial('/dev/ttyUSB1', 115200, timeout=1)
            self.espbike.reset_input_buffer()
        except:
            self.espbike = "none"
            
    

    def format_data(self):
        """This method reads the serials and joins them together as a string,
        before returning as a dictionary"""
        while True:
            try:
                esp_bike_raw = self.espbike.readline().decode('utf-8').rstrip()
#                 if esp_bike_unfiltered[0] == "d":
#                     esp_bike_acknowledged = esp_bike_unfiltered
#                     print(f"data received by esp: {esp_bike_acknowledged}")
#                 if esp_bike_unfiltered[0] != "d":
#                     esp_bike_raw = esp_bike_unfiltered
#                     self.last_log = esp_bike_raw
#                 else:
#                     esp_bike_raw = self.last_log            
            except:
                esp_bike_raw = "'data': 'none'"
            try:
                esp_hatch_raw = self.esphatch.readline().decode('utf-8').rstrip()
            except:
                esp_hatch_raw = "'data': 'none'"
            print(esp_bike_raw)
            print(esp_hatch_raw)
            data_string = ''.join(("{",esp_bike_raw,",",esp_hatch_raw,"}"))
#             print(f"data string: {data_string}")
            
            try:
                data_dict = ast.literal_eval(data_string)
                break
            except SyntaxError:
                print("data format error")
                continue
            except ValueError:
                print("value error due to malformed node or string")
                continue
            
        return data_dict
    
    def update_attributes(self, data_as_dict):
        """This method updates the class instance attributes based on the formatted dictionary.
        Additionally, it registers the attribute as 'unavailable' if the sensor fails to send it
        to the pi."""
        available_data = []
        for key in data_as_dict:
            setattr(self, key, data_as_dict[key])
            available_data.append(key)
        unavailable_data = list(set(available_data).symmetric_difference(set(self.expected_data)))
        if len(unavailable_data) > 0:
            unavailable_data_as_dict= dict.fromkeys(unavailable_data, 0)
            for key in unavailable_data_as_dict:
                setattr(self, key, unavailable_data_as_dict[key])
    
    def process(self):
        """This method runs the update_attributes and format_data methods in the appropriate order,
        then returns a list of the current class instance attributes related to sensor data values.
        This data is then sent back to the esp32(bike) for transmission"""
        self.update_attributes(self.format_data())
        current_data = dict(self.__dict__)
        current_data.pop("expected_data")
        current_data.pop("esphatch")
        current_data.pop("espbike")
        current_data.pop("last_log")
        # testing number of data points
#         current_data.pop("c")
#         current_data.pop("l")
#         current_data.pop("r")
        current_data.pop("cr")
#         current_data.pop("s")
        current_data.pop("ts")
        current_data.pop("g")
        current_data.pop("sa")
        current_data.pop("ax")
        current_data.pop("ay")
        current_data.pop("az")
        current_data.pop("vx")
        current_data.pop("vy")
        current_data.pop("vz")
        current_data.pop("t")
        current_data.pop("p")
        current_data.pop("h")
        current_data.pop("la")
        current_data.pop("lo")
        current_data.pop("df")

        current_data_line = str(list(current_data.values()))[1:-1]
        current_data_string = f"{current_data_line}\n"
        current_data_bytes = str.encode(current_data_string)
        self.espbike.write(current_data_bytes)
        print(f"data sent: {current_data_line}")
        

        return current_data_line

