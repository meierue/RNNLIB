#!/usr/bin/env python
from matplotlib.pyplot import *
import re
from optparse import OptionParser

usage = "usage: %prog log_file"
parser = OptionParser(usage)
parser.add_option("-v", "--verbose", dest="verbose", default=False, action="store_true", help="verbose plot (including error types with verboseChar)?")
parser.add_option("-c", "--verbosechar", dest="verboseChar", default="_", action="store", help="special character for verbose plots")
(options, args) = parser.parse_args()
errors = dict()

#print options
if len(args) != 1:
        parser.error("incorrect number of arguments")
filename = args[0]
print "plotting errors from", filename
lines = file(filename, 'r').readlines()
errorType = ""
bestEpochs = dict()
for l in lines:
	words = l.split()
	if (l.find("epoch") >= 0 and l.find("took") >= 0):
		epochNum = int(l.split()[1])
	elif (l.find("train errors") >= 0):
		errorType = "train"
	elif (l.find("test errors") >= 0):
		errorType = "test"
	elif (l.find("validation errors") >= 0):
		errorType = "validation"
	elif (len(words) == 0 or l.find("best") >= 0):
		errorType = ""
		if l.find("best network") >= 0:
			bestEpochs[l.split()[2]] = epochNum
		elif l.find(".best_") >= 0:
			bestEpochs['(' + l.split('.')[-2].split('_')[1] + ')'] = epochNum
	elif len(words) == 2 and errorType <> "":
		errWord = words[0]
		if options.verbose or options.verboseChar not in errWord:
			errVal = float(words[1].strip('%'))
			if errWord not in errors:
				errors[errWord] = dict()
			if errorType in errors[words[0]]:
				errors[errWord][errorType][0].append(epochNum)
				errors[errWord][errorType][1].append(errVal)
			else:
				errors[errWord][errorType] = [[epochNum],[errVal]]

for err in errors.items():
	figure()
	title(filename + ' \n' + err[0]) 
	for dataSet in err[1].items():
		plot(dataSet[1][0], dataSet[1][1], linewidth=1.5, label=dataSet[0], marker='+')
	axes = gca()
	yRange = [axis()[2], axis()[3]]
	if len(bestEpochs) > 0:
		bone()
		for best in bestEpochs.items():
			if re.search("\(.*\)", best[0]):
				lab = "best "+best[0]
			else:
				lab = "best network"
			plot([best[1], best[1]], yRange, linestyle ='--', linewidth=1, label=lab)
	legend()
	legend(prop = matplotlib.font_manager.FontProperties(size = 'smaller'))

show()
