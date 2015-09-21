__author__ = 'mk'
import sys




def get_ckp_overhead(name,dbSize):
	path = name + "_" + str(dbSize) + "_" + "ckp_log"	
	file = open(path)
	line = file.readline()
	i = 0
	lastTime = 0
	overheadList = []
	while line:
		secStr,nsecStr = line.split(',')
		sec = int(secStr)
		nsec = int(nsecStr)
		time = sec * 1000000000 + nsec
		if i % 2 == 0:
			lastTime = time;	
		else:
			overheadList.append( time - lastTime)
			lastTime = 0
		line = file.readline()
	avg = sum(overheadList) / len(overheadList)
	file.close()
	return avg

logName = sys.argv[1]
ckpOverheadList = []
baseSize = 25600
resultFile = open(logName + "_ckp_overhead.dat","w")

for i in range(0,10,1):
	line = str(baseSize * (i + 1)) + "\t" + str( get_ckp_overhead(logName,baseSize * (i + 1))) + "\n"
	resultFile.write(line)
	
resultFile.close()
