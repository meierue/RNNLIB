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

#ifndef _INCLUDED_CodeNet_h  
#define _INCLUDED_CodeNet_h  

#include "Mdrnn.h"
#include "AutoregressionLayer.h"

struct CodeNet: public Mdrnn
{
	//data
	vector<int> hiddenSizes;
	bool sampling;
	bool recurrent; 
	bool complete;
	string hiddenType;

	//functions
	CodeNet(ostream& out, ConfigFile& conf, const DataHeader& data):
		Mdrnn(out, conf, data),
		hiddenSizes(conf.get_list<int>("hiddenSize")),
		sampling(conf.get<bool>("sampling", true)),
		recurrent(conf.get<bool>("recurrent", true)),
		complete(conf.get<bool>("complete", true)),
		hiddenType(conf.get<string>("hiddenType", "lstm"))
	{				
		//check config data ok
		assert(hiddenSizes.size());
		
		//add compressors
		Layer* code = this->get_input();
		Layer* target = code;
		int numLayers = hiddenSizes.size();
		for (int i = 0; i < numLayers; ++i)
		{
			if (!complete)
			{
				target = code;
			}
			string index = str(i);
			int hiddenSize = hiddenSizes[i];
			int compressorLevel = this->add_hidden_level(hiddenType, hiddenSize, recurrent, "compressor_" + index);
			this->connect_to_hidden_level(code, compressorLevel);
			code = this->gather_level("code_" + index, compressorLevel);
		}
		//add code sample to final code
		Layer* codeOut = (sampling ? this->add_layer(new SamplingLayer(code)) : code);
		
		//add decompressors
		int decompIdx = numLayers - 1;
		string index = str(decompIdx);
		int decompressorLevel = this->add_hidden_level(hiddenType, hiddenSizes[decompIdx], recurrent, "decompressor_" + index);
		this->connect_to_hidden_level(codeOut, decompressorLevel);
		if (complete && numLayers > 1)
		{
			code = this->gather_level("decompressor_" + index + "_code", decompressorLevel);
			codeOut = (sampling ? this->add_layer(new SamplingLayer(code)) : code);
			for (--decompIdx; decompIdx >= 0; --decompIdx)
			{
				index = str(decompIdx);
				decompressorLevel = this->add_hidden_level(hiddenType, hiddenSizes[decompIdx], recurrent, "decompressor_" + index);
				this->connect_to_hidden_level(codeOut, decompressorLevel);
				if (decompIdx > 0)
				{
					code = this->gather_level("decompressor_" + index + "_code", decompressorLevel);
					codeOut = (sampling ? this->add_layer(new SamplingLayer(code)) : code);
				}
			}
		}
		
		//add output layer
		this->outputLayer = new AutoregressionLayer("output", target);
		this->connect_from_hidden_level(decompressorLevel, this->outputLayer);
		this->add_bias(this->outputLayer);
	}
	void print(ostream& out = cout) const
	{
		Mdrnn::print(out);
		PRINT(recurrent, out);
		PRINT(hiddenSizes, out);
		PRINT(complete, out);
		PRINT(sampling, out);
		PRINT(hiddenType, out);
	}
};

#endif
