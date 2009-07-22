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

#ifndef _INCLUDED_DataSequence_h  
#define _INCLUDED_DataSequence_h  

#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <boost/bimap.hpp>
#include "Helpers.h"
#include "SeqBuffer.h"

#define PRINT_ARRAY_SHAPE(a, o) (o << #a ": shape = [" << a.shape << "], size = " << a.seq_size() << endl)

template<class R> static string label_seq_to_str(const R& labelSeq, const bimap<int, string>& labels, const string& delim = " ")
{
	if (!boost::size(labelSeq))
	{
		return "";
	}
	stringstream ss;
	typename range_const_iterator<R>::type it = boost::begin(labelSeq);
	ss << labels.left.at(*it);
	for (++it; it != boost::end(labelSeq); ++it)
	{
		ss << delim << labels.left.at(*it);
	}
	return ss.str();
}
static vector<int> str_to_label_seq(const string& labelSeqString, const bimap<int, string>& labels)
{
	static vector<int> v;
	v.clear();
	stringstream ss(labelSeqString);
	string lab;
	while(ss >> lab)
	{
		check(in_right(labels, lab), lab + " not found in labels");
		v += labels.right.at(lab);
	}
	return v;
}

struct DataSequence
{
	//data
	SeqBuffer<float> inputs;
	SeqBuffer<float> targetPatterns;
	SeqBuffer<int> targetClasses;
//	SeqBuffer<float> importance;
	vector<int> labelSeq;
	string tag;
	
	//functions
	DataSequence(const DataSequence& ds):
		inputs(ds.inputs),
		targetPatterns(ds.targetPatterns),
		targetClasses(ds.targetClasses),
//		importance(ds.importance),
		labelSeq(ds.labelSeq),
		tag(ds.tag)
	{
	}
	DataSequence(size_t inputDepth = 0, size_t targetPattDepth = 0, size_t targetClassDepth = 1):
		inputs(inputDepth),
		targetPatterns(targetPattDepth),
		targetClasses(targetClassDepth)
//		importance(1)
	{
	}
	size_t num_timesteps() const
	{
		return inputs.seq_size();
	}	
	void print(ostream& out, const bimap<int, string>* labels = 0) const
	{
		PRINT(tag, out);
		out << "input shape = (" << inputs.shape << ")" << endl;
		out << "timesteps = " << inputs.seq_size() << endl;
		if (labelSeq.size() && labels)
		{
			out << "target labels:" << endl;
			out << label_seq_to_str(this->labelSeq, *labels) << endl;
		}
		if (targetPatterns.size())
		{
			out << "target shape = (" << targetPatterns.shape << ")" << endl;
		}
	}
};
static ostream& operator <<(ostream& out, const DataSequence& seq)
{
	seq.print(out);
	return out;
}

#endif
