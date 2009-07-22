#! /usr/bin/env python

from optparse import OptionParser
from pylab import *

stds = array([1715.033, -138.8903]) 
#means = array([2743.271, 291.0705])

# parse command line options
usage = "usage: %prog [options] input_file [offset]"
parser = OptionParser(usage)
parser.add_option("-x", "--xline", action="store", type="int", dest="xline", default=1, help="line to use for x-coords")
parser.add_option("-y", "--yline", action="store", type="int", dest="yline", default=2, help="line to use for y-coords")
parser.add_option("-p", "--pressureline", action="store", type="int", dest="pressureline", default=-1, help="line to use for pressure info (sig only)")
parser.add_option("-s", "--strokes", action="store", type="string", dest="strokes", default="iam", help="stroke format (iam, sig, other)")
parser.add_option("-o", "--outfile", dest="outfile", default="screen", action="store", type="string", help="output filename (screen = plot to screen)")
parser.add_option("-a", "--adjust", dest="adjust", action="store_false", help="adjust std dev?")
parser.add_option("--scale", dest="scale", action="store", default="1,1", type="string", help="scale factors for figure size")
(options, args) = parser.parse_args()

if len(args) == 1:
	infilename = args[0]
	offset = 0
elif len(args) == 2:
	infilename, offset = args
	offset = int(offset)
else:
        parser.error("incorrect number of arguments")
print options
print "input filename", infilename
print "offset", offset

#read header (if any)
lines = file(infilename).readlines()
x = array(lines[options.xline].split()[offset:]).astype(float)
y = array(lines[options.yline].split()[offset:]).astype(float)
if options.adjust:
	x *= stds[0]
	y *= stds[1]
hold(True)
if options.strokes == "iam":
	minx = min(x)
	miny = min(y)
	grid = ones((max(x) - minx + 1, max(y) - miny + 1), 'float')	
	fig = figure(figsize=array(grid.shape)*array(options.scale.split(',')).astype(float))	
	strokes = array(lines[-1].split()[offset:]).astype(float)
	splits = [-1]
	for i,s in enumerate(strokes):
		if s >= 1:
			splits.append(i)
	print splits
	lines = []
	for i in range(1, len(splits)):
		plot (x[splits[i-1]+1:splits[i]+1],-y[splits[i-1]+1:splits[i]+1],linewidth=1,color='black',antialiased=False)
#	for i,s in enumerate(strokes):
#		if s < 1:
#			grid[int(x[i] - minx)][int(y[i] - miny)] = 0
#	imshow(grid.T)
	gray()
	gca().left = 0
	gca().right = 0
#	strokes = array(lines[-1].split()[offset:]).astype(float)
#	begin = 0
#	oldS = 1
#	for i,s in enumerate(strokes):
#		if (i == len(strokes) -1) or (s >= 1 and oldS < 1):
#			plot (x[begin:i],y[begin:i],linewidth=3,color='black')
#			begin = i+1
#		oldS = s
#elif options.strokes == "sig":
#	pressure = array(lines[options.pressureline].split()[offset:]).astype(float)
#	#for xcoord,ycoord,p in zip(x,y,pressure):	
else:
	plot (x,y,linewidth=1.5,color='black')
# show figure
if options.outfile == "screen":
	title(infilename)
	setp(gca(),xticks=[],yticks=[],frame_on=True)
	show()
else:
	setp(gca(),xticks=[],yticks=[],frame_on=False)
	savefig(options.outfile)


