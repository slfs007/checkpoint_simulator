__author__ = 'mk'
import sys
import matplotlib.pyplot as plt


unitSize = sys.argv[1];
unitNum = sys.argv[2];
uf = sys.argv[3];
threadID = sys.argv[4];
dataDir = sys.argv[5];
resultDir = sys.argv[6];

algLabel=['naive','cou','zigzag','pingpong','MK','LL']

plt.figure(figsize=(8,4))
for i in range(1,6,1):
    logPath = dataDir + str(i)+ "_latency_" + uf +"k_"  + unitNum + "_" + unitSize + "_" + threadID +  ".log"
    logFile = open( logPath)
    time=[]
    latency=[]
    timeSum = 0
    lastTime = 0
    #0.1 sec
    scale = 100000000
    count = 0
    for eachLine in logFile.readlines():
        timeNsStr,latencyNsStr = eachLine.split(",")
        timeNs = int(timeNsStr)
        latencyNs = int(latencyNsStr)

        if timeNs/scale != lastTime:
            if count == 0:
                lastTime = timeNs/scale
                timeSum = 0

            else:
                time.append(lastTime)
                latency.append(timeSum/count)
                count = 0
                lastTime = timeNs/scale
                timeSum = 0
        else:
            timeSum += latencyNs
            count += 1

    baseTime = time[0]
    for j in range(0,len(time),1):
        time[j] = time[j] - baseTime

    plt.plot(time,latency,label=algLabel[i],linewidth=1)
    logFile.close()
    time = []
    latency = []
plt.xlabel("time(ns)")
plt.ylabel("Latency")
plt.title("Latency Test")
plt.legend()
plt.savefig(resultDir + "Latency.pdf")
