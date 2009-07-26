#ifndef _INCLUDED_DecodingLayer_h  
#define _INCLUDED_DecodingLayer_h  

#include "SoftmaxLayer.hpp"
#include "StringAlignment.hpp"
#include "Word.hpp"
#include "ObjectStore.hpp"

struct token_prob_greater : public binary_function<const WordToken*, const WordToken*, const bool> 
{
	bool operator()(const WordToken* t1, const WordToken* t2) const
	{
		return t1->prob > t2->prob; 
	}
};
struct word_equal_to: public equal_to<Word*> 
{
	bool operator()(Word* w1, Word* w2)
	{
		return w1->labelSeq == w2->labelSeq;
	}
};
struct word_less: public less<Word*> 
{
	bool operator()(Word* w1, Word* w2) 
	{
		return w1->labelStr < w2->labelStr;
	}
};
ostream& operator << (ostream& out, const vector<Word*>& words)
{
	if (words.size())
	{
		out << words.front()->str;
		for (vector<Word*>::const_iterator w = words.begin() + 1; w != words.end(); ++w)
		{
			out << " " << (*w)->str;
		}
	}
	return out;
}

typedef map <pair<const Word*, const Word*>, LogDouble>::const_iterator MPWWDCI;
typedef map <const Word*, pair<LogDouble, LogDouble> >::const_iterator MPWDDCI;
typedef multiset<WordToken*, token_prob_greater>::iterator SPTTPGI;
typedef pair<const string, Word*> PSPW;

struct DecodingLayer: public TranscriptionLayer
{		
	//data
	map <string, Word*> dict;
	multimap<string, string> wordToLabelStrings;
	ObjectStore<WordToken> tokenStore;
	WordToken* nullToken;
	map <const Word*, pair<LogDouble, LogDouble> > oneGrams;
	map <pair<const Word*, const Word*>, LogDouble> twoGrams;
	multiset<WordToken*, token_prob_greater> oldTokens;
	multiset<WordToken*, token_prob_greater> newTokens;
	vector<WordToken*> bestTokens;
	int nBest;
	int fixedLength;
	LogDouble meanBigramProb;
	Word blankWord;
	
	//functions
	DecodingLayer(ostream& out, const string& name, const bimap<int, string>& labels, 
								  const string& dictFilename, const string& bigramFilename = "", int nb = 1, int fl = -1):
		TranscriptionLayer(out, name, labels),
		nullToken(new WordToken()),
		nBest(nb),
		fixedLength(fl),
		blankWord("", *this)
	{
		add_to_dict(&blankWord);
		ifstream dictFile(dictFilename.c_str());
		check(dictFile.is_open(), "cannot open dictionary file \"" + dictFilename + "\"");
		string line;
		this->out << "reading words from " << dictFilename << endl;
		while (getline(dictFile, line))
		{
			if (line != "")
			{
				add_to_dict(new Word(line, *this));
			}
		}
		this->out << dict.size() << " words read" << endl;
		if (verbose)
		{
			loop(const PSPW& p, dict)
			{
				this->out << *p.second << endl;
			}
		}
		if(bigramFilename != "")
		{
			ifstream bigramFile (bigramFilename.c_str());
			if (bigramFile.is_open())
			{
				this->out << "reading transition probabilities from " << bigramFilename << endl;
				string line;
				while (line.find("\\1-grams:") == string::npos && getline(bigramFile, line));
				while (line.find("\\2-grams:") == string::npos && getline(bigramFile, line))
				{
					if (line != "")
					{
						stringstream ss;
						ss << line;
						LogDouble inProb;
						LogDouble outProb;
						string word;
						ss >> inProb >> word >> outProb;
						if (in(wordToLabelStrings, word))
						{
							loop(const PSS& p, wordToLabelStrings.equal_range(word))
							{
								oneGrams[dict[p.second]] = make_pair(inProb, outProb);
							}
						}
						else
						{
							this->out << "WARNING word " << word << " found in oneGrams but not in dict" << endl;
						}
					}
				}
				this->out << oneGrams.size() << " one-grams read" << endl;
				int numMissingOneGrams = dict.size() - oneGrams.size();
				if (numMissingOneGrams)
				{
					this->out << "WARNING: " << numMissingOneGrams << " words with missing 1-grams" << endl;
					loop(PSPW& p, dict)
					{
						Word* w = p.second;
						if (oneGrams.find(w) == oneGrams.end())
						{
							this->out << w->str << endl;
						}
					}
				}
				while (line.find("\\end\\") == string::npos && getline(bigramFile, line))
				{
					if (line != "")
					{
						stringstream ss;
						ss << line;
						LogDouble prob;
						string word1;
						string word2;
						ss >> prob >> word1 >> word2;
						meanBigramProb = 0;
						int numBigramProbs = 0;
						if (in(wordToLabelStrings, word1) && in (wordToLabelStrings, word2))
						{
							loop(const PSS& p1, wordToLabelStrings.equal_range(word1))
							{
								loop(const PSS& p2, wordToLabelStrings.equal_range(word2))
								{
									twoGrams[make_pair(dict[p1.second], dict[p2.second])] = prob;
								}
							}
							meanBigramProb += prob;
							++numBigramProbs;
						}
						meanBigramProb /= (double)numBigramProbs;
					}
				}
				this->out << (int)twoGrams.size() << " bi-grams read" << endl;
				this->out << "meanBigramProb " << meanBigramProb << endl;
			}
			else
			{
				this->out << "unable to find bigram file " << bigramFilename << endl;
			}
		}		
	}
	~DecodingLayer(){}
	void add_to_dict(Word* w)
	{
		if (warn(!in(dict, w->labelStr), this->out, "duplicate definition in dictionary:\n" 
				 + w->str + " " + w->labelStr + "\n" 
				 + dict[w->labelStr]->str + " " + w->labelStr))
		{
//		check (!in(dict, w->labelStr), "duplicate definition in dictionary:\n" 
//			   + w->str + " " + w->labelStr + "\n" 
//			   + dict[w->labelStr]->str + " " + w->labelStr);
			dict[w->labelStr] = w;
			wordToLabelStrings.insert(make_pair(w->str, w->labelStr));
		}
	}
	LogDouble transition_prob(const Word* from, const Word* to)
	{
		MPWWDCI it = twoGrams.find(make_pair(from, to));
		if (it == twoGrams.end())
		{
			MPWDDCI fromIt = oneGrams.find(from);
			MPWDDCI toIt = oneGrams.find(to);
			if (fromIt != oneGrams.end() && toIt != oneGrams.end())
			{
				return fromIt->second.second * toIt->second.first;
			}
			else
			{
				return 2 * meanBigramProb;
			}
		}
		else
		{
			return it->second;
		}
	}
	WordToken* copy_token(WordToken* tok)
	{
		return tokenStore.copy(tok);
	}
	void delete_token(WordToken* tok)
	{
		tokenStore.delete_object(tok);
	}
	vector<Word*>& data_seq_to_words(const DataSequence& seq)
	{
		static vector<Word*> words;
		words.clear();
		string labelStr;
		loop(TII t, enumerate(seq.labelSeq))
		{
			check(in_left(labels, t.get<0>()), "label index " + str(t.get<0>()) + " not found in labels:" + str(labels));
			if (labelStr != "")
			{
				labelStr += " ";
			}
			labelStr += this->labels.left.at(t.get<0>());
			if (fixedLength != 1 || t.get<1>() == last_index(seq.labelSeq))
			{
				map<string, Word*>::const_iterator it = dict.find(labelStr);
				if (it != dict.end())
				{
					words += it->second;
					labelStr = "";
				}
			}
		}
		return words;
	}
	vector<Word*>& token_to_words(const WordToken* tok) const
	{
		static vector<Word*> words;
		words.clear();
		for(const WordToken* t = tok; t && t->word; t = t->prev)
		{
			if (t->word != &blankWord)
			{
				words += t->word;
			}
		}
		reverse(words);
		return words;
	}
	double calculate_errors(const DataSequence& seq)
	{		
		double ctcError = TranscriptionLayer::calculate_errors(seq);
		loop (WordToken* t, bestTokens)
		{
			delete_token(t);
		}
		bestTokens.clear();
		newTokens.clear();
		oldTokens.clear();
		
		//initialise the words and get the best initial token (null unless some word has length 1)
		loop(PSPW& p, dict)
		{
			newTokens.insert(copy_token(p.second->init_tokens()));
		}
		
		//pass the tokens
		loop(int t, range(1, totalTime))
		{
			//reduce newTokens list to nBest, unless using language model
			if (oneGrams.empty())
			{
				while(newTokens.size() > nBest)
				{
					SPTTPGI endTokIt = --newTokens.end();
					newTokens.erase(endTokIt);
					delete_token(*endTokIt);
				}
			}			
			newTokens.swap(oldTokens);
			loop (WordToken* tok, oldTokens)
			{
				tok->chosen = false;
			}
			newTokens.clear();
			loop(PSPW& p, dict)
			{
				Word*w = p.second;
				LogDouble bestProb = LogDouble(0);
				WordToken* inputToken = nullToken;
				LogDouble inputTransProb = oneGrams.empty() ? LogDouble(1) : LogDouble(0);
				loop(WordToken* tok, oldTokens)
				{
					LogDouble transProb = oneGrams.empty() ? LogDouble(1) : transition_prob(tok->word, w);
					LogDouble newProb = tok->prob * transProb;
					if (newProb > bestProb && (fixedLength < 0 || tok->length() < fixedLength))
					{
						bestProb = newProb;
						inputToken = tok;
						inputTransProb = transProb;
					}
					if (tok->prob <= bestProb)
					{
						break;
					}
				}
				inputToken->chosen = true;
				WordToken* outputToken = copy_token(w->pass_tokens(inputToken, inputTransProb, t));
				newTokens.insert(outputToken);
			} 
			loop(WordToken* tok, oldTokens)
			{
				tok->chosen ? bestTokens.push_back(tok) : delete_token(tok);		
			}
			oldTokens.clear();
		}
		if (fixedLength >= 0)
		{
			for (SPTTPGI it = newTokens.begin(); it != newTokens.end(); ++it)
			{
				if ((*it)->length() != fixedLength)
				{
					newTokens.erase(it);
					delete_token(*it);
				}
			}
		}
		vector<Word*>& targetWords = data_seq_to_words(seq);
		if (verbose)
		{
			this->out << "target word sequence (length " << targetWords.size() << "):" << endl << targetWords << endl;
			set<string> uniqueWords;
			loop(WordToken* tok, newTokens)
			{
				vector<Word*>& outputWords = token_to_words(tok);
				stringstream outputWordStream;
				outputWordStream << outputWords;
				if (!in(uniqueWords, outputWordStream.str()))
				{
					prt_line(out);
					uniqueWords.insert(outputWordStream.str());
					if (nBest > 1)
					{
						this->out << uniqueWords.size() << " best word transcription:" << endl;
					}
					this->out << "output word sequence (length " << outputWords.size() << "):" << endl << outputWordStream.str() << endl;
					this->out << "output word labels:" << endl;
					loop(Word* w, outputWords)
					{
						this->out << w->labelStr << " ";
					}
					this->out << endl;
					this->out << "log probability " << tok->prob.log << endl;
					StringAlignment<Word*> align(targetWords, outputWords);
					this->out << "word edit distance: " << align.distance << endl;
				}
				if (uniqueWords.size() >= nBest)
				{
					break;
				}
			}
		}
		StringAlignment<Word*> alignment(targetWords, token_to_words(*newTokens.begin()));
		double wordError = alignment.distance;
		ERR(wordError);
		this->normFactors["word"] = targetWords.size(); 
		this->errorMap["wordSeqError"] = wordError ? 1 : 0;
		if (fixedLength < 1)
		{		
			this->errorMap["wordSubstitutions"] = alignment.substitutions;
			this->errorMap["wordInsertions"] = alignment.insertions;
			this->errorMap["wordDeletions"] = alignment.deletions;
		}
		return ctcError;
	}								  
};

#endif
