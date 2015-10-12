__author__ = 'mk'

import matplotlib.pyplot as plt
import sys
import numpy as np
dataDir = sys.argv[1]
resDir = sys.argv[2]


plt.figure(figsize=(8,4))
algLabel=['naive','cou','zigzag','pingpong','MK']

for i in range(0,5,1):

    filePath = dataDir + str(i) + '_overhead.dat'
    file = open(filePath)
    x = []
    y = []
    for eachLine in file.readlines():
        xStr,yStr = eachLine.split()
        x.append(int(xStr))
        y.append(int(yStr))
    file.close()
    npX = np.array(x)
    npY = np.array(y)
    plt.plot(npX,npY,label=algLabel[i],linewidth=2)
plt.xlabel("Data Size")
plt.ylabel("Overhead")
plt.title("Overhead Per Checkpoint")
plt.savefig(resDir + "OverheadPerCheckpoint.pdf")

