import matplotlib.pyplot as plt
import sys

log_file_path = sys.argv[1]
plot_name = sys.argv[2]

log_file = open(log_file_path)

line = log_file.readline()
line_num = log_file.
plot_x = []
plot_y = []
#if line:
#    start_sec,start_nsec,end_sec,end_nsec = line.split(' ')
#    base = start_sec * 1000000000 + start_nsec;
#while line is True:
#    start_sec,start_nsec,end_sec,end_nsec = line.split(' ')
#    plot_x.append(start_sec * 1000000000 + start_nsec - base)
#    plot_y.append( (end_sec - start_sec) * 1000000000 + end_nsec - end_nsec)
frequency = (plot_x.index( len(plot_x) - 1) - plot_x.index(0)) / len(plot_x)
print frequency
log_file.close()