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

#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>
#include "MultilayerNet.h"
#include "NetcdfDataset.h"
#include "GradientCheck.h"
#include "WeightContainer.h"
#include "SteepestDescent.h"
#include "Rprop.h"
#include "Trainer.h"
#include "MemoryNet.h"
#include "CodeNet.h"
#include "PmNet.h"

typedef boost::iostreams::tee_device<ostream, ofstream> TeeDev;
typedef boost::iostreams::stream<TeeDev> TeeStream;
		
extern bool verbose;
vector<string> validDatasets = list_of<string>("train")("test")("val");

int main(int argc, char* argv[])
{
	if (argc < 2 || argc > 3)
	{
		cout << "usage rnnl [-s | --save] config_file" << endl;
		exit(0);
	}
	bool autosave = false;
	string configFilename;
	if (argc == 3)
	{
		string saveFlag(argv[1]);
		autosave = (saveFlag == "-s" || saveFlag == "--save");
		configFilename = argv[2];
	}
	else
	{
		configFilename = argv[1];
	}
	ConfigFile conf(configFilename);
#ifdef FAST_LOGISTIC
	Logistic::fill_lookup();
#endif
	string task = conf.get<string>("task");
	if (task == "transcription" && conf.has("dictionary"))
	{
		task = conf.set<string>("task", "dictionary_transcription");
	}
	bool display = conf.get<bool>("display", false);
	vector<int> jacobianCoords = conf.get_list<int>("jacobianCoords");
	bool gradCheck = conf.get<bool>("gradCheck", false);
	verbose = conf.get<bool>("verbose", false);
	int displaySequence = conf.get<int>("sequence", 0);
	string dataset = conf.get<string>("dataset", "train");
	check(in(validDatasets, dataset), 
		  dataset + " given as 'dataset' parameter in config file '" 
		  + configFilename + "'\nmust be one of '" + str(validDatasets) + "'");
	string dataFileString = dataset + "File";
	string saveName = "";
	ofstream* logout = 0;
	TeeDev* tdev = 0;
	TeeStream* tout = 0;
	string displayPath = "";
	string logname = "";
	if (display || jacobianCoords.size())
	{
		displayPath = conf.get<string>("displayPath");
		logname = displayPath + "log";
	} 
	else if (autosave)
	{
		if (in (conf.filename, '@'))
		{
			saveName = conf.filename.substr(0, conf.filename.find('@'));
		}
		else
		{
			saveName = conf.filename.substr(0, conf.filename.rfind('.'));
		}
		saveName += "@" + time_stamp();
		logname = saveName + ".log";
	}
	if (autosave || display || jacobianCoords.size())
	{
		logout = new ofstream(logname.c_str());
		check(logout->is_open(), "can't open log file " + logname);
		tdev = new TeeDev(cout, *logout);
		tout = new TeeStream(*tdev);
		cout << "writing to log file " << logname << endl;
	}
	ostream& out = tout ? *tout : cout;
	vector<string> dataFiles = conf.get_list<string>(dataFileString);
	int dataFileNum = conf.get<int>("dataFileNum", 0);
	check(dataFiles.size() > dataFileNum, "no " + ordinal(dataFileNum) + " file in size " + str(dataFiles.size()) + " file list " + dataFileString + " in " + configFilename);
	DataHeader header(dataFiles[dataFileNum], task, 1);
	DataSequence* testSeq = 0;
	if (display || gradCheck || jacobianCoords.size())
	{
		NetcdfDataset* data = new NetcdfDataset(dataFiles[dataFileNum], task, displaySequence);
		testSeq = new DataSequence((*data)[0]);
		delete data;
	}
	Mdrnn *net;
	if (task == "code")
	{
		net = new CodeNet(out, conf, header);
	}
	else if (task == "memory")
	{
		net = new MemoryNet(out, conf, header);
	}
	else if (task == "pm")
	{
		net = new PmNet(out, conf, header);
	}
	else
	{
		net = new MultilayerNet(out, conf, header);
	}
	out << endl << "network:" << endl;
	PRINT(task, out);
	out << *net;
	
	//build weight container after net is created
	WeightContainer::instance().build();
	int numWeights = WeightContainer::instance().weights.size();
	out << numWeights << " weights" << endl << endl;
	
	//build the network after the weight container
	net->build();
	
	//only construct optimiser after weight container is built
	Optimiser* opt;
	if (conf.get<string>("optimiser", "steepest") == "rprop")
	{
		opt = new Rprop(out);
	}
	else
	{
		opt = new SteepestDescent(out, conf.get<double>("learnRate", 1e-4), conf.get<double>("momentum", 0.9));
	}
	Trainer trainer(out, net, opt, conf);
	out << "setting random seed to " << Random::set_seed(conf.get<unsigned long int>("randSeed", 0)) << endl << endl;
	if (conf.get<bool>("loadWeights", false))
	{
		out << "loading dynamic data from "  << conf.filename << endl;
		DataExportHandler::instance().load(conf, out);
		out << "epoch = " << trainer.epoch << endl << endl;
	}
	double initWeightRange = conf.get<double>("initWeightRange", 0.1);
	out << "randomising uninitialised weights with mean 0 std. dev. " << initWeightRange << endl << endl;
	WeightContainer::instance().randomise(initWeightRange);	
	out << "optimiser:" << endl << *opt << endl;
	if (gradCheck)
 	{
		out << "data header:" << endl << header << endl;
		out << "running gradient check for sequence " << displaySequence << endl;
		out << *testSeq; 
		prt_line(out);
 		GradientCheck(out, net, *testSeq, conf.get<int>("sigFigs", 6), 
			conf.get<double>("pert", 1e-5), conf.get<bool>("verbose", false));
 	}
	else if (jacobianCoords.size())
	{
		out << "data header:" << endl << header << endl;
		out << "calculating Jacobian for sequence " << displaySequence << " at coords " << jacobianCoords << endl;
		out << *testSeq; 
		out << "output path: " << endl << displayPath << endl;
		net->feed_forward(*testSeq);
		net->print_output_shape(out);
		net->outputLayer->outputErrors.get(jacobianCoords) = net->outputLayer->outputActivations.get(jacobianCoords);
		net->feed_back();
		DataExportHandler::instance().display(displayPath);
	}
	else if (display)
	{
		out << "data header:" << endl << header << endl;
		out << "displaying sequence " << displaySequence << endl;
		out << *testSeq; 
		out << "output path: " << endl << displayPath << endl;
		net->train(*testSeq);
		net->print_output_shape(out);
		out << "errors:" << endl << net->outputLayer->errorMap;
		DataExportHandler::instance().display(displayPath);
	}
	else if (conf.get<bool>("errorTest", false))
	{
		trainer.calculate_all_errors();
	}
	else
	{
		out << "trainer:" << endl;
		trainer.train(saveName);
	}
	if (logout)
	{
		delete logout;
	}
	if (tdev)
	{
		delete tdev;
	}
//	if (tout)
//	{
//		delete tout;
//	}
	delete net;
	delete opt;
}
