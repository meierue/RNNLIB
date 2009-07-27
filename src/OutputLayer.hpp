/*Copyright 2009 Alex Graves

This file is part of RNNLIB.

RNNLIB is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RNNLIB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RNNLIB.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef _INCLUDED_OutputLayer_h  
#define _INCLUDED_OutputLayer_h  

#include <map>
#include "Layer.hpp"
#include "DataSequence.hpp"

#define ERR(x) this->errorMap[#x] = x

struct OutputLayer: public Layer
{	
	//data
	map <string, double> errorMap;
	map <string, double> normFactors;
	set<string> criteria;
	
	//functions
	OutputLayer(const string& name, size_t numSeqDims, size_t size):
		Layer(name, numSeqDims, size, size)
	{
	}
	virtual double calculate_errors(const DataSequence& seq) = 0;
};

#endif
