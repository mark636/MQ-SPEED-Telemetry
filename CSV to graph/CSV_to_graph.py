import matplotlib.pyplot as plt
import pandas as pd 
import csv

# Changeable Variables
x = 'x3_8mag_1090.csv'
S1 = ['Sensor 1']
S2 = ['Sensor 2']

# Plotting Data
data = pd.read_csv(x)

# Displaying plot data
def plot_graph():
    plt.title('RPM Graph')
    plt.ylabel('RPM')
    plt.grid(True)
    for column in data.columns:
        plt.plot(data['Sensor 2 '], label=column)
    plt.show()

# Analysis of data
def print_data():
    print('Mean:', float(data.mean()))
    print('Median:', float(data.median()))
    print('MAX:', float(data.max()))
    print('Min:', float(data.min()))

#print_data()
plot_graph()