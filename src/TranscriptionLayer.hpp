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

#ifndef _INCLUDED_TranscriptionLayer_h  
#define _INCLUDED_TranscriptionLayer_h  

#include <boost/bimap.hpp>
#include "SoftmaxLayer.hpp"
#include "StringAlignment.hpp"

struct TranscriptionLayer: public SoftmaxLayer
{	
	//data
	ostream& out;
	bimap <int, string> labels;
	SeqBuffer<LogDouble> forwardVariables;
	SeqBuffer<LogDouble> backwardVariables;
	int blank;
	int totalSegments;
	int totalTime;
	vector<LogDouble> dEdYTerms;
	
	//functions
	TranscriptionLayer(ostream& o, const string& name, const bimap<int, string>& lab):
		SoftmaxLayer(name, 1, lab.size() + 1),
		out(o),
		labels(lab),
		blank(labels.size()),
		dEdYTerms(this->output_size())
	{
		labels.insert(bimap<int, string>::relation(blank, "blank"));
		this->criteria = list_of("labelError")("sequenceError")("ctcError");
		display(this->inputErrors, "inputErrors", &labels);
		display(this->outputErrors, "outputErrors", &labels);
		display(this->inputActivations, "inputActivations", &labels);
		display(this->outputActivations, "outputActivations", &labels);
		display(this->forwardVariables, "forwardVariables", &labels);
		display(this->backwardVariables, "backwardVariables", &labels);
	}	
	virtual ~TranscriptionLayer()
	{
	}
	pair<counting_iterator<int>, counting_iterator<int> > segment_range(int time, int totalSegs = -1) const
	{
		if (totalSegs < 0)
		{
			totalSegs = totalSegments;
		}
        return ::range<int>(max(0, totalSegs - (2 *(totalTime-time))), min(totalSegs, 2 * (time + 1)));
	}
	vector<int>& path_to_string(const vector<int>& path) const
	{
		static vector<int> str;
		str.clear();
		int prevLabel = -1;
		loop (int label, path)
		{
			if (label != blank && (str.empty() || label != str.back() || prevLabel == blank))
			{
				str += label;
			}
			prevLabel = label;
		}
		return str;
	}
	vector<int>& best_label_seq() const
	{
		static vector<int> path;
		path.clear();
		loop(int i, ::range(this->outputActivations.seq_size()))
		{
			path += max_index(this->outputActivations[i]);
		}
		return path_to_string(path);
	}
	double calculate_errors(const DataSequence& seq)
	{
		totalTime = this->outputActivations.seq_size();
		int requiredTime = seq.labelSeq.size();
		int oldLabel = -1;
		loop(int label, seq.labelSeq)
		{
			if (label == oldLabel)
			{
				++requiredTime;
			}
			oldLabel = label;
		}
		if (totalTime < requiredTime)
		{
			out << "warning, seq " << seq.tag << " has requiredTime " << requiredTime << " > totalTime " << totalTime << endl;
			return 0;
		}		
		totalSegments = (seq.labelSeq.size() * 2) + 1;
		
		//calculate the forward variables
		forwardVariables.reshape_with_depth(list_of(totalTime), totalSegments, 0);
		forwardVariables.data[0] = this->logActivations.data[blank];
		if (totalSegments > 1)
		{
			forwardVariables.data[1] = this->logActivations.data[seq.labelSeq[0]];
		}
		loop(int t, ::range(1, totalTime))
		{
			View<LogDouble> logActs = this->logActivations[t];
			View<LogDouble> oldFvars = forwardVariables[t-1];
			View<LogDouble> fvars = forwardVariables[t];
			loop(int s, segment_range(t))
			{
				LogDouble fv;
				//s odd (label output)
				if (s & 1)
				{
					int labelIndex = s/2;
					int labelNum = seq.labelSeq[labelIndex];
					int lastLabelNum = seq.labelSeq[labelIndex-1];
					fv = oldFvars[s] + oldFvars[s-1];
					if (s > 1 && (labelNum != lastLabelNum))
					{
						fv += oldFvars[s-2];
					}
					fv *= logActs[labelNum];
				}
				//s even (blank output)
				else
				{
					fv = oldFvars[s];
					if (s)
					{
						fv += oldFvars[s-1];
					}
					fv *= logActs[blank];
				}
				fvars[s] = fv;
			}
		}
		View<LogDouble> lastFvs = forwardVariables[totalTime - 1];
		LogDouble logProb = lastFvs.back();
		if (totalSegments > 1)
		{
			logProb += nth_last(lastFvs, 2);
		}
		check(logProb.log <= 0, "sequence\n" + str(seq) + "has log probability " + str(logProb.log));

		//calculate the backward variables
		backwardVariables.reshape_with_depth(list_of(totalTime), totalSegments, LogDouble());
		View<LogDouble> lastBvs = backwardVariables[totalTime - 1];
		lastBvs.back() = 1;
		if (totalSegments > 1)
		{
			nth_last(lastBvs, 2) = 1;
		}
		//loop over time, calculating backward variables recursively
		loop_back(int t, ::range(totalTime - 1))
		{
			View<LogDouble> oldLogActs = this->logActivations[t+1];
			View<LogDouble> oldBvars = backwardVariables[t+1];
			View<LogDouble> bvars = backwardVariables[t];
			loop(int s, segment_range(t))
			{
				LogDouble bv;
				
				//s odd (label output)
				if (s&1)
				{
					int labelIndex = s/2;
					int labelNum = seq.labelSeq[labelIndex];
					int nextLabelNum = seq.labelSeq[labelIndex + 1];
					bv = (oldBvars[s] * oldLogActs[labelNum]) + (oldBvars[s + 1] * oldLogActs[blank]);
					if ((s < (totalSegments-2)) && (labelNum != nextLabelNum))
					{
						bv += (oldBvars[s+2] * oldLogActs[nextLabelNum]);
					}
				}
				
				//s even (blank output)
				else
				{
					bv = oldBvars[s] * oldLogActs[blank];
					if (s < (totalSegments-1))
					{
						bv += (oldBvars[s+1] * oldLogActs[seq.labelSeq[s/2]]);
					}
				}
				bvars[s] = bv;
			}
		}
		//inject the training errors
		loop(int time, ::range(totalTime))
		{
			fill(dEdYTerms, LogDouble(0));
			View<LogDouble> fvars = forwardVariables[time];
			View<LogDouble> bvars = backwardVariables[time];
			loop (int s, ::range(totalSegments))
			{
				//k = blank for even s, target label for odd s
				int k = (s&1) ? seq.labelSeq[s/2] : blank;
				dEdYTerms[k] += (fvars[s] * bvars[s]);
			}
			loop(TDLL t, zip(this->outputErrors[time], dEdYTerms, this->logActivations[time]))
			{
				t.get<0>() = -((t.get<1>() / (logProb * t.get<2>())).exp());
			}
		}
		//calculate the aligment errors
		vector<int> outputLabelSeq = best_label_seq();
		StringAlignment<int> alignment(seq.labelSeq, outputLabelSeq);
		double labelError = alignment.distance;
		double substitutions = alignment.substitutions;
		double deletions = alignment.deletions;
		double insertions = alignment.insertions;
		double seqError = labelError ? 1 : 0;
		double ctcError = -logProb.log;
		
		//store errors in map
		this->normFactors["label"] = seq.labelSeq.size(); 
		ERR(labelError);
		ERR(seqError);
		ERR(substitutions);
		ERR(deletions);
		ERR(insertions);
		ERR(ctcError);
		if (verbose)
		{
			out << "target label sequence (length " << seq.labelSeq.size() << "):" << endl << label_seq_to_str(seq.labelSeq, labels) << endl;
			out << "output label sequence (length " << outputLabelSeq.size() << "):" << endl << label_seq_to_str(outputLabelSeq, labels) << endl;
		}
		return ctcError;
	}
};

#endif
