__author__ = 'mk'
__author__ = 'mk'

import numpy
import sys

fileName = sys.argv[1]
uniSize = int(sys.argv[2])
uniMax = int(sys.argv[3])
uniSize = uniSize * 1000 * 10


zipfFile = open(fileName,"w")


for i in range(0,uniSize,1):
    if i >= uniMax:
        i = i % uniMax
    zipfFile.write(str(i) + "\n")
zipfFile.close()