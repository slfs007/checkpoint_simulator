__author__ = 'mk'

import sys

log_file_path = sys.argv[1]
plot_name = sys.argv[2]


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
stepTime = 10000000
lastTime = xList[0]/stepTime
sumSec = 0
i = 0
xSecList = []
ySecList = []
baseTime = lastTime
avgDivisor = 0
for eachTime in xList:

    eachTime = eachTime / stepTime
    if eachTime == lastTime:
        sumSec = sumSec + yList[i]
	avgDivisor = avgDivisor + 1
    else:
        xSecList.append(lastTime - baseTime)
        ySecList.append(sumSec/ avgDivisor)
        sumSec = 0
	avgDivisor = 0
        lastTime = eachTime
    i = i + 1
print len(xSecList),len(ySecList)

forGnuPlotFile = open("./" +plot_name + "for_gnuplot.dat","w")

for i in range(0,len(ySecList),1):
    line = str(i) + '\t' + str(ySecList[i]) + '\n'
    forGnuPlotFile.write(line)
forGnuPlotFile.close()

