/*Copyright 2009 Alex Graves

This file is part of rnn_lib.

rnn_lib is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rnn_lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with rnn_lib.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef _INCLUDED_BiasLayer_h  
#define _INCLUDED_BiasLayer_h  

#include "Layer.hpp"

struct BiasLayer: public Layer
{
	//data
	View<double> acts;
	View<double> errors;
	
	//functions
	BiasLayer():
		Layer("bias", 0, 0, 1),
		acts(this->outputActivations[0]),
		errors(this->outputErrors[0])
	{
		acts.front() = 1;
	}
	~BiasLayer(){}
	const View<double> out_acts(const vector<int>& coords)
	{
		return acts;
	}
	const View<double> out_errs(const vector<int>& coords)
	{
		return errors;
	}
};

#endif
