#pragma once
#include <iostream>  
#include <sstream>
#include <string>
using namespace std;

class StringNumUtils
{
public:
	StringNumUtils();
	~StringNumUtils();
public:
	template <class Type> string numToString(Type& num);
	
	template <class Type> Type stringToNum(const string& str);
	
};

template<class Type>
inline string StringNumUtils::numToString(Type & num)
{
	stringstream ss;
	ss << num;
	string s1 = ss.str();
	return s1;
}

template<class Type>
inline Type StringNumUtils::stringToNum(const string & str)
{
	istringstream iss(str);
	Type num;
	iss >> num;
	return num;
}
