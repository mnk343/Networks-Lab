file = open('data1.txt' , 'r')
file2 = open('data1_mani.txt', 'w')

for data in file:
	number = float(data)
	if(number > 0.2):
		number -= 0.2
	file2.write(str(number) + "\n")
