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

#ifndef _INCLUDED_LstmLayer_h
#define _INCLUDED_LstmLayer_h

#include "Layer.h"
#include "Matrix.h"
#include "WeightContainer.h"
#define PEEPS

template <class CI, class CO, class G> struct LstmLayer: public Layer
{
	//data
	size_t numBlocks;
	size_t cellsPerBlock;
	size_t numCells;
	size_t gatesPerBlock;
	size_t unitsPerBlock;
	size_t peepsPerBlock;
	SeqBuffer<double> inGateActs;
	SeqBuffer<double> forgetGateActs;
	SeqBuffer<double> outGateActs;
	SeqBuffer<double> preOutGateActs;
	SeqBuffer<double> states;
	SeqBuffer<double> preGateStates;
	SeqBuffer<double> cellErrors;
	vector<vector<int> > stateDelays;
	vector<int> delayedCoords;
	vector<View<double> > oldStates;
	vector<View<double> > nextErrors;
	vector<View<double> > nextFgActs;
	vector<View<double> > nextCellErrors;
#ifdef PEEPS
	pair<size_t, size_t> peepRange;
#endif		
	
	//functions
	LstmLayer(const string& name, const vector<int>& directions, size_t nb, size_t cpb = 1):
			Layer(name, directions, (cpb + directions.size() + 2) * nb, nb),	
			numBlocks(nb),
			cellsPerBlock(cpb),
			numCells(numBlocks * cellsPerBlock),
			gatesPerBlock(this->num_seq_dims() + 2),
			unitsPerBlock(gatesPerBlock + cellsPerBlock),
			peepsPerBlock(gatesPerBlock * cellsPerBlock),
			inGateActs(numBlocks),
			forgetGateActs(numBlocks * this->num_seq_dims()),
			outGateActs(numBlocks),
			preOutGateActs(numCells),
			states(numCells),
			preGateStates(numCells),
			cellErrors(numCells),	
			stateDelays(this->num_seq_dims()),
			delayedCoords(this->num_seq_dims()),
			oldStates(this->num_seq_dims()),
			nextErrors(this->num_seq_dims()),
			nextFgActs(this->num_seq_dims()),
			nextCellErrors(this->num_seq_dims())
#ifdef PEEPS
			,peepRange(WeightContainer::instance().new_parameters(peepsPerBlock*numBlocks, name, name, name + "_peepholes"))
#endif
	{		
		//initialise the state delays
		loop(int i, range(this->num_seq_dims()))
		{
			stateDelays[i].resize(this->num_seq_dims(), 0);
			stateDelays[i][i] = -directions[i];
		}
		//export the data
		display(this->inputActivations, "inputActivations");
		display(this->outputActivations, "outputActivations");
		display(this->inputErrors, "inputErrors");
		display(this->outputErrors, "outputErrors");
		DISPLAY(cellErrors);
		DISPLAY(states);
		DISPLAY(inGateActs);
		DISPLAY(forgetGateActs);
		DISPLAY(outGateActs);
	}	
	~LstmLayer()
	{
	}
	void start_sequence()
	{
		Layer::start_sequence();
		inGateActs.reshape(this->output_seq_shape());
		forgetGateActs.reshape(this->output_seq_shape());
		outGateActs.reshape(this->output_seq_shape());
		preOutGateActs.reshape(this->output_seq_shape());
		states.reshape(this->output_seq_shape());
		preGateStates.reshape(this->output_seq_shape());
		cellErrors.reshape(this->output_seq_shape());
	}
	void feed_forward(const vector<int>& coords)
	{
		double* actBegin = this->outputActivations[coords].begin();
		double* inActIt = this->inputActivations[coords].begin();
		double* inGateActBegin = inGateActs[coords].begin();
		double* fgActBegin = forgetGateActs[coords].begin();
		double* outGateActBegin = outGateActs[coords].begin();
		double* stateBegin = states[coords].begin();
		double* preGateStateBegin = preGateStates[coords].begin();
		double* preOutGateActBegin = preOutGateActs[coords].begin();
		loop(int d, range(this->num_seq_dims()))
		{
			oldStates[d] = states.at(range_plus(delayedCoords, coords, stateDelays[d]));
		}
#ifdef PEEPS
		const double* peepWtIt = WeightContainer::instance().get_weights(peepRange).begin();
#endif
		int cellStart = 0;
		int cellEnd = cellsPerBlock;
		double* fgActEnd = fgActBegin + this->num_seq_dims();
		loop(int b, range(numBlocks))
		{
#ifdef PEEPS
			View<double> fgActs(fgActBegin, fgActEnd);
			//input gate
			//extra inputs from peepholes (from old states)
			loop(const View<double>& os, oldStates)
			{
				if (os.begin())
				{
					dot(os.begin() + cellStart, os.begin() + cellEnd, peepWtIt, inActIt, inActIt + 1);
				}
			}
			peepWtIt += cellsPerBlock;
#endif	
			double inGateAct = G::fn(*inActIt);
			inGateActBegin[b] = inGateAct;
			++inActIt;
			
			//forget gates
			//extra inputs from peepholes (from old states)	
			loop(int d, range(this->num_seq_dims()))
			{
#ifdef PEEPS
				const View<double>& os = oldStates[d];
				if (os.begin())
				{	
					dot(os.begin() + cellStart, os.begin() + cellEnd, peepWtIt, inActIt, inActIt + 1);
				}
				peepWtIt += cellsPerBlock;
#endif
				fgActs[d] = G::fn(*inActIt);
				++inActIt;
			}
			
			//pre-gate cell states
			transform(inActIt, inActIt + cellsPerBlock, preGateStateBegin + cellStart, CI::fn);
			inActIt += cellsPerBlock;
			
			//cell states
			loop(int c, range(cellStart, cellEnd))
			{
				double state = inGateAct * preGateStateBegin[c];
				loop(int d, range(this->num_seq_dims()))
				{
					const View<double>& os = oldStates[d];
					if (os.begin())
					{
						state += fgActs[d] * os[c];
					}
				}
				stateBegin[c] = state;
				preOutGateActBegin[c] = CO::fn(state);
			}
			
			//output gate
			//extra input from peephole (from current state)
#ifdef PEEPS			
			dot(stateBegin + cellStart, stateBegin + cellEnd, peepWtIt, inActIt, inActIt + 1);
			peepWtIt += cellsPerBlock;
#endif
			
			double outGateAct = G::fn(*inActIt);
			outGateActBegin[b] = outGateAct;
			++inActIt;
			
			//output activations
			transform(preOutGateActBegin + cellStart, preOutGateActBegin + cellEnd, actBegin + cellStart, 
					  bind2nd(multiplies<double>(), outGateAct));
			cellStart = cellEnd;
			cellEnd += cellsPerBlock;
			fgActBegin = fgActEnd;
			fgActEnd += this->num_seq_dims();
		}
	}
	void feed_back(const vector<int>& coords)
	{
		//activations
		const double* inGateActBegin = inGateActs[coords].begin();
		const double* forgetGateActBegin = forgetGateActs[coords].begin();
		const double* outGateActBegin = outGateActs[coords].begin();
		const double* preGateStateBegin = preGateStates[coords].begin();
		const double* preOutGateActBegin = preOutGateActs[coords].begin();
		
		//errors
		View<double> inErrs = this->inputErrors[coords];
		double* cellErrorBegin = cellErrors[coords].begin();
		const double* outputErrorBegin = this->outputErrors[coords].begin();
		double* errorIt = inErrs.begin();
#ifdef PEEPS
		const double* peepWtIt = WeightContainer::instance().get_weights(peepRange).begin();
#endif
		loop(int d, range(this->num_seq_dims()))
		{
			oldStates[d] = states.at(range_plus(delayedCoords, coords, stateDelays[d]));
			range_minus(delayedCoords, coords, stateDelays[d]);
			nextErrors[d] = this->inputErrors.at(delayedCoords);
			nextFgActs[d] = forgetGateActs.at(delayedCoords);
			nextCellErrors[d] = cellErrors.at(delayedCoords);
		}
		int cellStart = 0;
		int cellEnd = cellsPerBlock;
		int fgStart = 0;
		int gateStart = 0;
		loop(int b, range(numBlocks))
		{
			double inGateAct = inGateActBegin[b];
			double outGateAct = outGateActBegin[b];
			
			//output gate error
			double outGateError = G::deriv(outGateAct) * 
				inner_product(preOutGateActBegin + cellStart, preOutGateActBegin + cellEnd, outputErrorBegin + cellStart, 0.0);
			
			//cell pds (dE/dState)
			loop(int c, range(cellStart, cellEnd))
			{
				double deriv = (CO::deriv(preOutGateActBegin[c]) * outGateAct * outputErrorBegin[c]);
#ifdef PEEPS
				int cOffset = c - cellStart;
				double igPeepWt = peepWtIt[cOffset];
				double ogPeepWt = peepWtIt[peepsPerBlock - cellsPerBlock + cOffset];
				deriv += outGateError * ogPeepWt;
#endif
				loop(int d, range(this->num_seq_dims()))
				{
#ifdef PEEPS
					double fgPeepWt = peepWtIt[cOffset + (cellsPerBlock * (d + 1))];
#endif
					const View<double>& nextErrs = nextErrors[d];
					if (nextErrs.begin())
					{
#ifdef PEEPS
						deriv += (nextErrs[gateStart + 1 + d] * fgPeepWt) + (nextErrs[gateStart] * igPeepWt);
#endif
						deriv += (nextFgActs[d][fgStart + d] * nextCellErrors[d][c]);
					}
				}
				cellErrorBegin[c] = deriv;
			}
			
			//input gate error
			*errorIt = G::deriv(inGateAct) * 
				inner_product(cellErrorBegin + cellStart, cellErrorBegin + cellEnd, preGateStateBegin + cellStart, 0.0);
			++errorIt;
			
			//forget gate error
			loop(int d, range(this->num_seq_dims()))
			{
				const View<double>& os = oldStates[d];
				if (os.begin())
				{
					*errorIt = G::deriv(forgetGateActBegin[fgStart + d]) * 
						inner_product(cellErrorBegin + cellStart, cellErrorBegin + cellEnd, os.begin() + cellStart, 0.0);
				}
				else
				{
					*errorIt = 0;
				}
				++errorIt;				
			}
			
			//cell errors
			loop(int c, range(cellStart, cellEnd))
			{
				*errorIt = inGateAct * CI::deriv(preGateStateBegin[c]) * cellErrorBegin[c];
				++errorIt;
			}
			*errorIt = outGateError;
			++errorIt;
#ifdef PEEPS
			peepWtIt += peepsPerBlock;
#endif
			cellStart += cellsPerBlock;
			cellEnd += cellsPerBlock;
			fgStart += this->num_seq_dims();
			gateStart += unitsPerBlock;
		}
		
		//constrain errors to be in [-1,1] for stability
		if (!runningGradTest)
		{
			bound_range(inErrs, -1.0, 1.0);
		}
	}
#ifdef PEEPS
	void update_derivs(const vector<int>& coords)
	{
		const double* stateBegin = states[coords].begin();
		const double* errorBegin = this->inputErrors[coords].begin();
		double* pdIt = WeightContainer::instance().get_derivs(peepRange).begin();
		loop(int d, range(this->num_seq_dims()))
		{
			oldStates[d] = states.at(range_plus(delayedCoords, coords, stateDelays[d]));
		}
		loop(int b, range(numBlocks))
		{
			int cellStart = b * cellsPerBlock;
			int cellEnd = cellStart + cellsPerBlock;
			int errorOffset = b * unitsPerBlock;
			double inGateError = errorBegin[errorOffset];
			loop(int d, range(this->num_seq_dims()))
			{
				const View<double>& os = oldStates[d];
				if (os.begin())
				{	
					loop(int c, range(cellStart, cellEnd))
					{
						pdIt[c - cellStart] += inGateError * os[c];
					}
					double forgGateError = errorBegin[errorOffset + d + 1];
					loop(int c, range(cellStart, cellEnd))
					{
						pdIt[(c - cellStart) + ((d + 1) * cellsPerBlock)] += forgGateError * os[c];
					}
				}
			}
			double outGateError = errorBegin[errorOffset + unitsPerBlock - 1];
			loop(int c, range(cellStart, cellEnd))
			{
				pdIt[(c - cellStart) + peepsPerBlock - cellsPerBlock] += outGateError * stateBegin[c];
			}
			pdIt += peepsPerBlock;
		}
	}
	void print(ostream& out = cout) const
	{
		Layer::print(out);
		out << " " << difference(peepRange) << " peeps";
	}
#endif
};

#endif
