#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <vector>
#include <errno.h>
#include <string.h>

#include "Global.h"
#include "sls_detector_defs.h"

#include <iostream>
using namespace std;

class slsDetector;


template<typename _Ret, typename _Class,typename _Arg1, typename _Store>
class func_t{
public:
	func_t(_Ret (_Class::*fn)(_Arg1),_Class* ptr,_Arg1 arg1, _Store* sto):m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_store(sto){}
	~func_t() {}

	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1));}

private:
	_Class* m_ptr;
	_Ret (_Class::*m_fn)(_Arg1);
	_Arg1 m_arg1;
	_Store* m_store;
};


class Task:  public virtual slsDetectorDefs{
public:
	Task(func_t <int,slsDetector,int,int>* t):m_int1(t),m_string1(0),m_chararr1(0),
		m_settings(0){};
	Task(func_t <string,slsDetector,string,string>* t):	m_int1(0),m_string1(t),m_chararr1(0),
		m_settings(0){};
	Task(func_t <char*,slsDetector,char*,string>* t):m_int1(0),m_string1(0),m_chararr1(t),
		m_settings(0){};


	//settings
	Task(func_t <slsDetectorDefs::detectorSettings,slsDetector,int,int>* t):
		m_int1(0),m_string1(0),m_chararr1(0),
		m_settings(t)
	{};

	~Task(){}

	void operator()(){
		if(m_int1)			(*m_int1)();
		else if(m_string1)	(*m_string1)();
		else if(m_chararr1)	(*m_chararr1)();
		else if(m_settings)	(*m_settings)();
	}

private:
	func_t <int,slsDetector,int,int>* m_int1;
	func_t <string,slsDetector,string,string>* m_string1;
	func_t <char*,slsDetector,char*,string>* m_chararr1;
	func_t <slsDetectorDefs::detectorSettings,slsDetector,int,int>* m_settings;
};


