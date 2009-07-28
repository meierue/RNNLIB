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

#ifndef _INCLUDED_MultilayerNet_h  
#define _INCLUDED_MultilayerNet_h  

#include "Mdrnn.hpp"
#include "RegressionLayer.hpp"
#include "ClassificationLayer.hpp"
#include "AutoregressionLayer.hpp"
#include "TranscriptionLayer.hpp"
#include "DecodingLayer.hpp"
 
struct MultilayerNet: public Mdrnn
{
	//functions
	MultilayerNet(ostream& out, ConfigFile& conf, const DataHeader& data):
		Mdrnn(out, conf, data)
	{		
		vector<int> hiddenSizes = conf.get_list<int>("hiddenSize");
		assert(hiddenSizes.size());
		vector<string> hiddenTypes = conf.get_list<string>("hiddenType", "lstm", hiddenSizes.size());
		vector<vector<size_t> > hiddenBlocks = conf.get_array<size_t>("hiddenBlock");
		assert(hiddenBlocks.size() < hiddenSizes.size());
		vector<int> subsampleSizes = conf.get_list<int>("subsampleSize");
		assert(subsampleSizes.size() < hiddenSizes.size());
		vector<bool> recurrent = conf.get_list<bool>("recurrent", true, hiddenSizes.size());
		Layer* in = this->get_input();
		loop(int i, indices(hiddenSizes))
		{
			this->add_hidden_level(hiddenTypes.at(i), hiddenSizes.at(i), recurrent.at(i), "hidden_" + str(i));
			this->connect_to_hidden_level(in, i);
			vector<Layer*> blocks;
			if (i < hiddenBlocks.size())
			{
				loop(Layer* l, hiddenLevels[i])
				{
					blocks += this->add_layer(new BlockLayer(l, hiddenBlocks.at(i)));
				}
			}
			vector<Layer*>& topLayers = blocks.size() ? blocks : hiddenLevels[i];
			if (i < subsampleSizes.size())
			{
				in = this->add_layer(new NeuronLayer<Tanh>("subsample_" + str(i), this->num_seq_dims(), subsampleSizes.at(i)));
				loop(Layer* l, topLayers)
				{
					this->connect_layers(l, in);
				}
			}
			else if (i < last_index(hiddenSizes))
			{
				in = this->add_layer(new GatherLayer("gather_" + str(i), topLayers));
			}
		}
		string task = conf.get<string>("task");
		string outputName = "output";
		Layer* output;
		if (task == "regression")
		{
			output = this->outputLayer = new RegressionLayer(this->out, outputName, this->num_seq_dims(), data.outputSize);
		}
		else if (task == "sequence_regression")
		{
 			output = this->outputLayer = new RegressionLayer(this->out, outputName, 0, data.outputSize);
			if (this->num_seq_dims())
			{
				output = this->collapse_layer(this->hiddenLayers.back(), this->outputLayer);
			}
		}
		else if (task == "autoassociation")
		{
			output = this->outputLayer = new AutoregressionLayer(outputName, this->get_input());
		}
		else if (task == "classification")
		{
			output = this->outputLayer = new ClassificationLayer(this->out, outputName, this->num_seq_dims(), data.targetLabels);
		}
 		else if (task == "sequence_classification")
 		{
 			output = this->outputLayer = new ClassificationLayer(this->out, outputName, 0, data.targetLabels);
			if (this->num_seq_dims())
			{
				output = this->collapse_layer(this->hiddenLayers.back(), this->outputLayer);
			}
 		}
 		else if (task == "transcription")
 		{
			assert(this->num_seq_dims());
 			output = this->outputLayer = new TranscriptionLayer(this->out, outputName, data.targetLabels);
			if (this->num_seq_dims() > 1)
			{
				output = this->collapse_layer(this->hiddenLayers.back(), this->outputLayer, list_of(true));
			}
 		}
 		else if (task == "decode")
 		{
			assert(this->num_seq_dims());
 			output = this->outputLayer = new DecodingLayer(this->out, outputName, data.targetLabels, conf.get<string>("dictionary", ""),
																conf.get<string>("bigrams", ""), conf.get<int>("nBest", 1), conf.get<int>("fixedLength", -1));
			if (this->num_seq_dims() > 1)
			{
				output = this->collapse_layer(this->hiddenLayers.back(), this->outputLayer, list_of(true));
			}
 		}
		else
		{
			cout << "ERROR: unknown output layer type '" << task << "'" << endl;
			exit(0);
		}
 		this->add_bias(this->outputLayer);
		connect_from_hidden_level(hiddenLevels.size() - 1, output);
	}
};

#endif
