__author__ = 'mk'

import matplotlib.pyplot as plt
import sys
import math
import numpy as np
dataDir = sys.argv[1]
resDir = sys.argv[2]


plt.figure(figsize=(8,4))
algLabel=['naive','cou','zigzag','pingpong','MK','LL']

for i in range(0,6,1):

    filePath = dataDir + str(i) + '_overhead.dat'
    file = open(filePath)
    x = []
    y = []
    for eachLine in file.readlines():
        xStr,yStr = eachLine.split()
        x.append(int(xStr))
        y.append(math.log(float(yStr)))
    file.close()

    plt.plot(x,y,label=algLabel[i],linewidth=1)
plt.xlabel("Data Size")
plt.ylabel("Overhead")
plt.title("Overhead Per Checkpoint")
plt.legend()
plt.savefig(resDir + "OverheadPerCheckpoint.pdf")

