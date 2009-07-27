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

#ifndef _INCLUDED_Random_h  
#define _INCLUDED_Random_h  

namespace Random
{ 
	unsigned int set_seed(unsigned int seed = 0);
	double normal();						//normal distribution with mean 0 std dev 1
	double normal(double dev, double mean = 0);	//normal distribution with user defined mean, dev
	double uniform(double range);			//uniform real in (-range, range)
	double uniform();						//uniform real in (0,1)
};

#endif
