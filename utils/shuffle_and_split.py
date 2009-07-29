#!/usr/bin/python
import random
import sys
from scipy import *
from optparse import OptionParser

#command line options
parser = OptionParser("usage: %prog [options] input_file split_fractions output_root")
(options, args) = parser.parse_args()
if (len(args) != 3):
	parser.error("incorrect number of arguments")
print options
infilename = args [0]
splitfracs = list(array(args[1].split()).astype(float))
outroot = args[2]
lines = file(infilename).readlines()
random.shuffle(lines)
while(sum(splitfracs) < 1.0 - 0.00001):
	splitfracs.append(min(splitfracs[-1], 1.0 - sum(splitfracs)))
if sum(splitfracs) > 1.0:
	print "ERROR: split fractions sum to", sum(splitfracs), "> 1.0, exiting"
	sys.exit(0)
#print splitfracs
line = 0
splitlines = []
for i,s in enumerate(splitfracs[:-1]):
	line += int(s * len(lines))
	splitlines.append(line)
splitlines.append(len(lines))
oldl = 0
for i,l in enumerate(splitlines):
	out = file(outroot + '_' + str(i) + '.txt', 'w')
	for line in lines[oldl:l]:
		print >> out, line.strip()
	oldl =l
out.close()
