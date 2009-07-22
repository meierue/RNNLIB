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

#ifndef _INCLUDED_Word_h  
#define _INCLUDED_Word_h  

#include <string>
#include <vector>
#include <string>
#include <map>
#include "Helpers.h"
#include "LogDouble.h"

struct Word;

struct WordToken
{
	//data
	Word* word;
	const WordToken* prev;
	LogDouble prob;
	bool chosen;	
	
	//functions
	WordToken(Word* wor = 0, WordToken* pre = 0, LogDouble pro = LogDouble()):
		word(wor),
		prev(pre),
		prob(pro),
		chosen(false)
	{
	}
	static WordToken* best (WordToken* t1, WordToken*t2)
	{
		return t1->prob > t2->prob ? t1 : t2;
	}
	size_t length() const
	{
		size_t l = 0;
		for (const WordToken* t = this; t; t=t->prev, ++l);
		return l;
	}	
};

typedef vector<WordToken*>::iterator VPTI;

struct Word
{
	//data
	const TranscriptionLayer& output;
	vector<WordToken*> tokens;
	vector<WordToken*> oldTokens;
	WordToken* transitionToken;
	string str;
	string labelStr;
	vector<int> labelSeq;
	int totalSegments;
	vector<set<int> >competingLabels;

	//functions
	Word(const string& definition, const TranscriptionLayer& out):
		output(out),
		transitionToken(new WordToken(this))
	{
		stringstream ss (definition);
		ss >> str;
		string label;
		while (ss >> label)
		{
			labelSeq += output.labels.right.at(label);
			labelStr += " " + label;
		}
		trim(labelStr);
		totalSegments = (2*(int)labelSeq.size()) + 1;
		while(tokens.size() < totalSegments)
		{
			tokens += new WordToken(this);
			oldTokens += new WordToken(this);
		}
	}
	~Word()
	{
		delete_range(tokens);
		delete_range(oldTokens);
		delete transitionToken;
	}
	WordToken* init_tokens()
	{
		View<const LogDouble> logActs = output.logActivations[0];
		oldTokens[0]->prob = logActs[output.blank];
		if (oldTokens.size() > 1)
		{
			oldTokens[1]->prob = logActs[labelSeq[0]];
		}
		loop(int i, indices(tokens))
		{
			tokens[i]->prev = 0;
			tokens[i]->prob = LogDouble(0);
			oldTokens[i]->prev = 0;
			if (i > 1)
			{
				oldTokens[i]->prob = LogDouble(0);
			}
		}
		return emit_token();
	}
	WordToken* emit_token()
	{
		return ((totalSegments == 1) ? tokens.back() : WordToken::best(tokens.back(), nth_last(tokens, 2)));
	}
	WordToken* pass_tokens(const WordToken* inputToken, LogDouble transitionProb, int t)
	{
		//copy the external token to an updated transition token
		transitionToken->prob = inputToken->prob * transitionProb;
		transitionToken->prev = inputToken;
			
		//loop over the allowed segments, passing the tokens forwards
		View<const LogDouble> logActs = output.logActivations[t];
		loop(int s, output.segment_range(t, totalSegments))
		{
			WordToken* testToken;
			if (s == 0 || s == 1)
			{
				testToken = WordToken::best(transitionToken, oldTokens[s]);
			}
			else
			{
				testToken = oldTokens[s];
			}
			if (s)
			{
				testToken = WordToken::best(testToken, oldTokens[s-1]);
			}
			if (s&1)
			{
				if ((s > 1) && (labelSeq[s/2] != labelSeq[(s/2)-1]))
				{
					testToken = WordToken::best(testToken, oldTokens[s-2]);
				}
			}
			WordToken* token = tokens[s];
			token->prob = testToken->prob;
			token->prev = testToken->prev;
			if (s&1)
			{
				token->prob *= logActs[labelSeq[s/2]];
			}
			else
			{			
				token->prob *= logActs[output.blank];
			}
		}
		loop(int i, indices(tokens))
		{
			oldTokens[i]->prev = tokens[i]->prev;
			oldTokens[i]->prob = tokens[i]->prob;
		}
		return emit_token();
	}
};

ostream& operator << (ostream& out, const WordToken& tok)
{
	vector<const Word*> words;
	for (const WordToken* t = &tok; t && t->word; t = t->prev)
	{
		words += t->word;
	}
	loop_back(const Word* w, words)
	{
		out << w->str << " ";
	}
	return out;
}

ostream& operator << (ostream& out, const Word& w)
{
	out << w.str << " " << w.labelStr;
	return out;
}

#endif
