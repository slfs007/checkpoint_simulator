__author__ = 'mk'
import matplotlib.pyplot as plt
import sys

logPath  = sys.argv[1]
dbSize = sys.argv[2]
unitSize = sys.argv[3]
threadNum = sys.argv[4]
resultDir = sys.argv[5]

algLabel = ["naive","cou","zz","pp","mk","ll"]
algColor = ["red","blue","yellow","green","purple","orange"]
for i in range(0,6,1):
    logFile = open(logPath + "tps_" + str(i))
    notuse,tps = logFile.readline().split(':')
    tps = long(tps)
    plt.bar(i,tps,label = algLabel[i],color=algColor[i])
    logFile.close()
plt.title("Database Size:" + dbSize  +",Thread Num:" + threadNum + ",Unit Size:" + unitSize)
plt.legend(loc = 10)
plt.xlabel("Algorithm")
plt.ylabel("TPS")
plt.savefig(resultDir + "TPS.pdf");