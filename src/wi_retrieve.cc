/*
 * wi_retrieve.cc
 *
 *  Created on: Apr 4, 2013
 *      Author: anguelos
 */

#include <png.h>
#include <tiff.h>
#include "__bilde__.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string/split.hpp>


std::vector<std::string> testSetFnames;
std::vector<std::string> trainSetFnames;
std::string outFname;
std::string binOperator;
std::string distance;
int printColumnCount;
int radius;
bool set255To0;
bool set0To0;
std::string rebaseComponents;
std::string scaleComponents;
std::string dumpTrainset;
std::string dumpTestset;

std::vector<std::vector<double> > getComponents(std::string name);

std::vector<double> getScaler(std::string name);

void dump(std::ostream& out, std::vector<std::vector<double> >& vals,
		std::string hSeparator,const std::vector<std::string>& fnames) {
	out << vals[0][0];
	for (int j = 1; j < vals[0].size(); j++) {
		out << hSeparator << vals[0][j];
	}
	out<<hSeparator<<fnames[0];
	for (int k = 1; k < vals.size(); k++) {
		out <<"\n"<< vals[k][0];
		for (int j = 1; j < vals[k].size(); j++) {
			out << hSeparator << vals[k][j];
		}
		out<<hSeparator<<fnames[k];
	}
}

std::string filename2classname(std::string fullPath) {
	std::vector<std::string> dirs;
	boost::split(dirs, fullPath, boost::is_any_of("/"));
	std::vector<std::string> filenameParts;
	boost::split(filenameParts, dirs[dirs.size() - 1], boost::is_any_of("."));
	return filenameParts[0];
}

std::vector<std::string> filenames2classnames(
		const std::vector<std::string>& v) {
	std::vector<std::string> res;
	for (int k = 0; k < v.size(); k++) {
		res.push_back(filename2classname(v[k]));
	}
	return res;
}

std::vector<double> loadWriterFeatures(std::string fname, int radius,
		std::string binOperator) {
	bilde::Buffer<bilde::t_uint8> inimg(
			bilde::documents::binarization::otsuBinarize(
					bilde::io::tiff::loadTiffGrayscale<bilde::t_uint8>(fname)));
	bilde::Buffer<bilde::t_uint8> lbpimg(inimg.width, inimg.height);

	bilde::operations::fixed_histogram::FixedHistogram<256> lbpHist(lbpimg);
	if (set255To0) {
		lbpHist.__data__[255] = 0;
	}
	if (set0To0) {
		lbpHist.__data__[0] = 0;
	}
	bilde::StaticVector<bilde::t_real64, 329> feats(
			bilde::documents::texture_features::getAllLbpFeatures(
					lbpHist.__data__));
	return  feats.asStdVector();
}

std::vector<std::vector<double> > loadWriterFeatures(
	std::vector<std::string> fnames, int radius, std::string compare) {
std::vector<std::vector<double> > res;
for (int k = 0; k < fnames.size(); k++) {
	res.push_back(loadWriterFeatures(fnames[k], radius, compare));
}
return res;
}

bool setParams(int argc, char** argv) {
//std::cerr<<"Before args\n";
bilde::util::Args("Writer identification classifier ",
		"anguelos.nicolaou@gmail.com");
bilde::util::Args().addIntParam(false, "k", "column-count",
		"The maximum number of columns to print out. If set to 0 no limit is set",
		&printColumnCount).setDefault(0);
bilde::util::Args().addBoolParam(false, "S", "set-255-to-0",
		"Wheather to set 255 in the LBP histogram to 0 occurences", &set255To0).setDefault(
		true);
bilde::util::Args().addBoolParam(false, "s", "set-0-to-0",
		"Wheather to set 0 in the LBP histogram to 0 occurences", &set0To0).setDefault(
		false);

bilde::util::Args().addStringSelectParam(false, "c", "principal-components",
		"Wheather to rebase to feature space to principal components.", {
				"null", "dfki", "icdar", "ichfr" }, &rebaseComponents).setDefault(
		"icdar");

bilde::util::Args().addStringSelectParam(false, "F", "scale-vector",
		"Wheather to scale the components", { "null", "dfki1class",
				"dfki3class",  "2011", "2012" },
		&scaleComponents).setDefault("2012");

bilde::util::Args().addIntParam(false, "r", "radius",
		"LBP operator radius (10 = 1 pixel). Accepted values 10, 15, 20, or 30",
		&radius).setDefault(30);
bilde::util::Args().addStringSelectParam(false, "f", "function",
		" The boolean function used for the LBP",
		{ "Equal255", "Equal", "NonEqual", "Greater", "GreaterEqual", "Smaller",
				"SmallerEqual" }, &binOperator).setDefault("Equal");
bilde::util::Args().addStringSelectParam(false, "d", "distance",
		" The distance metric.", { "L1", "L2", "LInf" }, &distance).setDefault(
		"L1");

bilde::util::Args().addStringListParam(false, "T", "train-set",
		"a list of TIFF files to be used for training", -1, &trainSetFnames);

bilde::util::Args().addStringListParam(false, "t", "test-set",
		"a list of TIFF files to be used for test", -1, &testSetFnames);

bilde::util::Args().addStringParam(false, "o", "output",
		"the path to an output file", &outFname);

bilde::util::Args().addStringParam(false, "u", "dump-trainset",
		"dump the tainset features", &dumpTrainset).setDefault("");

bilde::util::Args().addStringParam(false, "m", "dump-testset",
		"dump the testset features", &dumpTestset).setDefault("");


bilde::util::Args().parseArgv(argc, argv);

if (argc < 6) {
	std::cerr << bilde::util::Args().getHelpMessage()
			<< "\nInsufficient arguments!\nAborting.\n";
	exit(1);
}
if (radius != 10 && radius != 15 && radius != 20 && radius != 30) {
	std::cerr << "--radius should be 10, 15, 20, or 30\nAborting";
	exit(1);
}
if (trainSetFnames.size() < 2) {
	std::cerr
			<< "Error --train-set should contain at least 2 images\nAborting\n";
	exit(1);
}
if (testSetFnames.size() < 1) {
	std::cerr
			<< "Error --test-set should contain at least 1 image\nAborting\n";
	exit(1);
}
//std::cerr<<"Before validate\n";
if (bilde::util::Args().verboseLevel > 4) {
	std::cerr<<"DEBUG Validating\n";
	std::cerr << "Validating params:\n"
			<< bilde::util::Args().getValidationMsg() << "\n";
	std::cerr<<"DEBUG Validating FINISHED\n";
}
if (!bilde::util::Args().validate()) {
	//std::cerr<<"Before error message\n";
	//std::cerr<<"Argument validation failed message:\n"<<bilde::util::Args().getValidationMsg()<<"\nAborting\n";
	exit(1);
} else {
	if (bilde::util::Args().verboseLevel > 5) {
		std::cerr << "outFname : " << outFname << "\n";
		std::cerr << "trainSetFnames : " << trainSetFnames[0];
		for (int i = 1; i < trainSetFnames.size(); i++) {
			std::cerr << "\n\t" << trainSetFnames[i];
		};
		std::cerr << "\n";
		std::cerr << "testSetFnames : " << testSetFnames[0];
		for (int i = 1; i < testSetFnames.size(); i++) {
			std::cerr << "\n\t" << testSetFnames[i];
		};
		std::cerr << "\n";
		std::cerr << "radius : " << radius << "\n";
		std::cerr << "set0to0 : " << set0To0 << "\n";
		std::cerr << "set255to0 : " << set255To0 << "\n";
		std::cerr << "binOperator : " << binOperator << "\n";
		std::cerr << "distance : " << distance << "\n";
		std::cerr << "scaleComponents : " << scaleComponents << "\n";
		std::cerr << "rebaseComponents : " << rebaseComponents << "\n";
		std::cerr << "dumpTrainset : "<<dumpTrainset<<"\n";
		std::cerr << "dumpTestset : "<<dumpTestset<<"\n";
	}
}
return true;
}

std::string classify(const std::vector<std::vector<double> >&trainSet,
	const std::vector<std::vector<double> >&testSet,
	const std::vector<std::string> &trainFileNames,
	const std::vector<std::string> &testFileNames, std::string colSeparator =
			" ") {
std::vector<std::string> testSetClasses;
std::vector<std::string> trainSetClasses;
if (bilde::util::Args().verboseLevel > 6) {
	std::cerr << "Converting  trainset filenames to train-set classes ...";
}
trainSetClasses = filenames2classnames(trainFileNames);
if (bilde::util::Args().verboseLevel > 6) {
	std::cerr
			<< " conversion done.\nConverting  trainset filenames to test-set classes.";
}
testSetClasses = filenames2classnames(testFileNames);
if (bilde::util::Args().verboseLevel > 6) {
	std::cerr << " conversion done.\n";
}
if (bilde::util::Args().verboseLevel > 1) {
	std::cerr << "Classifying ... ";
}
std::vector<std::vector<std::pair<double, int> > > classificationResults;
if (distance == "L1") {
	classificationResults = bilde::util::knn::sortByDistance<
			bilde::util::knn::L1Distance>(trainSet, testSet);
}
if (distance == "L2") {
	classificationResults = bilde::util::knn::sortByDistance<
			bilde::util::knn::L2Distance>(trainSet, testSet);
}
if (distance == "LInf") {
	classificationResults = bilde::util::knn::sortByDistance<
			bilde::util::knn::LInfDistance>(trainSet, testSet);
}
if (bilde::util::Args().verboseLevel > 8) {
	std::cerr << "\nClasified " << classificationResults.size()
			<< " test samples\n";
}
if (bilde::util::Args().verboseLevel > 9) {
	for (int k = 0; k < classificationResults.size(); k++) {
		std::cerr << "\nTest pattern " << k << " results: "
				<< classificationResults[k].size() << " train set\n";
	}
}
if (bilde::util::Args().verboseLevel > 9) {
	std::cerr << "Test-classes\n";
	for (int k = 0; k < testSetClasses.size(); k++) {
		std::cerr << "\t" << k << ":\t" << testSetClasses[k] << "\n";
	}
	std::cerr << "Train-classes\n";
	for (int k = 0; k < trainSetClasses.size(); k++) {
		std::cerr << "\t" << k << ":\t" << trainSetClasses[k] << "\n";
	}
}

if (bilde::util::Args().verboseLevel > 1) {
	std::cerr << "Clasification done.\n";
}
std::stringstream tmp;
for (int testPatNum = 0; testPatNum < classificationResults.size();
		testPatNum++) {
	tmp << testSetClasses[testPatNum];
	for (int resultNum = 0;
			resultNum < classificationResults[testPatNum].size()
					&& (printColumnCount <= 0 || resultNum < printColumnCount);
			resultNum++) {
		tmp << colSeparator
				<< trainSetClasses[classificationResults[testPatNum][resultNum].second];
	}
	tmp << "\n";
}
if (bilde::util::Args().verboseLevel > 10) {
	std::cerr << "Results:\n" << "\n";
	std::cerr << tmp.str() << "\n";
}
return tmp.str();
}

int __main__(int argc, char** argv) {
setParams(argc, argv);
if (bilde::util::Args().verboseLevel > 1) {
	std::cerr << "Loading test-set: ... \n";
}
std::vector<std::vector<double> > inTestSet = loadWriterFeatures(testSetFnames,
		radius, binOperator);
if (bilde::util::Args().verboseLevel > 1) {
	std::cerr << "loaded " << inTestSet.size()
			<< " writer features in test-set\nLoading train-set: ... \n";
}
std::vector<std::vector<double> > inTrainSet = loadWriterFeatures(
		trainSetFnames, radius, binOperator);
if (bilde::util::Args().verboseLevel > 1) {
	std::cerr << "loaded " << inTrainSet.size()
			<< " writer features train-set\n";
}
std::vector<std::vector<double> > testSet;
std::vector<std::vector<double> > trainSet;
std::vector<std::vector<double> > components = getComponents(rebaseComponents);

testSet = bilde::util::matrix::multiply(inTestSet, components);
trainSet = bilde::util::matrix::multiply(inTrainSet, components);
std::vector<double> scaler = getScaler(scaleComponents);
testSet = bilde::util::matrix::scaleRows(testSet, scaler);
trainSet = bilde::util::matrix::scaleRows(trainSet, scaler);
//std::cerr<<components[0][0]<<","<<components[0][1]<<"\n";
if (dumpTestset != "") {
	std::ofstream out;
	out.open(dumpTestset.c_str());
	dump(out,testSet,",",testSetFnames);
}
if (dumpTrainset != "") {
	std::ofstream out;
	out.open(dumpTrainset.c_str());
	dump(out,trainSet,",",trainSetFnames);
}
if (outFname == "stdout") {
std::cout << classify(trainSet, testSet, trainSetFnames, testSetFnames) << "\n";
} else {
std::ofstream out;
out.open(outFname.c_str(), std::iostream::app);
out << classify(trainSet, testSet, trainSetFnames, testSetFnames) << "\n";
}
return 0;
}

int main(int argc, char**argv) {
try {
return __main__(argc, argv);
} catch (const char* msg) {
std::cerr << "Exception thrown msg:\n" << msg << "\n";
}
}
