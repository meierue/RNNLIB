#ifndef _INCLUDED_WordTranscriptionLayer_h  
#define _INCLUDED_WordTranscriptionLayer_h  

#include "DecodingLayer.hpp"

struct WordTranscriptionLayer: public DecodingLayer
{	
	//data
	vector<pair<Word*, int> >segmentWordOffsets;

	//functions
	WordTranscriptionLayer(ostream& out, const string& name, const bimap<int, string>& labels, 
				  const string& dictFilename, const string& bigramFilename = "", int nb = 1, int fl = -1):
		DecodingLayer(out, name, labels, dictFilename, bigramFilename, nb, fl)
	{
		loop(PSPW& p, this->dict)
		{
			Word* word = p.second;
			vector<set<int> >& wordLabels = word->competingLabels;
			const vector<int>& labSeq = word->labelSeq;
			loop(TISETI t, enumerate(wordLabels))
			{
				int s = t.get<0>();
				set<int>& segmentLabels = t.get<1>();
				
				//add blank label to every set
				segmentLabels.insert(this->blank);
				
				//add word delimiter to every set, unless fixed to 1 word
				if (fixedLength != 1)
				{
					segmentLabels.insert(labSeq.back());
				}
				
				//add own label if label unit (i.e. prefixLegth odd)
				int labelIndex = s/2;
				if (s&1)
				{ 
					segmentLabels.insert(labSeq[labelIndex]);
				}
				//TODO allow for fixed length > 1 ()
				//requires context dependent competingLabels, so ends of nth words in seq have no competitors
				//could hack into decoder, but tricky
				int prefixLength = (fixedLength == 1 || s < wordLabels.size() - 2) ? (s+1)/2 : 0;
				
				//add all other next letters from words matching prefix up to prefixLength
				loop(PSPW& p2, this->dict)
				{
					Word* word2 = p2.second;
					const vector<int>& labSeq2 = word2->labelSeq;
					if (labSeq2.size() > prefixLength && equal(word->labelSeq.begin(), word->labelSeq.begin() + prefixLength, labSeq2.begin()))
					{
						int nextLabel = labSeq2[prefixLength];
						if ((!(s&1)) || (nextLabel != labSeq[labelIndex]))
						{
							segmentLabels.insert(nextLabel);
						}
					}
				}
			}
			PRT(*word);
			PRTN(wordLabels);
		}
	}
	int segment_to_Label(int s)
	{
		pair<Word*, int> p = segmentWordOffsets[s];
		if (s & 1)
		{
			return p.first->labelSeq[p.second / 2];
		} 
		return blankUnit;
	}
	const set<int>& competing_labels(int s)
	{
		const pair<Word*, int>& p = segmentWordOffsets[s];
		return p.first->competingLabels[p.second];
	}
	LogDouble& log_scale_factor(int t, int s)
	{
		const pair<Word*, int>& p = segmentWordOffsets[s];
		return p.first->log_scale_factor(t, p.second);
	}
	LogDouble& log_act(int t, int s)
	{
		return this->unnormedLogActs[t].first[segmentToLabel(s)];
	}
	double calculate_errors(const DataSequence& seq)
	{
		//FORWARD ALGORITHM
		this->totalTime = this->outputActivations.seq_size();
		vector<Word*> targetWordSeq = this->data_seq_to_words(seq);
		check(!in(targetWordSeq, -1), "out of dict word found in sequence\n" + str(seq));
		segmentWordOffsets.clear();
		loop(Word* w, targetWordSeq)
		{
			loop(int i, ::range(w->totalSegments - 1))
			{
				segmentWordOffsets.push_back (make_pair(w, i));
			}
		}
		segmentWordOffsets.push_back(make_pair(segmentWordOffsets.back().first, segmentWordOffsets.back().second + 1));
		int requiredTime = seq.labelSeq.size() + count_adjacent(seq.labelSeq);
//		int oldLabel = -1;
//		loop(int label, seq.labelSeq)
//		{
//			if (label == oldLabel)
//			{
//				++requiredTime;
//			}
//			oldLabel = label;
//		}
		if (totalTime < requiredTime)
		{
			out << "warning, seq " << seq.tag << " has requiredTime " << requiredTime << " > totalTime " << totalTime << endl;
			return 0;
		}		
		this->totalSegments = (seq.labelSeq.size() * 2) + 1;
		
		//calculate (scaled) forward variables
		this->forwardVariables.reshape_with_depth(list_of(totalTime), totalSegments, 0);
		double initZ = log_scale_factor(0, 0);
		this->forwardVariables[0][0] = log_act(0, 0) - initZ;
		vector<LogDouble> fvars(totalSegments);
		fvars[0] = this->forwardVariables[0][0] / log_scale_factor(1, 0);
		if (totalSegments > 1)
		{
			this->forwardVariables[0][1] = log_act(0, 1) / initZ;
			fvars[1] = this->forwardVariables[0][1] / log_scale_factor(1, 1);
		}
		loop(int t, ::range(1, totalTime))
		{
			loop(int s, segment_range(t))
			{
				LogDouble fv;
				
				//s odd (label output)
				if (s&1)
				{
					fv = fvars[s] + fvars[s-1];
					if ((s > 1) && (segmentToLabel(s) != segmentToLabel(s-2)))
					{
						fv += fvars[s-2];
					}
				}
				//s even (blank output)
				else
				{
					fv = fvars[s];
					if (s)
					{
						fv += fvars[s-1];
					}
				}
				fv *= log_act(t, s);
				forwardVariables[t][s] = fv;
				if (fv >= 0)
				{
					cout << "ERROR " << t << ' ' << s << ' ' << fv  << ' ' << log_act(t, s) << endl;
				}
			}
			copy(forwardVariables[t], fvars);
			if (t < totalTime -1)
			{
				loop(int s, segment_range(t))
				{
					fvars[s] /= log_scale_factor(t+1, s);
				}
			}
		}
		LogDouble logProb = forwardVariables.back().back();
		if (totalSegments > 1)
		{
			LogDouble += nth_last(forwardVariables.back(), 2);
		}
		check(logProb.log <= 0, "sequence\n" + str(seq) + "has log probability " + str(logProb.log));
		
		//RUN THE BACKWARD ALGORITHM
		//init arrays
		backwardVariables.reshape_with_depth(list_of(totalTime), totalSegments, LogDouble());
		
		//initialise (scaled) backward variables
		backwardVariables.back().back() = 0;
		if (totalSegments > 1)
		{
			nth_last(backwardVariables.back(), 2) = 0;
		}
		//loop over time, calculating backward variables recursively
		loop_back(int t, ::range(totalTime - 1))
		{
			Range<LogDouble> oldLogActs = this->logActivations[t+1];
			Range<LogDouble> oldBvars = backwardVariables[t+1];
			Range<LogDouble> bvars = backwardVariables[t];
			loop(int s, segment_range(t))
			{
				double bv;
				//s odd (label output)
				if (s&1)
				{
					bv = (oldBvars[s] * log_act(t+1, s)) + (oldBvars[s+1] * log_act(t+1, s+1));
					if ((s < (totalSegments-2)) && (segmentToLabel(s) != segmentToLabel(s+2)))
					{
						bv += oldBvars[s+2] * log_act(t+1, s+2);
					}
				}
				
				//s even (blank output)
				else
				{
					bv = oldBvars[s] * log_act(t+1, s);
					if (s < (totalSegments-1))
					{
						bv += oldBvars[s+1] * log_act(t+1, s+1);
					}
				}
				bvars[s] = bv / log_scale_factor(t+1, s);
			}
		}
		
		//INJECT THE ERRORS
		for (int t = 0; t < totalTime; ++t, actBegin += size)
		{
			if (t == 0)
			{
				const set<int>& indices = segmentToCompetingLabels(0);
				for (int j = 0; j < sliceSize; ++j)
				{
					for (int k = 0; k < size; ++k, ++errIt)
					{
						if (in(indices, k))
						{
							*errIt = safeExp(actBegin[k] - log_scale_factor(0, 0));
						}					
					}
				}
			}
			else
			{
				for (int j = 0; j < sliceSize; ++j)
				{
					for (int k = 0; k < size; ++k, ++errIt)
					{
						*errIt = safeExp(actBegin[k] + dEdATerms[k] - normTerm);
					}
				}			
			}
			errIt -= sliceSize * size;
			//calculate dE/dY terms and rescaling factor
			fill(dEdYTerms.begin(), dEdYTerms.end(), -doubleMax);
			fill(dEdATerms.begin(), dEdATerms.end(), -doubleMax);
			normTerm = -doubleMax;
			for (int s = 0; s < totalSegments; ++s)
			{
				//k = no decision node for even s, node in target string for odd s
				int k = (s&1) ? labelSeq[s/2] : blankUnit;
				double forVar = forwardVariables[t][s];
				double backVar = backwardVariables[t][s];
				double fbProduct =  forVar + backVar;
				dEdYTerms[k] = logAdd(dEdYTerms[k], fbProduct);
				normTerm = logAdd(normTerm, fbProduct);
				if (t < totalTime-1)
				{
					const set<int>& indices = segmentToCompetingLabels(s);
					double dEdA = fbProduct - log_scale_factor(t+1, s);
					for (SICI it = indices.begin(); it != indices.end(); ++it)
					{
						dEdATerms[*it] = logAdd(dEdATerms[*it], dEdA);
					}
				}
			}			
			//inject errors thru softmax layer for all acts in slice
			for (int j = 0; j < sliceSize; ++j)
			{
				for (int k = 0; k < size; ++k, ++errIt)
				{
					*errIt -= safeExp(dEdYTerms[k] - normTerm);
				}
			}
		}
	}
	if(decode && (outputFlag == TEST || outputFlag == DISPLAY))
	{
#ifndef _WIN32
		if (sequenceDebugOutput)
		{
			wordDecoder->printTargetWordSeq(seq);
		}
#endif	
		if (wordDecoder->fixedLength == 1)
		{
			vector<double> scores;
			for (int i = 0; i < wordDecoder->words.size(); ++i)
			{
				scores.push_back(calcWordForwardVariables(wordDecoder->words[i]));
			}
			int targetWordIndex = targetWordSeq.front();
			PRT(targetWordIndex);
			string targetWord = wordDecoder->words[targetWordIndex]->word;
			vector<pair<double, int> >wordScores = enumerate(scores);
			sort(wordScores.rbegin(), wordScores.rend());
			string outputWord = wordDecoder->words[wordScores.front().second]->word;
			errorMap["wordErrorRate"] += make_pair<int,double>(1, outputWord != targetWord);
#ifndef _WIN32
			if (sequenceDebugOutput)
			{
				for (int i = 0; i < wordDecoder->nBest; ++i)
				{
					if (wordDecoder->nBest > 1)
					{
						cout << i << " best" << endl;
					}
					int wordNum = wordScores[i].second;
					Word* word = wordDecoder->words[wordNum];
					cout << "output word: " << word->word << endl;
					cout << word->labelSeq.size() << " output labels: " << word->labelString << endl;
					cout << "word index: " << wordNum << endl;
					cout << "log probability " << wordScores[i].first << endl;
					if (word->word == targetWord)
					{
						cout << "RIGHT" << endl;
					}
					else
					{
						cout << "WRONG" << endl;
					}
				}
				
				// 				for (int i = 0; i < wordDecoder->nBest; ++i)
				// 				{
				// 					if (wordDecoder->nBest > 1)
				// 					{
				// 						cout << i << " best" << endl;
				// 					}
				// 					int wordNum = wordScores[i].second;
				// 					Word* word = wordDecoder->words[wordNum];
				// 					cout << "output word: " << word->word << endl;
				// 					cout << word->labelSeq.size() << " output labels: " << word->labelString << endl;
				// 					cout << "log probability " << wordScores[i].first << endl;
				// 					if (targetWordSeq.front() == wordNum)
				// 					{
				// 						cout << "RIGHT" << endl;
				// 					}
				// 					else
				// 					{
				// 						cout << "WRONG" << endl;
				// 					}
				// 				}
			}
#endif
		}
		else
		{
			wordDecoder->decode(errorMap, seq, actBuffer, expActBuffer);
		}
	}
	//store errors in map
	errorMap["ctcMlError"] += make_pair<int,double>(1, ctcMlError);
	
	//TODO substitution confusion matrix, insertion and deletion lists
	return ctcMlError;
};

#endif
