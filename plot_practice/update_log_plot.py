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

lastTime = xList[0]/100000000
sumSec = 0
i = 0
xSecList = []
ySecList = []
baseTime = lastTime
for eachTime in xList:

    eachTime = eachTime / 100000000
    if eachTime == lastTime:
        sumSec = sumSec + yList[i]
    else:
        xSecList.append(lastTime - baseTime)
        ySecList.append(sumSec)
        sumSec = 0
        lastTime = eachTime
    i = i + 1
print len(xSecList),len(ySecList)
plt.plot( xSecList,ySecList)
plt.ylabel(plot_name)
plt.show()
db_log.close()


