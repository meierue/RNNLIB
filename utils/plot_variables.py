#! /usr/bin/env python

from matplotlib.lines import Line2D
from optparse import OptionParser
from matplotlib import *
from PIL import Image
import numpy
from numpy import array
import sys

# parse command line options
usage = "usage: %prog [options] input-file"
parser = OptionParser(usage)
parser.add_option("-s", "--style", dest="linestyle", default="None", action="store", type="string", help="Line styles not implemented yet")
parser.add_option("-o", "--output", dest="outputFilename", default="screen", action="store", type="string", help="output filename (screen = plot to screen)")
parser.add_option("-g", "--gridplot", dest="gridPlot", default=False, action="store_true", help="do grid plot?")
parser.add_option("-n", "--numgrids", dest="numGrids", default=1, action="store", type=int, help="number of grids per data line (gridPlot only)")
parser.add_option("-w", "--gridwidth", dest="gridWidth", default=0, action="store", type=int, help="width of grids (gridPlot only; assumed square if <= 0)")
parser.add_option("-l", "--lo", dest="lo", default=0, action="store", type=float, help="lo value for y axis (auto if hi and lo 0)")
parser.add_option("-i", "--hi", dest="hi", default=0, action="store", type=float, help="hi value for y axis (auto if hi and lo 0)")
parser.add_option("-k", "--key", dest="key", default=False, action="store_true", help="display key")
parser.add_option("--use", dest="use", default="", type='string', action="store", help="comma separated list of variables to plot (indices or labels) OVERRIDES ignore")
parser.add_option("--ignore", dest="ignore", default="", type='string', action="store", help="comma separated list of variables not to plot (indices or labels)")
parser.add_option('-p', "--printlabels", dest="printlabels", default=False, action="store_true", help="print labels of variables with highest value?")
parser.add_option("-a", "--noaxes", dest="noAxes", default=False, action="store_true", help="don't show axes")
parser.add_option("-c", "--colour", dest="colour", default=False, action="store_true", help="plot in colour? (gridPlot only)")
parser.add_option("-m", "--min", dest="min", default=0, action="store", type=int, help="min line number")
parser.add_option("-x", "--max", dest="max", default=0, action="store", type=int, help="max line number (all lines if 0)")
parser.add_option("-t", "--transpose", dest="transpose", default=False, action="store_true", help="transpose data?")
parser.add_option("-f", "--flipud", dest="flipud", default=False, action="store_true", help="flip data upside down?")
parser.add_option("-b", "--abs", dest="abs", default=False, action="store_true", help="plot absolute values?")
parser.add_option("-r", "--rotate", dest="rotate", default=False, action="store_true", help="rotate data 90 degrees?")
parser.add_option("--ycropbegin", dest="yCropBegin", default=0, action="store", type=int, help="y crop begin")
parser.add_option("--ycropend", dest="yCropEnd", default=-1, action="store", type=int, help="y crop end")
parser.add_option("--xcropbegin", dest="xCropBegin", default=0, action="store", type=int, help="x crop begin")
parser.add_option("--xcropend", dest="xCropEnd", default=-1, action="store", type=int, help="x crop end")
parser.add_option("--showallgrids", dest="showAllGrids", default=False, action="store_true", help="show all grids? (otherwise limited to 10)")
parser.add_option("-v", "--vectorplot", dest="vectorPlot", default=False, action="store_true", help="plot vector field?")
parser.add_option("--rgb", dest="rgb", default=False, action="store_true", help="combine into rgb image(s)? (false if vectorPlot true)")
parser.add_option("--blockdims", dest="blockDims", default=None, action="store", type="string", help="2d block dims (e.g. 2,2)")
parser.add_option("--rgbmeans", dest="rgbMeans", default="0,0,0", action="store", type="string", help="rgb means")
parser.add_option("--rgbdevs", dest="rgbDevs", default="1,1,1", action="store", type="string", help="rgb means")
parser.add_option("--rgbsplit", dest="rgbSplit", default=False, action="store_true", help="split rgb image into separate channels?")
(options, args) = parser.parse_args()
if options.vectorPlot:
	options.rgb = False
if options.blockDims <> None:
	options.showAllGrids = True

if len(args) != 1:
        parser.error("incorrect number of arguments")
infilename = args[0]

print options
print "input filename", infilename

#read header (if any)
numRowsToSkip = 0
labels = []
data = file(infilename)
dimensions = []
line = data.readline().split()
while line[0][-1] == ':' or line[0] == "#":
	if line[0] == "LABELS:" or line[0] == "#":
		labels = line[1:]
		numRowsToSkip += 1
		print 'labels =', labels
	if line[0] == "DIMENSIONS:":
		for d in line[1:]:
			dimensions.append(int(d))
		numRowsToSkip += 1
		print 'dimensions =', dimensions
		if len(dimensions) >= 2 and int(dimensions[1]) > 1:
			options.gridPlot = True
			options.gridWidth = dimensions[0]
			if (len(dimensions) > 2):
				options.numGrids = product(dimensions[2:])
	line = data.readline().split()
print 'numRowsToSkip',numRowsToSkip
		
# load data file
data = load(infilename, skiprows = numRowsToSkip)
if len(data.shape) == 1:
	data.shape = (1, data.shape[0])
if len(labels):
	assert len(labels) == len(data)

#apply corrections to data depending on options
useList = options.use.replace(',', ' ').split()
ignoreList = options.ignore.replace(',', ' ').split()
if len(ignoreList) and not(len(useList)):
	for i in range(data.shape[0]):
		if str(i) not in ignoreList and not(len(labels) and labels[int(i)] in ignoreList):
			useList.append(i)
if len(useList):
	print data.shape
	lines = set()
	for v in useList:
		if v in labels:
			i = labels.index(v)
		else:
			i = int(v)
		if i >= 0 and i < len(data):
			lines.add(i)
	newlabels = []
	newdata = []
	print 'only plotting variables', sorted(lines)
	for i in sorted(lines):
		if len(labels):
			newlabels.append(labels[i])
		newdata.append(data[i])
	data = array(newdata)
	if len(labels):
		labels = newlabels
		print 'with labels', labels
if options.abs:
	newdata = []
	for l in data:
		newdata.append(fabs(l))
	newdata = array(newdata)

assert len(data)
print 'data shape =', data.shape

#print data
if options.max:
	data = data[options.min:options.maxx]
elif options.min:
	data = data[options.min:]

if options.gridPlot and len(data) > 5 and not options.showAllGrids:
	data = data[:5]

#print data
def pcolorLine(line):
	size = int(len(line)/options.numGrids)
	if options.rgb:
		area = size / 3
	else:
		area = size
	if options.gridWidth > 1:
		width = options.gridWidth
		height = area / width
	else:	
		height = width = int(sqrt(area))
	print "area",area,"width",width,"height",height
	begin=0
	rowscols = ceil(sqrt(options.numGrids))
	vectorGrids = []
	for i in range(options.numGrids):
		#get the data
		end = begin + size
		grid = line[begin:end]
		if options.abs:
			newgrid = []
			for g in grid:
				newgrid.append(math.fabs(g))
			grid = array(newgrid)
		begin = end
		if (options.rgb):
			grid.shape = (width,height,3)
		else:
			grid.shape = (width,height)
		if not options.vectorPlot:
			subplot(rowscols, rowscols, i+1) 
		if not options.transpose:
			grid = transpose(grid)
		if options.rotate:
			grid = rot90(grid)
		if options.flipud:
			grid = flipud(grid)
		cropped = False
		if options.yCropBegin <> 0 or options.yCropEnd <> -1:
			grid = grid[options.yCropBegin: options.yCropEnd]
			cropped = True
		if options.xCropBegin <> 0 or options.xCropEnd <> -1:
			newGrid = []
			for line in grid:
				newGrid.append(line[options.xCropBegin: options.xCropEnd])
			grid = array(newGrid)
			cropped = True
		if cropped:
			print 'cropped to', grid.shape
		if options.vectorPlot:
			vectorGrids.append(grid)
		else:
			imshow(grid, interpolation='nearest')
#			pcolor(grid, shading = 'flat')
#			setp(gca(),xticks=[],yticks=[],axisbelow='false',frame_on='false',xlim=(0.0,float(grid.shape[1])),ylim=(0.0,float(grid.shape[0])))
	if options.vectorPlot:
		return vectorGrids, rowscols
	
if options.gridPlot:

	if not options.colour:
		gray()

	if options.vectorPlot:
		fig = figure()
		title(infilename)
		xGrids, rowscols = pcolorLine(data[0])
		yGrids, rowscols2 = pcolorLine(data[1])
		for n in range(len(xGrids)):
			subplot(rowscols, rowscols, n+1) 
			quiver(xGrids[n], yGrids[n])
			setp(gca(),xticks=[],yticks=[],frame_on=False,xlim=(0.0,float(xGrids[0].shape[0])),ylim=(0.0,float(xGrids[0].shape[1])))
		
	else:
		if options.rgb:
			rgbDevs = numpy.array(options.rgbDevs.split(',')).astype(float)
			rgbMeans = numpy.array(options.rgbMeans.split(',')).astype(float)
			newData = []
			for i in range(0, len(data), 3):
				if i + 3 <= len(data):
					arrays = [numpy.array(data[i]),numpy.array(data[i+1]),numpy.array(data[i+2])]
					for j in range(3):
						arrays[j] = ((arrays[j]*rgbDevs[j]) + rgbMeans[j]) / 255.0
						for k in range(len(arrays[j])):
							if arrays[j][k] < 0.0:
								arrays[j][k] = -arrays[j][k]
							elif arrays[j][k] > 1.0:
								arrays[j][k] = 2.0-arrays[j][k]
					newData.append(numpy.column_stack(arrays).flatten())
			data = newData
		elif options.rgbSplit:
			r = data[::3]
			g = data[1::3]
			b = data[2::3]
			data = r_[r,g,b]
		if options.blockDims <> None:
			blockDims = numpy.array(options.blockDims.split(',')).astype(int)
			print blockDims
			if len(blockDims == 2):
				blockArea = numpy.product(blockDims)
				newData = []
				for i in range(0, len(data), blockArea):
					if i + blockArea <= len(data):
						width = options.gridWidth
						height = len(data[i]) / options.gridWidth
						if (options.rgb):
							colourDepth = 3
						else:
							colourDepth = 1
						height /= colourDepth
						print "width, height", width, height
						newline = zeros((width*blockDims[0], height*blockDims[1], colourDepth), 'f')
						print newline.shape
						index = 0
						for j in range(width):
							for k in range(height):
								blockIndex = 0
								for l in range(blockDims[0]):
									for m in range(blockDims[1]):
										newline[(j*blockDims[0]) + l][(k*blockDims[1]) + m][:] = data[i + blockIndex][index:index+colourDepth]
										blockIndex += 1
								index += colourDepth
						newData.append(newline.flatten())
				data = newData
				if options.gridWidth > 1:
					options.gridWidth *= blockDims[0]
		for i,line in enumerate(data):
			fig = figure()
			title(infilename + '_' + str(i + options.min))
			pcolorLine(line)
else:

	# create figure
	fig = figure()
	title(infilename)
	axes = axes()

	# add data to figure
	lineLabelMap = dict()
	colors = []
	for c in range(data.shape[0]):
		lineData = data[c,:]
		if len(labels):
			lab = labels[c]
		else:
			lab = str(c)
		line2d = axes.plot(lineData, linewidth=1.5, label = lab)[0]
		colors.append(line2d.get_color())
		lineLabelMap[line2d] = lab

	if options.printlabels:
		prevMaxNum = -1
		yPlotCoord = axes.get_ylim()[1] + 0.01 * (axes.get_ylim()[1] - axes.get_ylim()[0])
		for r in range(data.shape[1]):
			max = 0
			maxNum = -1
			for v in range(len(data[:,r])):
				val = abs(data[v][r])
				if val > max:
					max = val
					maxNum = v
			if maxNum < len(labels) -1 and maxNum <> prevMaxNum:
					text(r, yPlotCoord, labels[maxNum], {'color': 'black', 'fontsize' : 'medium'}, fontweight = 'bold', horizontalalignment = 'center', verticalalignment = 'bottom')
					prevMaxNum = maxNum

if (options.lo <> 0 or  options.hi <> 0):
	axes.set_ylim( (options.lo,options.hi))

if options.noAxes:
	axis('off')
else:
	axis('on')
if options.key:
	legend(prop = matplotlib.font_manager.FontProperties(size = 'smaller'))


def pick(event):
	if event.key=='p' and event.inaxes is not None:
		ax = event.inaxes
		a = ax.pick(event.x, event.y)

		if isinstance(a, Line2D) and a in lineLabelMap:
			labelNum = int(lineLabelMap[a])
			if labelNum >= 0 and labelNum < len(labels):
				print event.xdata, event.ydata, labels[labelNum], labelNum
				text(event.xdata, event.ydata, labels[labelNum], {'color': 'black', 'fontsize' : 'medium'}, fontweight = 'bold', horizontalalignment = 'center', verticalalignment = 'center')
				draw()


# show figure
if options.outputFilename == "screen":
	connect('key_press_event', pick)
	show()
else:
	savefig(options.outputFilename)




    












##!/usr/bin/env python
#"""
#Hold the pointer over an object and press "p" to pick it.  When
#picked it will turn red 
#
#Note this algorithm calculates distance to the vertices of the
#polygon, so if you want to pick a patch, click on the edge!
#
#"""
#from pylab import *
#from matplotlib.text import Text
#from matplotlib.lines import Line2D
#from matplotlib.patches import Patch
#
#def pick(event):
#    if event.key=='p' and event.inaxes is not None:
#        ax = event.inaxes
#        a = ax.pick(event.x, event.y)
#        
#	if isinstance(a, Text):
#            a.set_color('r')
#        elif isinstance(a, Line2D):
#            a.set_markerfacecolor('r')
#	elif isinstance(a, Patch):
#            a.set_facecolor('r')
#        draw()
#            
#    
#connect('key_press_event', pick)
#
#ax = subplot(111)
#title('Put mouse over object and press "p" to pick it')
#
#for i in range(20):
#    x, y = rand(2)
#    text(x,y,'hi!')
#
#for i in range(5):
#    x = rand(10)
#    y = rand(10)
#    plot(x,y,'go')
#
#for i in range(5):
#    x = rand()
#    y = rand()
#    center = x,y
#    p = Circle(center, radius=.1)
#    ax.add_patch(p)
#    
#
#show()
