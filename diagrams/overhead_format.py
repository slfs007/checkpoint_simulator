__author__ = 'mk'
import sys




def get_ckp_overhead(algType,uf,dbSize,unitSize,dataDir):
	logPath = dataDir + str(algType) + "_overhead_" + str(uf) +"k_"  + str(dbSize) + "_" + str(unitSize) + ".log"
	file = open(logPath)
	#pharse first line( name)
	line = file.readline()
	#pharse second line( unuse infomation)
	line = file.readline()
	overheadList = []
	for eachLine in file.readlines():
		prepare,overhead,total = eachLine.split()
		overheadList.append(int(total))

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
