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

#ifndef _INCLUDED_NetcdfDataset_h  
#define _INCLUDED_NetcdfDataset_h  

#include <functional>
#include <algorithm>
#include <string>
#include <numeric>
#include <map>
#include <netcdfcpp.h>
#include <boost/bimap.hpp>
#include "DataSequence.hpp"
#include "Helpers.hpp"

#define SEQ_IT vector<DataSequence*>::iterator
#define CONST_SEQ_IT vector<DataSequence*>::const_iterator
static int load_nc_dim(const NcFile& ncf, const string& name, bool required = true)
{
	NcDim* d = 0;
	try
	{
		d = ncf.get_dim(name.c_str());
	}
	catch(char* str)
	{
		check(!required, string(str) + "\ndimension " + name + " not found in netcdf file");
	}
	int size = d ? d->size() : 0;
	return size;
}
static NcVar* load_nc_variable (const NcFile& ncf, const string& name, bool required = true)
{
	NcVar *v = 0;
	try
	{
		v = ncf.get_var(name.c_str());
	}
	catch(char* str)
	{
		check(!required, string(str) + "\nvariable " + name + " not found in netcdf file");
	}
	return v;
}
static string get_nc_string(const NcFile& ncf, const string& name, int offset = 0, bool required = true)
{
	static array<long, 2> offsets = {{0, 0}};
	static array<long, 2> counts = {{1, 0}};
	NcVar *v = load_nc_variable(ncf, name.c_str(), required);
	if (v)
	{
		long* shape = v->edges();
		offsets.front() = offset;
		counts.back() = shape[1];
		v->set_cur(&offsets.front());
		char* temp = new char [shape[1]];
		delete shape;
		bool success = v->get(temp, &counts.front());
		if(!success)
		{
			check(!required, " index " + str(offset) + " out of bounds for " + name + " in netcdf file");
		}
		string s(temp);
		delete [] temp;
		return s;
	}
	return "";
}
template<class T> static bool load_nc_array(const NcFile& ncf, const string& name, vector<T>& dest, bool required = true, int offset = 0, int count = -1)
{
	NcVar *v = load_nc_variable(ncf, name.c_str(), required);
	if (v)
	{
		vector<long> offsets = list_of(offset).repeat(v->num_dims()-1, 0);
		v->set_cur(&offsets.front());
		vector<long> counts (v->num_dims());
		long* shape = v->edges();
		transform(shape, shape + v->num_dims(), offsets.begin(), counts.begin(), minus<long>());
		delete shape;
		if (count > 0)
		{
			counts[0] = count;
		}
		dest.resize(product(counts));
		bool success = v->get(&dest.front(), &counts.front());
		if (!success)
		{
			dest.resize(0);
			check(!required, string("NetcdfDataset::load_nc_array<") + typeid(T).name() + "> " + name + '\n' + "failed with offset " + str(offsets) + ", counts " + str(counts));
		}
		return success;
	}
	return false;
}
template<class T> static vector<T> get_nc_array_step(const NcFile& ncf, const string& name, int offset = 0, bool required = true)
{
	vector<T> dest;
	load_nc_array(ncf, name, dest, required, offset, 1);
	return dest;
}

struct DataHeader
{
	//data
	int numDims;
	bimap<int, string> inputLabels;
	bimap<int, string> targetLabels;
	map<string, int> targetLabelCounts;
	int inputSize;
	int outputSize;
	int numSequences;
	int numTimesteps;
	int totalTargetStringLength;

	//functions
	DataHeader(const string& filename, const string& task, double dataFraction):
		outputSize(0),
		numTimesteps(0),
		totalTargetStringLength(0)
	{
		NcFile nc(filename.c_str(), NcFile::ReadOnly);
		check(nc.is_valid(), "can't open data file " + filename);
		NcError err(NcError::silent_nonfatal);
		numDims = load_nc_dim(nc, "numDims", false);
		if (!numDims)
		{
			numDims = 1;
		}
		inputSize = load_nc_dim(nc, "inputPattSize");
		numSequences = load_nc_dim(nc, "numSeqs") * bound(dataFraction, 0.0, 1.0);
		loop(int s, range(numSequences))
		{
			vector<int> seqDims = get_nc_array_step<int>(nc, "seqDims", s, numDims != 1);
			if (seqDims.empty())
			{
				seqDims = get_nc_array_step<int>(nc, "seqLengths", s);
			}
			numTimesteps += product(seqDims);
		}
		if (get_nc_string(nc, "inputLabels", 0, false) != "")
		{
			for (int i = 0; i < inputSize; ++i)
			{
				inputLabels.insert(bimap<int, string>::relation(i, get_nc_string(nc, "inputLabels", i)));
			}
		}
		if (in(task, "regression"))
		{
			outputSize = load_nc_dim(nc, "targetPattSize");
		}
		else if (task == "memory" || task == "code")
		{
			outputSize = inputSize;
		}
		else if (task == "classification" || task == "sequence_classification" || task == "transcription" || task == "dictionary_transcription")
		{
			outputSize = load_nc_dim(nc, "numLabels");
			for (int i = 0; i < outputSize; ++i)
			{
				targetLabels.insert(bimap<int, string>::relation(i, get_nc_string(nc, "labels", i)));
			}
		}
		if (task == "classification")
		{	
			vector<int> targetClasses;
			load_nc_array(nc, "targetClasses", targetClasses);
			loop(int t, targetClasses)
			{
				if (t >= 0)
				{
					++targetLabelCounts[targetLabels.left.at(t)];
				}
			}
		}
		else if (task == "sequence_classification" || task == "transcription" || task == "dictionary_transcription")
		{
			loop(int i, range(outputSize))
			{
				targetLabelCounts[targetLabels.left.at(i)] = 0;
			}
			loop(int s, range(numSequences))
			{
				stringstream labelSeq (get_nc_string(nc, "targetStrings", s));
				string label; 
				while(labelSeq >> label)
				{
					check(in_right(targetLabels, label), "label \'" + label + "\' in \'" + labelSeq.str() + "\' not found in target labels");
					++targetLabelCounts[label];
					++totalTargetStringLength;
				}
			}
		}
	}
	void print(ostream& out) const
	{
		PRINT(numDims, out);
		PRINT(inputSize, out);
		if (outputSize >= 0)
		{
			PRINT(outputSize, out);
		}
		PRINT(numSequences, out);
		PRINT(numTimesteps, out);
		if (targetLabels.size())
		{
			prt_line(out);
			out << targetLabels.size() << " target labels:" << endl;
			for (BMISLCI it = targetLabels.left.begin(); it != targetLabels.left.end(); ++it)
			{
				out << (*it) << " (" << at(targetLabelCounts, it->second) << ")" << endl;
			}
		}
		if (inputLabels.size())
		{
			prt_line(out);
			out << "input labels:" << endl << inputLabels;
		}
		
	}
};

static ostream& operator << (ostream& out, const DataHeader& dh)
{
	dh.print(out);
	return out;
}

struct NetcdfDataset
{
	//data
	NcFile nc;
	string filename;
	string task;
	DataHeader header;	
	vector<DataSequence*> sequences;
	SeqBuffer<int> inputSeqDims;
	SeqBuffer<int> targetSeqDims;
	NcError err;

	//functions
	NetcdfDataset(const string& fname, const string& t, double fraction = 1.0):
		nc (fname.c_str(), NcFile::ReadOnly),
		filename(fname),
		task(t),
		header(filename, task, fraction),
		err(NcError::silent_nonfatal)
	{
		init();
		load_sequences(0, bound(fraction, 0.0, 1.0) * load_dim("numSeqs"));
	}
	NetcdfDataset(const string& fname, const string& t, int seqNum):
		nc (fname.c_str(), NcFile::ReadOnly),
		filename(fname),
		task(t),
		header(filename, task, 0),
		err(NcError::silent_nonfatal)
	{
		init();
		load_sequences(seqNum, seqNum + 1);
	}
	~NetcdfDataset()
	{
		delete_range(sequences);
	}
	void init()
	{
		check(nc.is_valid(), "can't open data file " + filename);
		inputSeqDims.reshape_with_depth(list_of<int>(load_dim("numSeqs")), header.numDims);
 		if (!load_array("seqDims", inputSeqDims.data, header.numDims != 1))
		{
			load_array("seqLengths", inputSeqDims.data);
		}
		targetSeqDims.reshape_with_depth(list_of<int>(load_dim("numSeqs")), header.numDims);
		if (!load_array("targetSeqDims", targetSeqDims.data))
		{
			targetSeqDims = inputSeqDims;
		}
	}
	int size() const
	{
		return sequences.size();
	}
	void shuffle_sequences()
	{
		shuffle(sequences);
	}
	DataSequence& operator[](int n)
	{
		return *sequences.at(n);
	}
	int timesteps() const
	{
		int total = 0;
		loop(const DataSequence* seq, sequences)
		{
			total += seq->inputs.seq_size();
		}
		return total;
	}
	pair<int,int> seq_to_offset(int seqNum) const
	{
		return make_pair(product(inputSeqDims[seqNum]), product(targetSeqDims[seqNum]));
	}
	pair<int,int> get_offset(int seqNum) const
	{
		pair<int, int> offset(0, 0);
		loop(int i, range(seqNum))
		{
			offset += seq_to_offset(i);
		}
		return offset;
	}
	void load_sequences (int first, int last)
	{
		pair<int, int> offsets = get_offset(first);
		loop(int i, range(first, last))
		{
			check(i >= 0 && i < inputSeqDims.shape[0], "sequence " + str(i) + " requested from data file " + str(filename) + " containing " + str(inputSeqDims.shape[0]) + " sequences");
			DataSequence* seq = new DataSequence(header.inputSize, in(task, "regression") ? header.outputSize : 0);
			vector<int> inputShape = flip(inputSeqDims[i]);
			int inputCount = product(inputShape);
			vector<int> targetShape = flip(targetSeqDims[i]);
			int targetCount = product(targetShape);
			load_to_seq_buffer(seq->inputs, inputShape, "inputs", true, offsets.first, inputCount);
// 			if (find_variable("importance"))
// 			{
// 				load_to_seq_buffer(seq->importance, targetShape, "importance", true, offsets.second, targetCount);
// 			}
			if (in(task, "regression"))
			{
				if (task == "sequence_regression")
				{
					targetShape.clear();
				}
				load_to_seq_buffer(seq->targetPatterns, targetShape, "targetPatterns", true, offsets.second, targetCount);
			}
			else if (task == "classification")
			{
				load_to_seq_buffer(seq->targetClasses, targetShape, "targetClasses", true, offsets.second, targetCount);
			}
			else if (task == "sequence_classification" || task == "transcription" || task == "dictionary_transcription")
			{
				seq->labelSeq = str_to_label_seq(get_string("targetStrings", i), header.targetLabels);
				if (task == "sequence_classification")
				{
					seq->targetClasses.get(list_of(0)) = seq->labelSeq[0];
				}
			}
			seq->tag = get_string("seqTags", i, false);
			sequences.push_back(seq);
			offsets += make_pair(inputCount, targetCount);
		}
	}
	bool find_variable(const string& name)
	{
		NcVar *v = load_variable(name, false);
		bool ret = (v != 0);
		return ret;
	}
	NcVar* load_variable (const string& name, bool required = true)
	{
		return load_nc_variable(nc, name, required);
	}	
	int load_dim(const string& name, bool required = true)
	{
		return load_nc_dim(nc, name, required);
	}
	string get_string(const string& name, int offset = 0, bool required = true)
	{
		return get_nc_string(nc, name, offset, required);
	}
	template<class T, class R> bool load_to_seq_buffer(SeqBuffer<T>& dest, const R& shape, const string& name, bool required = true, int offset = 0, int count = -1)
	{
		dest.reshape(shape);
		return load_array(name, dest.data, required, offset, count);
	}
	template<class T> bool load_array(const string& name, vector<T>& dest, bool required = true, int offset = 0, int count = -1)
	{
		return load_nc_array<T>(nc, name, dest, required, offset, count);
	}
	void print(ostream& out) const
	{
		PRINT(filename, out);
		out << sequences.size() << " sequences" << endl;
		out << timesteps() << " timesteps" << endl;
		header.print(out);
	}
};

static ostream& operator << (ostream& out, const NetcdfDataset& d)
{
	d.print(out);
	return out;
}

struct DataList
{
	//data
	vector<string> filenames;
	vector<DataHeader> headers;
	map <string, double> targetLabelFrequencies;
	string task;
	int numSequences;
	int numTimesteps;
	int totalTargetStringLength;
	NetcdfDataset* dataset;
	int datasetIndex;
	DataSequence* seq;
	int seqIndex;
	double dataFraction;
	bool shuffled;
	
	//functions
	DataList(const vector<string>& filenams, const string& t, bool shuffle, double loadFrac):
	filenames(filenams),
	task(t),
	numSequences(0),
	numTimesteps(0),
	totalTargetStringLength(0),
	dataset(0),
	datasetIndex(-1),
	seq(0),
	seqIndex(-1),	
	dataFraction(loadFrac),
	shuffled(shuffle)
	{
		for (int i = 0; i < filenames.size(); ++i)
		{
			headers += DataHeader(filenames[i], task, loadFrac);
			const DataHeader& curr = headers.back();
			numSequences += curr.numSequences;
			numTimesteps += curr.numTimesteps;
			totalTargetStringLength += curr.totalTargetStringLength;
			targetLabelFrequencies += curr.targetLabelCounts;
			if (i)
			{
				const DataHeader& prev = nth_last(headers, 2);
				assert(prev.numDims == curr.numDims);
				assert(prev.targetLabels == curr.targetLabels);
				assert(prev.inputSize == curr.inputSize);
				assert(prev.outputSize == curr.outputSize);
			}
		}
		targetLabelFrequencies /= (double) sum_right(targetLabelFrequencies);
	}
	~DataList()
	{
		delete dataset;
	}
	void init()
	{
	}
	void delete_dataset()
	{
		seqIndex = -1;
		delete dataset;
		dataset = 0;
		seq = 0;		
	}
	bool next_dataset()
	{
		delete_dataset();
		if (datasetIndex >= (int)last_index(filenames))
		{
			datasetIndex = -1;
			return true;
		}
		++datasetIndex;
		dataset = new NetcdfDataset(filenames[datasetIndex], task, dataFraction);
		if (!dataset->size())
		{
			return next_dataset();
		}
		if (shuffled)
		{
			dataset->shuffle_sequences();
		}
		return false;
	}
	DataSequence* next_sequence()
	{
		bool finished = false;
		if (!dataset || seqIndex >= (int)last_index(dataset->sequences))
		{
			finished = next_dataset();
		}
		if (!finished)
		{
			++seqIndex;
			seq = dataset->sequences[seqIndex];
		}
		return seq;
	}
	DataSequence* start()
	{
		datasetIndex = -1;
		delete_dataset();
		if (shuffled)
		{
			shuffle(filenames);
		}
		return next_sequence();
	}
	int size() const
	{
		return filenames.size();
	}
	void print(ostream& out = cout) const
	{
		PRINT(numSequences, out);
		PRINT(numTimesteps, out);
		if(verbose)
		{
			out << "avg timesteps/seq = " << (double) numTimesteps / (double)numSequences << endl;
		}
		if (dataFraction != 1)
		{
			PRINT(dataFraction, out);
		}
		out << filenames.size() << " filenames"<< endl;
		print_range(out, filenames, string("\n"));
		out << endl;
		if (verbose)
		{
			out << "inputSize = " << headers.front().inputSize << endl;
			out << "outputSize = " << headers.front().outputSize << endl;
			out << "numDims = " << headers.front().numDims << endl;
			PRINT(task, out);
			PRINT(shuffled, out);
			const bimap<int, string>& targetLabels = headers.front().targetLabels;
			const bimap<int, string>& inputLabels = headers.front().inputLabels;
			if (inputLabels.size())
			{
				out << inputLabels.size() << " input labels" << endl;
				out << inputLabels;
			}
			if (targetLabels.size())
			{
				out << targetLabels.size() << " target labels" << endl;
				for (BMISLCI it = targetLabels.left.begin(); it != targetLabels.left.end(); ++it)
				{
					out << (*it) << " (" << setprecision (3) << at(targetLabelFrequencies, it->second) * 100 << "%)" << endl;
				}
				PRINT(totalTargetStringLength, out);
			}
		}
	}
};

static ostream& operator << (ostream& out, const DataList& dl)
{
	dl.print(out);
	return out;
}

#endif
