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

#ifndef _INCLUDED_InputLayer_h  
#define _INCLUDED_InputLayer_h  

#include "Layer.hpp"

struct InputLayer: public Layer
{
	//functions
	InputLayer(const string& name, size_t numSeqDims, size_t size, const bimap<int, string>& labels):
		Layer(name, numSeqDims, 0, size)
	{
		const bimap<int, string>* labelPtr = labels.empty() ? 0 : &labels;
		display(this->outputActivations, "activations", labelPtr);
		display(this->outputErrors, "errors", labelPtr);
	}
	~InputLayer(){}
	template<typename T> void copy_inputs(const SeqBuffer<T>& inputs)
	{
		assert(inputs.depth == this->output_size());
		this->outputActivations = inputs;
		this->outputErrors.reshape(this->outputActivations, 0);
	}
};

#endif
