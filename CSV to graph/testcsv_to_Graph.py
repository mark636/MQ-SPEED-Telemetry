import pandas as pd
from matplotlib import pyplot as plt
# Read a CSV file
reader = pd.read_csv(("Test_2022_12_13_19_40_31.csv"), skipinitialspace=True)
# Plot the lines
reader.plot()
plt.show()