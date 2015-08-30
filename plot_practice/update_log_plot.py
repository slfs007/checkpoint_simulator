__author__ = 'mk'
import matplotlib.pyplot as plt
import sys

log_file_path = sys.argv[1]
plot_name = sys.argv[2]

db_log = open(log_file_path)
line = db_log.readline()
num1str,num2str = line.split(',')
num1 = int(num1str)
num2 = int(num2str)
db_log_list = []
db_log_x_list = []
db_log_y_list = []
i = 0
num = 1000000
while i < num:
    i = i + 1
    db_log_list.append(num1 * 1000000000 + num2)
    line = db_log.readline()
    if line:
        num1str,num2str = line.split(',')
        num1 = int(num1str)
        num2 = int(num2str)
base = db_log_list[0]
for i in range(0,num,1):
    db_log_list[i] = db_log_list[i] - base
    if 0 == i%2:
        db_log_x_list.append(db_log_list[i])
    else:
        db_log_y_list.append(db_log_list[i] - db_log_list[i - 1])

print len(db_log_x_list),len(db_log_y_list)
plt.plot( range(0,len(db_log_y_list),1),db_log_y_list)
plt.ylabel(plot_name)
plt.show()
db_log.close()


