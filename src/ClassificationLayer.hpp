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

#ifndef _INCLUDED_ClassificationLayer_h  
#define _INCLUDED_ClassificationLayer_h  

#include <boost/bimap.hpp>
#include "SoftmaxLayer.hpp"

struct ClassificationLayer: public SoftmaxLayer
{
	//data
	ostream& out;
	const bimap<int, string>& labels;
	SeqBuffer<int> targets;
	vector<vector<int> > confusionMatrix;
	vector<int> classErrors;
	vector<int> classTargets;
	vector<pair<double, int> >labelProbs;
	
	//functions
	ClassificationLayer(ostream& o, const string& name, size_t numSeqDims, const bimap<int, string>& lab):
		SoftmaxLayer(name, numSeqDims, lab.size()),
		out(o),
		labels(lab),
		targets(this->output_size()),	
		confusionMatrix(this->output_size()),
		classErrors(this->output_size()),
		classTargets(this->output_size()),
		labelProbs(this->output_size())
	{
		loop(vector<int>& v, confusionMatrix)
		{
			v.resize(this->output_size());
		}
		this->criteria = list_of("crossEntropyError")("classificationError");
		display(targets, "targets", &labels);
		display(this->inputErrors, "inputErrors", &labels);
		display(this->inputErrors, "outputErrors", &labels);
		display(this->inputActivations, "inputActivations", &labels);
		display(this->outputActivations, "outputActivations", &labels);
	}
	double calculate_errors(const DataSequence& seq)
	{
		assert(equal(seq.targetClasses.seq_shape(), this->outputActivations.seq_shape()));
		loop(vector<int>& v, confusionMatrix)
		{
			fill(v, 0);
		}
		targets.reshape(this->outputActivations, 0);
		double crossEntropyError = 0;
		loop(int i, range(this->outputActivations.seq_size()))
		{
			int targetClass = seq.targetClasses[i].front();			
			if (targetClass >= 0)
			{
				View<int> targs = targets[i];	
				targs[targetClass] = 1;
				View<double> acts = this->outputActivations[i];
				loop(TDDI t, zip(this->outputErrors[i], acts, targs))
				{
					double act = t.get<1>();
					double targ = t.get<2>();
					t.get<0>() = -targ/act;
					if (targ)
					{
						crossEntropyError -= targ * log(act/targ);
					}
				}
				int outputClass = max_index(acts);
				++confusionMatrix[targetClass][outputClass];
			}
		}
		this->errorMap.clear();
		for (int i = 0; i < confusionMatrix.size(); ++i)
		{
			vector<int>& v = confusionMatrix[i];
			classTargets[i] = sum(v);
			classErrors[i] = classTargets[i] - v[i];
		}
		double numTargets = sum(classTargets);
		if (numTargets)
		{
			this->errorMap["crossEntropyError"] = crossEntropyError;
			this->errorMap["classificationError"] = sum(classErrors) / numTargets;
			for (int i = 0; i < confusionMatrix.size(); ++i)
			{
				if (classTargets[i])
				{
					this->errorMap["_" + labels.left.at(i)] = classErrors[i] / numTargets;
					if(verbose)
					{
						vector<int>& v = confusionMatrix[i];
						for (int j = 0; j < v.size(); ++j)
						{
							if (j != i)
							{
								this->errorMap["_" + labels.left.at(i) + "->" + labels.left.at(j)] = v[j] / numTargets;
							}
						}
					}
				}
			}
		}
		if (verbose)
		{
			out << "sorted log probs:" << endl;
			if (this->num_seq_dims() == 0)
			{
				View<LogDouble> logActs = this->logActivations[0];
				loop (int i, range(this->output_size()))
				{
					labelProbs[i] = make_pair(logActs[i].log, i);
				}
				reverse_sort(labelProbs);
				for (int i = 0; i < this->output_size(); ++i)
				{
					out << labels.left.at(labelProbs[i].second) << " " << labelProbs[i].first << endl;
				}
			}
		}
		return crossEntropyError;
	}
};

#endif
