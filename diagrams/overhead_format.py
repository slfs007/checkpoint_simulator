__author__ = 'mk'
import sys




def get_ckp_overhead(algType,uf,dbSize,unitSize,dataDir):
	logPath = dataDir + str(algType) + "_overhead_" + str(uf) +"k_"  + str(dbSize) + "_" + str(unitSize) + ".log"
	file = open(logPath)


	line = file.readline()
	overheadList = []
	while line:
		overheadList.append(int(line))
		line = file.readline()
	avg = sum(overheadList) / len(overheadList)
	file.close()
	return avg

algType = int(sys.argv[1])
baseSize = int(sys.argv[2])
baseUF = int(sys.argv[3])
unitSize = int(sys.argv[4])

resultDir = sys.argv[5]
dataDir =  sys.argv[6]


resultFile = open(str(resultDir) + str(algType) + "_overhead.dat","w")
for i in range(0,10,1):
	line = str(baseSize * (i + 1)) + "\t" + \
           str( get_ckp_overhead(algType,baseUF * (i + 1),baseSize * (i + 1),
                                 unitSize,dataDir)) + "\n"
	resultFile.write(line)

resultFile.close()
