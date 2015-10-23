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
for i in range(0,6,1):
    logPath = dataDir + str(i)+ "_latency_" + uf +"k_"  + unitNum + "_" + unitSize + "_" + threadID +  ".log"
    logFile = open( logPath)
    time=[]
    latency=[]
    for eachLine in logFile.readlines():
        timeNs,latencyNs = eachLine.split(",")
        time.append(long(timeNs))
        time[-1] = time[-1] - time[0]
        latency.append(long(latencyNs))
    plt.plot(time,latency,label=algLabel[i],linewidth=1)

plt.xlabel("time(ns)")
plt.ylabel("Latency")
plt.title("Latency Test")
plt.legend()
plt.savefig(resultDir + "Latency.pdf")