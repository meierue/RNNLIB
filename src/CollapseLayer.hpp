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

#ifndef _INCLUDED_CollapseLayer_h  
#define _INCLUDED_CollapseLayer_h  

#include "Layer.hpp"

struct CollapseLayer: public Layer
{	
	//data
	vector<bool> activeDims;
	vector<size_t> outSeqShape;
	
	//functions
	CollapseLayer(Layer* src, Layer* des, const vector<bool>& activDims = list_of<bool>()):
		Layer(des->name + "_collapse", des->directions, des->input_size(), des->input_size(), src),
		activeDims(activDims)
	{
		activeDims.resize(src->num_seq_dims(), false);
		assert(count(activDims, true) == des->num_seq_dims());
		display(this->inputActivations, "inputActivations");
		display(this->inputErrors, "inputErrors");
		display(this->outputActivations, "outputActivations");
		display(this->outputErrors, "outputErrors");
	}
	virtual void start_sequence()
	{	
		outSeqShape.clear();
		for (int i = 0; i < activeDims.size(); ++i)
		{
			if (activeDims[i])
			{
				outSeqShape += this->source->output_seq_shape()[i];
			}
		}
		assert(outSeqShape.size() == this->num_seq_dims());
		this->inputActivations.reshape(this->source->output_seq_shape(), 0);
		this->inputErrors.reshape(this->source->output_seq_shape(), 0);
		this->outputActivations.reshape(outSeqShape, 0);
		this->outputErrors.reshape(outSeqShape, 0);
	}
	vector<int> get_out_coords(const vector<int>& inCoords)
	{
		vector<int> outCoords;
		assert(inCoords.size() == activeDims.size());
		for (int i = 0; i < inCoords.size(); ++i)
		{
			if (activeDims[i])
			{
				outCoords += inCoords[i];
			}
		}
		assert(outCoords.size() == num_seq_dims());
		return outCoords;
	}
	void feed_forward(const vector<int>& coords)
	{
		range_plus_equals(this->outputActivations[get_out_coords(coords)], this->inputActivations[coords]);
	}
	void feed_back(const vector<int>& coords)
	{
		copy(this->outputErrors[get_out_coords(coords)], this->inputErrors[coords]);
	}
};

#endif

