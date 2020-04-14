import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd

file = open('data2.txt' , 'r')

X1=[]
for data in file:
	number = float(data)
	X1.append(number)

sns.distplot(X1, hist=True, kde=True, 
             bins=100, color = 'turquoise',
             hist_kws={'edgecolor':'black'})
# Add labels
plt.title('Normal Graph of rtts')
plt.xlabel('rtt')
plt.ylabel('packets')

plt.show()