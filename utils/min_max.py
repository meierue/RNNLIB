#! /usr/bin/env python
import glob
from optparse import OptionParser

# parse command line options
usage = "usage: %prog input-file"
parser = OptionParser(usage)
(options, args) = parser.parse_args()
#if len(args) != 1:
#        parser.error("incorrect number of arguments")

filePattern = args[0]
#print "input file pattern", filePattern

# load data file
filenames = glob.glob(filePattern)
#print filenames
minVal = 100000000.0
maxVal = -minVal
for f in filenames:
#	print f
	lines = file(f).readlines()
	for l in lines:
		data = l.split()
		for d in data:
			f = float(d)
			if f > maxVal:
				maxVal = f
			if f < minVal:
				minVal = f

print minVal,maxVal
