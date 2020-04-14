file = open('data2.txt' , 'r')
file2 = open('results2.txt', 'w')

arr = []

for data in file:
	number = float(data)
	arr.append(number)

arr.sort()
minimum = 100000
maximum = 0
add = 0
count = 0

for number in arr:
	add += number
	minimum = min(minimum , number)
	maximum = max(minimum , number)
	count+=1

add/= count
if(count %2 == 0 ):
	median = (arr[count/2] + arr[count/2-1] )/2
else:
	median = arr[count/2]

file2.write("Minimum: " + str(minimum) + "\n")
file2.write("Maximum: " + str(maximum) + "\n")
file2.write("Mean: " + str(add) + "\n")
file2.write("Median: " + str(median) + "\n")

file2.close()
file.close()