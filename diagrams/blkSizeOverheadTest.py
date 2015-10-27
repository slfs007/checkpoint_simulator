__author__ = 'mk'
__author__ = 'mk'
import sys
import matplotlib.pyplot as plt


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


baseNum = int(sys.argv[1])
baseUF = int(sys.argv[2])
baseBlkSize = int(sys.argv[3])

resultDir = sys.argv[4]
dataDir =  sys.argv[5]

algLabel=['naive','cou','zigzag','pingpong','MK','LL']
plt.figure(figsize=(8,4))
for i in range(0,6,1):
    dataSize = []
    Overhead = []
    for j in range(0,5,1):
        #x
        dataSize.append(baseBlkSize)
        #y
        Overhead.append(get_ckp_overhead(i,baseUF,baseNum,baseBlkSize,dataDir))
        baseBlkSize = baseBlkSize / 2
        baseUF = baseUF * 2
        baseNum = baseNum * 2
    plt.plot(dataSize,Overhead,label = algLabel,linewidth=1)
plt.xlabel("block size")
plt.ylabel("Overhead")
plt.title("Block Test")
plt.legend()
plt.savefig(resultDir + "Block.pdf")

