
# importing libraries
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import csv
 
# defining surface and axes
with open('vary_final_saved.csv','r') as csvfile:
    lines = csv.reader(csvfile, delimiter=',')
    for row in lines:
        x = int(row[0])
        y = int(row[1])
        z = int(row[2])

a=np.array(x)
b=np.array(y)
c=np.array(z)
  
fig = plt.figure(figsize = (10,10))
ax = plt.axes(projection='3d')
ax.grid()

ax.scatter(a,b,c, c = 'r', s = 50)
ax.set_title('3D Scatter Plot')
  
plt.show()