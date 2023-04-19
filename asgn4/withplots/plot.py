import pandas as pd
import matplotlib.pyplot as plt

headers=['Prob by 100000','Transmission']

df = pd.read_csv('prob_saved.csv', names=headers)

df.set_index('Prob by 100000').plot()

plt.show()