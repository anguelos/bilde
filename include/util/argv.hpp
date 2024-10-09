/*
 * argv.hpp
 *
 *  Created on: Mar 17, 2013
 *      Author: anguelos
 */

#ifndef ARGV_HPP_
#define ARGV_HPP_

#include <boost/algorithm/string.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

namespace bilde {
namespace util {
namespace argv {



template<typename T> const char* __getTypeDescription__();
template<> const char* __getTypeDescription__<std::string>() {
	return "string";
}
template<> const char* __getTypeDescription__<bool>() {
	return "boolean";
}
template<> const char* __getTypeDescription__<t_sint32>() {
	return "integer";
}
template<> const char* __getTypeDescription__<t_real64>() {
	return "real";
}
template<> const char* __getTypeDescription__<std::vector<std::string> >() {
	return "string list";
}


inline std::vector<int> strVector2int(const std::vector<std::string> &v){
    std::vector<int> res;
    for(auto iter=v.begin();iter!=v.end();++iter){
        res.push_back(std::stoi(*iter));
    }
    return res;
}

inline std::vector<double> strVector2double(const std::vector<std::string> &v){
    std::vector<double> res;
    for(auto iter=v.begin();iter!=v.end();++iter){
        res.push_back(std::stod(*iter));
    }
    return res;
}


struct __BaseArgument__ {
	std::string shortname;
	std::string longname;
	std::string help;
	std::string typeDescription;
	void* data;
	__BaseArgument__(std::string sname, std::string lname, std::string h,
			std::string tdescription, void * d) :
			shortname(sname), longname(lname), help(h), typeDescription(
					tdescription), data(d) {
	}
	__BaseArgument__(const __BaseArgument__& b) :
	shortname(b.shortname), longname(b.longname), help(b.help), typeDescription(
			b.typeDescription), data(b.data) {
	}
	virtual std::string __getDefaultValString__() const=0;
	std::string getDescription() const {
		std::stringstream tmp;
		tmp << shortname << " | " << longname << " t: "
		<< typeDescription << " " << help << ". "
		<< this->__getDefaultValString__();
		return tmp.str();
	}
	template<typename T> T getAs() const {
		if (strcmp(__getTypeDescription__<T>(), typeDescription) != 0) {
			throw "__BaseArgument__::getAs<T> improper template type";
		}
		return *((T*) (data));
	}
	virtual std::string __getValidationMsg__()const=0;
	virtual bool __validate__() const =0;
	virtual void __setVal__(std::vector<std::string> valVector)=0;
};

template<typename T> struct TypedArgument: public __BaseArgument__ {
	static std::map<std::string,bool> __getBooleanMap__(){
		std::map<std::string,bool> res;
		res["true"]=true;
		res["True"]=true;
		res["TRUE"]=true;
		res["T"]=true;
		res["t"]=true;
		res["1"]=true;
		res["false"]=false;
		res["False"]=false;
		res["FALSE"]=false;
		res["F"]=false;
		res["f"]=false;
		res["0"]=false;
		return res;
	}
	static std::map<std::string,bool>& __getBooleanMapOnce__(){
		static std::map<std::string,bool> res=__getBooleanMap__();
		return res;
	}
	bool hasDefault;
	bool valSet;
	T* optionVal;
	T storedData;
	TypedArgument(const TypedArgument<T>& ta) :
			__BaseArgument__(ta), hasDefault(ta.hasDefault), valSet(ta.valSet), optionVal(
					ta.isInternalyStored() ? &storedData : ta.optionVal), storedData(
					ta.storedData) {
	}
	TypedArgument(std::string sn, std::string ln, std::string h, T* val) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<T>(), val), hasDefault(
					false), valSet(false), optionVal(val) {
	}
	TypedArgument(std::string sn, std::string ln, std::string h) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<T>(), &storedData),
			hasDefault(false), valSet(false), optionVal(&storedData) {
	}
	bool isInternalyStored() const {
		return (optionVal) == &(storedData);
	}
	void setDefault(T d) {
		*optionVal = d;
		hasDefault = true;
	}
	bool __validate__() const {
		//std::cerr<<"__validate__ : hasDefault:"<<hasDefault<<" , "<<" valSet:"<<valSet;
		return hasDefault || valSet;
	}
	std::string __getValidationMsg__() const {
		if (__validate__()) {
			return "";
		} else {
			std::stringstream tmp;
			tmp << "Option " << longname
					<< " was not set and doesn't have a default value.";
			return tmp.str();
		}
	}
	void __setVal__(std::vector<std::string> valVector) {
		//std::cerr<<"Set val starting \n";for(int k=0;k<valVector.size();k++){std::cerr<<valVector[k]<<",";}std::cerr<<" done\n";
		this->valSet=true;
		if (__getTypeDescription__<T>() == __getTypeDescription__<bool>()  ) {
			if(valVector.size()==1){
				valVector.push_back("true");
			}
			//std::cerr<<"SET VAL bool :"<<__getBooleanMapOnce__().at(valVector[1])<<"\n";
			*optionVal=__getBooleanMapOnce__().at(valVector[1]);
		} else {
			if (valVector.size() != 2) {
				std::cerr<<"Option "<<valVector[0]<<" : expectected only one value, got "<<valVector.size()-1<<" values\nAborting.\n";
				exit(1);
			}
			std::stringstream ss(valVector[1]);
			ss >> *optionVal;
		}
	}
	std::string __getDefaultValString__() const {
		if (!hasDefault) {
			return "";
		} else {
			std::stringstream tmp;
			tmp << " Default value: " << *optionVal;
			return tmp.str();
		}
	}
};

template<> struct TypedArgument<std::string> : public __BaseArgument__ {
	std::vector<std::string> allowed;
	std::string storedData;
	bool hasDefault;
	bool valSet;
	std::string* optionVal;
	TypedArgument<std::string>(const TypedArgument<std::string>& ta) :
			__BaseArgument__(ta), storedData(ta.storedData), hasDefault(ta.hasDefault), valSet(ta.valSet), optionVal(ta.isInternalyStored() ? &storedData : ta.optionVal) {
	}

	TypedArgument<std::string>(std::string sn, std::string ln, std::string h,
			std::string* val, std::vector<std::string> allowedOptions) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<std::string>(),val), 
			allowed(allowedOptions), hasDefault(false), valSet(false), optionVal(val) {
	}
	TypedArgument<std::string>(std::string sn, std::string ln, std::string h, std::vector<std::string> allowedOptions) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<std::string>(), &storedData), 
			allowed(allowedOptions), hasDefault(false), valSet(false), optionVal(&storedData) {
	}

	TypedArgument<std::string>(std::string sn, std::string ln, std::string h, std::string* val) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<std::string>(),val),
			hasDefault(false), valSet(false), optionVal(val) {
	}

	TypedArgument<std::string>(std::string sn, std::string ln, std::string h) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<std::string>(),&storedData), 
			hasDefault(false), valSet(false), optionVal(&storedData) {
	}
	bool __validate__() const {
		bool res = hasDefault || valSet;
		bool found = allowed.size() == 0;
		for (unsigned int k = 0; k < allowed.size() ; k++) {
			found = found || (*optionVal == allowed[k]);
		}
		return res && found;
	}
	std::string __getValidationMsg__() const {
		//std::cerr<<"DEBUG __getValidationMsg__<std::string> start\n";
		if (__validate__()) {
			//std::cerr<<"DEBUG __getValidationMsg__<std::string> end\n";
			return "";
		} else {
			std::stringstream tmp;
			if (hasDefault || valSet ||allowed.size()==0) {
				tmp << "String option " << longname
						<< " was not set and doesn't have a default value.";
			} else {
				//std::cerr<<"DEBUG __getValidationMsg__<std::string> long name: "<<longname<<" {"<<*optionVal<<"} alowed size "<<allowed.size()<<"\n";
				tmp << "Option " << longname << " has value '" << *optionVal
						<< "'. Allowed values are [ " << allowed[0];
				for (unsigned int k = 1; k < allowed.size(); k++) {
					tmp << " | " << allowed[1];
				}
				tmp << " ].";
			}
			//std::cerr<<"DEBUG __getValidationMsg__<std::string> end\n";
			return tmp.str();
		}
	}
	bool isInternalyStored() const {
		return (optionVal) == &(storedData);
	}
	void setDefault(std::string d) {
		*optionVal = d;
		hasDefault = true;
	}
	void __setVal__(std::vector<std::string> valVector) {
		//std::cerr<<"\n__setVal__<std::string> value set #"<<valVector[0]<<"#"<<valVector[1]<<"#"<<this->getDescription()<<"\n";
		this->valSet=true;
		if (valVector.size() != 2) {
			throw "Expected specifically one option";
		}
		*optionVal = valVector[1];
	}
	std::string __getDefaultValString__() const {
		std::stringstream tmp;
		if(allowed.size()>0){
			tmp<<" values: ["<<allowed[0];
			for(unsigned int k=1;k<allowed.size();k++){
				tmp<<"|"<<allowed[k];
			}
			tmp<<"]";
		}
		if (!hasDefault) {
			return "";
		} else {
			tmp << " Default value: " << *optionVal;
		}
		return tmp.str();
	}
};

template<> struct TypedArgument<std::vector<std::string> > : public __BaseArgument__ {
	std::vector<std::string> storedData;
	bool hasDefault;
	bool valSet;
	std::vector<std::string>* optionVal;
	unsigned int maxAllowedOptions;
	TypedArgument<std::vector<std::string> >(const TypedArgument<std::vector<std::string> >& ta) :
			__BaseArgument__(ta), storedData(ta.storedData), 
			hasDefault(ta.hasDefault), valSet(ta.valSet),
			optionVal(ta.isInternalyStored() ? &storedData : ta.optionVal),maxAllowedOptions(ta.maxAllowedOptions) {
	}
	TypedArgument<std::vector<std::string> >(std::string sn, std::string ln,
		std::string h, std::vector<std::string>* val, int maxAllowed) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<std::vector<std::string> >(),val),
			hasDefault(false), valSet(false), optionVal(val), maxAllowedOptions(maxAllowed) {
	}
	TypedArgument<std::vector<std::string> >(std::string sn, std::string ln,std::string h, int maxAllowed) :
			__BaseArgument__(sn, ln, h, __getTypeDescription__<std::vector<std::string> >(),&storedData), 
			hasDefault(false), valSet(false), optionVal(&storedData), maxAllowedOptions(maxAllowed) {
	}
	bool __validate__() const {
		return (maxAllowedOptions < 0 || maxAllowedOptions >= optionVal->size());
	}
	std::string __getValidationMsg__() const {
		//std::cerr<<"DEBUG __getValidationMsg__<std::vector<std::string> > begin\n";
		if (__validate__()) {
			//std::cerr<<"DEBUG __getValidationMsg__<std::vector<std::string> > end\n";
			return "";
		} else {
			std::stringstream tmp;
			tmp << "Option " << longname << " was passed "<<optionVal->size()<<" arguments. Maximum allowed is "<<maxAllowedOptions <<"\n";
			//std::cerr<<"DEBUG __getValidationMsg__<std::vector<std::string> > end\n";
			return tmp.str();
		}
	}
	bool isInternalyStored() const {
		return (optionVal) == &(storedData);
	}
	void setDefault(std::vector<std::string> d) {
		*optionVal = d;
		hasDefault = true;
	}
	void __setVal__(std::vector<std::string> valVector) {
		optionVal->clear();
		this->valSet=true;
		if (maxAllowedOptions > 0
				&& optionVal->size() > maxAllowedOptions + 1) {
			throw("maxAllowedOptions Exceeded");
		}
		for (unsigned int j = 1; j < valVector.size(); j++) {
			optionVal->push_back(valVector[j]);
		}
	}
	std::string __getDefaultValString__() const {
		if (!hasDefault) {
			return "";
		} else {
			std::stringstream tmp;
			tmp << " Default value: [ "
					<< ((optionVal->size() > 0) ? (*optionVal)[0] : "");
			for (unsigned int k = 1; k < optionVal->size(); k++) {
				tmp << " , " << (*optionVal)[k];
			}
			tmp << " ] .";
			return tmp.str();
		}
	}
};

inline void  __printArgs__(std::vector<std::vector<std::string> >args,std::ostream & out=std::cerr){
	out<<"Arguments:\n";
	for(unsigned int k=0;k<args.size();k++){
		out<<"{ ";
		for(unsigned int j=0;j<args[k].size();j++){
			out<<" ["<<args[k][j]<<" ] ";
		}
		out<<"}\n";
	}
	out<<"\n\n";
}

struct ArgumentParser {
    static std::vector<std::string> __parseArgumentsFromFile__(std::string fname){
		std::ifstream fin;
		fin.open(fname.c_str());
		std::string token;
        std::vector<std::string> res;
		while(fin>>token){
			res.push_back(token);
		}
		return res;
	}
    static std::vector<std::string> __argcv2strVector__(int argc,char** argv){
        std::vector<std::string> res(argc);
        for(int k=0;k<argc;k++){
            res[k]=argv[k];
        }
        return res;
    }

    static std::vector<std::vector<std::string> > __argv2Vector__(std::vector<std::string> argv) {
		std::vector<std::vector<std::string> > res;
        int argc=argv.size();
		if (argc > 1) {
			res.push_back(std::vector<std::string>());
		}
		int curVector = 0;
		int k = 1;
		while (k < argc) {
			if (argv[k][0] == '-') {
				if (argv[k][1] == '\0') {
					if(res[res.size()-1].size()!=0){
						res.push_back(std::vector<std::string>());
						curVector++;
					}
				} else {
					if(res[res.size()-1].size()!=0){
						res.push_back( { argv[k] });
						curVector++;
					}else{
						res[res.size()-1].push_back(argv[k]);
					}


				}
			} else {
				res[curVector].push_back(argv[k]);
			}
			k++;
		}
		if(res.size()>0 && res[res.size()-1].size()==0){
			res.pop_back();
		}
		return res;
	}

	static void __removeEqualities__(
			std::vector<std::vector<std::string> >& args) {
		for (unsigned int k = 0; k < args.size(); k++) {
			if (args[k][0].find('=') != std::string::npos) {

				int equalPos = args[k][0].find_first_of('=');
				std::string option = args[k][0].substr(0, equalPos);
				std::string optionValue = args[k][0].substr(equalPos + 1,
						std::string::npos);
				//std::cerr<<"Found not equal {"<<args[k][0]<<"} =>{"<<option<<","<<optionValue<<"}\n";
				args[k].erase(args[k].begin());
				//std::cerr<<"New head:"<<args[k][0]<<"\n";
				args[k].insert(args[k].begin(), optionValue);
				//std::cerr<<"New head:"<<args[k][0]<<"\n";
				args[k].insert(args[k].begin(), option);
				//std::cerr<<"New head:"<<args[k][0]<<"\n";
			}
		}
	}

	static void __replaceLongNames__(
			std::vector<std::vector<std::string> >& args,
			std::map<std::string, std::string> allNames2ShortNames) {
		for (unsigned int k = 0; k < args.size(); k++) {
			if (args[k][0][0] == '-') {
				try{
					args[k][0] = allNames2ShortNames.at(args[k][0]);
				}catch(const std::exception& ex){
					std::cerr<<"Unknown option "<<args[k][0]<<".\nAborting.\n";
					exit(1);
				}
			}
		}
	}

	static std::vector<std::string> __removeExistingPositionals__(
			std::vector<std::vector<std::string> > args,
			std::vector<std::string> positionals) {
		std::vector<std::string> filteredPositionals;
		for (unsigned int j = 0; j < positionals.size(); j++) {
			bool found = false;
			for (unsigned int k = 0; k < args.size() && !found; k++) {
				if (args[k][0] == positionals[j]) {
					found = true;
				}
			}
			if (!found) {
				filteredPositionals.push_back(positionals[j]);
			}
		}
		return filteredPositionals;
	}

	static void __addPositionals__(std::vector<std::vector<std::string> >& args,
			std::vector<std::string> positionals) {
		unsigned int curPositional = 0;
		for (unsigned int k = 0; k < args.size(); k++) {
			if (args[k][0][0] != '-') {
				if (curPositional >= positionals.size()) {
					throw "positionals exceeded declared positionals";
				}
				args[k].insert(args[k].begin(), positionals[curPositional]);
				curPositional++;
			}
		}
	}
	std::string __dumpArgs__(){
		std::stringstream tmp;
		for(unsigned int k=0;k<args.size();k++){
			for(unsigned int j=0;j<args[k].size();j++){
				tmp<<"{"<<args[k][j]<<"} ";
			}
			tmp<<"\n";
		}
		return tmp.str();
	}
	bool validate(){
		for (unsigned int k = 0; k < __allShortNames__.size(); k++) {
			if (!allArgs.at(__allShortNames__[k]).__validate__()) {
				//std::cerr<<"Validation fails at "<<__allShortNames__[k]<<"\n"<<allArgs.at(__allShortNames__[k]).__getValidationMsg__() <<"\n";
				return false;
			}else{
				//std::cerr<<"Validation passes at "<<__allShortNames__[k]<<"\n\n";
			}
		}
		return true;
	}
	std::string getValidationMsg(){
		for (unsigned int k = 0; k < __allShortNames__.size(); k++) {
			if (!allArgs.at(__allShortNames__[k]).__validate__()) {
				return allArgs.at(__allShortNames__[k]).__getValidationMsg__();
			}
		}
		return "";
	}
	void parseArgv(int argc, char** argv) {
		__programName__ = argv[0];
        args = __argv2Vector__(__argcv2strVector__(argc,
                argv));
		//__printArgs__(args);
		__removeEqualities__(args);
		//__printArgs__(args);
		__replaceLongNames__(args, allNames2ShortNames);


        //__printArgs__(args);
		std::vector<std::string> cleanedPositionals =
				__removeExistingPositionals__(args, positionalShortNames);
		__addPositionals__(args, cleanedPositionals);
		//__printArgs__(args);
		for (unsigned int k = 0; k < args.size(); k++) {
			allArgs.at(args[k][0]).__setVal__(args[k]);
		}
        if(*(stringArgs.at("-I").optionVal)!=""){
            std::vector<std::string> parsedArgs={"NULL"};
            std::vector<std::string> fileArgs=__parseArgumentsFromFile__(*(stringArgs.at("-I").optionVal));
            parsedArgs.insert(parsedArgs.end(),fileArgs.begin(),fileArgs.end());
            std::vector<std::vector<std::string> > newArgs=__argv2Vector__(parsedArgs);
            __removeEqualities__(newArgs);
            __replaceLongNames__(newArgs, allNames2ShortNames);
            args.insert(args.end(), newArgs.begin(), newArgs.end());
            for (unsigned int k = 0; k < newArgs.size(); k++) {
                allArgs.at(newArgs[k][0]).__setVal__(newArgs[k]);
            }
        }

		if(*(boolArgs.at("-h").optionVal)){
			std::cout<<this->getHelpMessage()<<"\n";
			exit(0);
		}

        if(this->verboseLevel>5){
            std::cerr<<"Argv Arguments:\n";
            for(unsigned int k =0;k<args.size();k++){
                for(unsigned int j=0;j<args[k].size();j++){
                    std::cerr<<" {"<<args[k][j]<<"} ";
                }
                std::cerr<<"\n";
            }
        }

	}
	std::vector<std::vector<std::string> > args;
	std::string __contactInfo__;
	std::string __programName__;
	std::string help;
	std::vector<std::string> __allShortNames__;
	std::vector<std::string> positionalShortNames;
	std::map<std::string, std::string> allNames2ShortNames;
	std::map<std::string, TypedArgument<bool> > boolArgs;
	std::map<std::string, TypedArgument<t_sint32> > intArgs;
	std::map<std::string, TypedArgument<t_real64> > realArgs;
	std::map<std::string, TypedArgument<std::string> > stringArgs;
	std::map<std::string, TypedArgument<std::vector<std::string> > > stringListArgs;
	std::map<std::string, __BaseArgument__ &> allArgs;
	int verboseLevel;
	ArgumentParser(std::string description = "", std::string ci = "") {
		this->help = description+"This software was complied on "+std::string(__DATE__)+".";
		this->__contactInfo__ = ci;
		this->__programName__ = "NA";
		this->addBoolParam(false,"h","help"," Print this help message. ").setDefault(false);
		this->addIntParam(false,"v","verbose-level"," Verbocity level integer. 0: least verbose , 10: most verbose. ",&verboseLevel).setDefault(0);
        this->addStringParam(false,"I","input-argv"," Aditional File from where to load command paramenters. Usefull for huge argument lists.").setDefault("");
	}
	std::string getHelpMessage() {
		std::stringstream tmp;
		tmp << __programName__ << ":\n\nGeneral description:\n" << help
				<< "\n\nParameters:\n\n";
		for (unsigned int k = 0; k < __allShortNames__.size(); k++) {
			tmp << (allArgs.at(__allShortNames__[k])).getDescription() << "\n\n";
		}
		if (positionalShortNames.size() > 0) {
			tmp << "Positional arguments:\n" << positionalShortNames[0];
			for (unsigned int k = 1; k < positionalShortNames.size(); k++) {
				tmp << " , " << positionalShortNames[k];
			}
			tmp << "\n\n";
		}
		if (__contactInfo__.length() > 0) {
			tmp << "Contact info : " << __contactInfo__ << "\n\n";
		}
		return tmp.str();
	}
	int getIntParam(std::string name) {
		std::string sn = allNames2ShortNames[name];
		return *(intArgs.at(sn).optionVal);
	}
	bool getBoolParam(std::string name) {
		std::string sn = allNames2ShortNames[name];
		//std::cerr<<"NAME"<<name<<" count"<<boolArgs.count(sn)<<" val "<<*(boolArgs.at(sn).optionVal)<<"\n";
		return *(boolArgs.at(sn).optionVal);
	}
	t_real64 getRealParam(std::string name) {
		std::string sn = allNames2ShortNames[name];
		return *(realArgs.at(sn).optionVal);
	}
	std::string getStringParam(std::string name) {
		std::string sn = allNames2ShortNames[name];
		return *(stringArgs.at(sn).optionVal);
	}
	std::vector<std::string> getStringListParam(std::string name) {
		std::string sn = allNames2ShortNames[name];
		return *(stringListArgs.at(sn).optionVal);
	}
	TypedArgument<t_sint32>& addIntParam(bool isPositional, std::string sn,
			std::string ln, std::string h, int *val = NULL) {
		sn = "-" + sn;
		ln = "--" + ln;
		allNames2ShortNames[sn] = sn;
		allNames2ShortNames[ln] = sn;
		if (allArgs.count(sn) > 0) {
			throw "addParam param already defined";
		}
		__allShortNames__.push_back(sn);
		if (isPositional) {
			positionalShortNames.push_back(sn);
		}
		if (val == NULL) {
			intArgs.insert(
					std::map<std::string, TypedArgument<t_sint32> >::value_type(
							sn, TypedArgument<t_sint32>(sn, ln, h)));
		} else {
			intArgs.insert(
					std::map<std::string, TypedArgument<t_sint32> >::value_type(
							sn, TypedArgument<t_sint32>(sn, ln, h, val)));
		}
		allArgs.insert(std::map<std::string, __BaseArgument__& >::value_type(sn,intArgs.at(sn)));
		return intArgs.at(sn);
	}
	TypedArgument<bool>& addBoolParam(bool isPositional, std::string sn,
			std::string ln, std::string h, bool *val = NULL) {
		sn = "-" + sn;
		ln = "--" + ln;
		allNames2ShortNames[sn] = sn;
		allNames2ShortNames[ln] = sn;
		if (allArgs.count(sn) > 0) {
			throw "addParam param already defined";
		}
		__allShortNames__.push_back(sn);
		if (isPositional) {
			positionalShortNames.push_back(sn);
		}
		if (val == NULL) {
			boolArgs.insert(
					std::map<std::string, TypedArgument<bool> >::value_type(sn,
							TypedArgument<bool>(sn, ln, h)));

		} else {
			boolArgs.insert(
					std::map<std::string, TypedArgument<bool> >::value_type(sn,
							TypedArgument<bool>(sn, ln, h, val)));
		}
		allArgs.insert(std::map<std::string, __BaseArgument__& >::value_type(sn,boolArgs.at(sn)));
		//std::cerr<<"SET NAME"<<sn<<" count"<<boolArgs.count(sn)<<"\n";
		return boolArgs.at(sn);
	}
	TypedArgument<t_real64>& addRealParam(bool isPositional, std::string sn,
			std::string ln, std::string h, t_real64 *val = NULL) {
		sn = "-" + sn;
		ln = "--" + ln;
		allNames2ShortNames[sn] = sn;
		allNames2ShortNames[ln] = sn;
		if (allArgs.count(sn) > 0) {
			throw "addParam param already defined";
		}
		__allShortNames__.push_back(sn);
		if (isPositional) {
			positionalShortNames.push_back(sn);
		}
		if (val == NULL) {
			realArgs.insert(
					std::map<std::string, TypedArgument<t_real64> >::value_type(
							sn, TypedArgument<t_real64>(sn, ln, h)));
		} else {
			realArgs.insert(
					std::map<std::string, TypedArgument<t_real64> >::value_type(
							sn, TypedArgument<t_real64>(sn, ln, h, val)));
		}
		allArgs.insert(std::map<std::string, __BaseArgument__& >::value_type(sn,realArgs.at(sn)));
		return realArgs.at(sn);
	}
	TypedArgument<std::string>& addStringParam(bool isPositional,
			std::string sn, std::string ln, std::string h, std::string *val =
					NULL) {
		sn = "-" + sn;
		ln = "--" + ln;
		allNames2ShortNames[sn] = sn;
		allNames2ShortNames[ln] = sn;
		if (allArgs.count(sn) > 0) {
			throw "addParam param already defined";
		}
		__allShortNames__.push_back(sn);
		if (isPositional) {
			positionalShortNames.push_back(sn);
		}
		if (val == NULL) {
			stringArgs.insert(
					std::map<std::string, TypedArgument<std::string> >::value_type(
							sn, TypedArgument<std::string>(sn, ln, h)));
		} else {
			stringArgs.insert(
					std::map<std::string, TypedArgument<std::string> >::value_type(
							sn, TypedArgument<std::string>(sn, ln, h, val)));
		}
		allArgs.insert(std::map<std::string, __BaseArgument__& >::value_type(sn,stringArgs.at(sn)));
		return stringArgs.at(sn);
	}
	TypedArgument<std::string>& addStringSelectParam(bool isPositional,
			std::string sn, std::string ln, std::string h,
			std::vector<std::string> choices, std::string *val = NULL) {
		sn = "-" + sn;
		ln = "--" + ln;
		allNames2ShortNames[sn] = sn;
		allNames2ShortNames[ln] = sn;
		if (allArgs.count(sn) > 0) {
			throw "addParam param already defined";
		}
		__allShortNames__.push_back(sn);
		if (isPositional) {
			positionalShortNames.push_back(sn);
		}
		if (val == NULL) {
			stringArgs.insert(
					std::map<std::string, TypedArgument<std::string> >::value_type(
							sn,
							TypedArgument<std::string>(sn, ln, h, choices)));
		} else {
			stringArgs.insert(
					std::map<std::string, TypedArgument<std::string> >::value_type(
							sn,
							TypedArgument<std::string>(sn, ln, h, val,
									choices)));
		}
		allArgs.insert(std::map<std::string, __BaseArgument__& >::value_type(sn,stringArgs.at(sn)));
		stringArgs.at(sn).allowed=choices;
		return stringArgs.at(sn);
	}
	TypedArgument<std::vector<std::string> >& addStringListParam(
			bool isPositional, std::string sn, std::string ln, std::string h,
			int maxCount = -1, std::vector<std::string> *val = NULL) {
		sn = "-" + sn;
		ln = "--" + ln;
		allNames2ShortNames[sn] = sn;
		allNames2ShortNames[ln] = sn;
		if (allArgs.count(sn) > 0) {
			throw "addParam param already defined";
		}
		__allShortNames__.push_back(sn);
		if (isPositional) {
			positionalShortNames.push_back(sn);
		}
		if (val == NULL) {
			stringListArgs.insert(
					std::map<std::string,
							TypedArgument<std::vector<std::string> > >::value_type(
							sn,
							TypedArgument<std::vector<std::string> >(sn, ln, h,
									maxCount)));

		} else {
			stringListArgs.insert(
					std::map<std::string,
							TypedArgument<std::vector<std::string> > >::value_type(
							sn,
							TypedArgument<std::vector<std::string> >(sn, ln, h,
									val, maxCount)));
		}
		allArgs.insert(std::map<std::string, __BaseArgument__& >::value_type(sn,stringListArgs.at(sn)));
		return stringListArgs.at(sn);
	}

}
;

}
argv::ArgumentParser& Args(std::string description = "",
	std::string contactInfo = "") {
static argv::ArgumentParser res(description, contactInfo);
return res;
}

#define VERBOSE (bilde::util::Args().verboseLevel)

}
}

#endif /* ARGV_HPP_ */
