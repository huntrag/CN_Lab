
# importing libraries
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import csv
from mpl_toolkits import mplot3d
from mpl_toolkits.mplot3d import axes3d
# defining surface and axes
x=[]
y=[]
z=[]
with open('vary_final.csv','r') as csvfile:
    lines = csv.reader(csvfile, delimiter=',')
    for row in lines:
        x.append(int(row[0]))
        y.append(int(row[1]))
        z.append(int(row[2]))


fig = plt.figure(figsize = (10, 7))
ax = plt.axes(projection ="3d")
 
# Creating plot
ax.scatter3D(x, y, z, color = "green")
plt.title("Variable File size and packet size")
 
# show plot
plt.show()

# fig = plt.figure()
# wf = fig.add_subplot(111, projection='3d')
# wf.plot_wireframe(x,y,z, rstride=2,
#                   cstride=2,color='green')
  
# # displaying the visualization
# wf.set_title('Example 1')
# plt.show()