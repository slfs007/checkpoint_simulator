__author__ = 'mk'
import matplotlib.pyplot as plt
import sys

log_file_path = sys.argv[1]
plot_name = sys.argv[2]
avg_divisor = int(sys.argv[3])

db_log = open(log_file_path)

xList = []
yList = []

line = db_log.readline()
i = 0
while line :
    secStr,nsecStr = line.split(',')
    sec = int(secStr)
    nsec = int(nsecStr)
    time = sec * 1000000000 + nsec
    if 0 == i % 2:
        xList.append(time)
    else:
        yList.append(time - xList[-1])
    i = i + 1
    line = db_log.readline()
i = 0
print len( yList)
avgYList = []
sum = 0
while i < len(yList):
    sum = sum + yList[i]
    i = i + 1
    if 0 == i % avg_divisor:
        avgYList.append(sum / avg_divisor)
        sum = 0
print len( avgYList)
plt.plot( range(0,len(avgYList),1),avgYList)
plt.ylabel(plot_name)
plt.show()
db_log.close()


