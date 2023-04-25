import matplotlib.pyplot as plt
import csv
  
Names = []
Values = []
  
with open('prob_final.csv','r') as csvfile:
    lines = csv.reader(csvfile, delimiter=',')
    for row in lines:
        Names.append(row[0])
        Values.append(int(row[1]))
  
plt.scatter(Names, Values, color = 'g',s = 10)
plt.xlabel('Error')
plt.ylabel('Transmissions')
plt.title('Error Probability', fontsize = 20)
  
plt.show()