from Scientific.IO.NetCDF import NetCDFFile
from numpy import *

def createNcDim(ncfile,name,d):
	print "creating netcdf dimension:",name,d
	ncfile.createDimension(name,d)

#assumes ncfile will be written over (opened with 'w')
def	createNcVar(ncfile,vname,data,vtype,dims,desc):
	print "creating netcdf variable",vname
	nc_var = ncfile.createVariable (vname,vtype,dims)
	nc_var.longname	= desc
	nc_var.assignValue(data)
	print shape(nc_var)

def maxLen(strings):
	maxLength=0
	for s in strings:
		length=len(s)
		if (length>maxLength):
			maxLength=length
	return maxLength

def	createNcStrings(ncfile,vname,strings,dims,desc):
	print "wrting strings", vname
	maxLength = maxLen(strings) + 1
	nullStrings = []
	for s in strings:
		nullStrings.append(list(s) +['\0']*(maxLength - len(s)))
	createNcDim(ncfile,dims[1],maxLength)
	createNcVar(ncfile,vname,array(nullStrings),'c',dims,desc)

# def	createNcString(ncfile,vname,string,dims,desc):
# 	print "writing string",vname
# 	nullString = string + '\0'
# 	createNcDim(ncfile,dims[0],len(nullString))
# 	createNcVar(ncfile,vname,nullString,'c',dims,desc)
