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
class func1_t{
public:
	func1_t(_Ret (_Class::*fn)(_Arg1),_Class* ptr,_Arg1 arg1, _Store* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_store(sto){}
	~func1_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1));}
private:
	_Class* m_ptr;
	_Ret (_Class::*m_fn)(_Arg1);
	_Arg1 m_arg1;
	_Store* m_store;
};

template<typename _Ret, typename _Class,typename _Arg1, typename _Arg2,typename _Store>
class func2_t{
public:
	func2_t(_Ret (_Class::*fn)(_Arg1,_Arg2),_Class* ptr,_Arg1 arg1,_Arg2 arg2,_Store* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_store(sto){}
	~func2_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2));}
private:
	_Class* m_ptr;
	_Ret (_Class::*m_fn)(_Arg1,_Arg2);
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Store* m_store;
};

template<typename _Ret, typename _Class,typename _Arg1, typename _Arg2, typename _Arg3, typename _Arg4,typename _Store>
class func4_t{
public:
	func4_t(_Ret (_Class::*fn)(_Arg1,_Arg2,_Arg3,_Arg4),_Class* ptr,_Arg1 arg1,_Arg2 arg2,_Arg3 arg3,_Arg4 arg4,_Store* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_arg3(arg3),m_arg4(arg4),m_store(sto){}
	~func4_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2,m_arg3,m_arg4));}
private:
	_Class* m_ptr;
	_Ret (_Class::*m_fn)(_Arg1,_Arg2,_Arg3,_Arg4);
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Arg3 m_arg3;
	_Arg4 m_arg4;
	_Store* m_store;
};

class Task:  public virtual slsDetectorDefs{
public:
	Task(func1_t <int,slsDetector,int,int>* t):
		m_int1(t),m_string1(0),m_chararr1(0),m_f2_1(0),m_settings(0){};
	Task(func1_t <string,slsDetector,string,string>* t):
		m_int1(0),m_string1(t),m_chararr1(0),m_f2_1(0),m_settings(0){};
	Task(func1_t <char*,slsDetector,char*,string>* t):
		m_int1(0),m_string1(0),m_chararr1(t),m_f2_1(0),m_settings(0){};

	Task(func2_t <int,slsDetector,string,int,int>* t):
		m_int1(0),m_string1(0),m_chararr1(0),m_f2_1(t),m_settings(0){};

	//specialized
	Task(func1_t <detectorSettings,slsDetector,int,int>* t):
		m_int1(0),m_string1(0),m_chararr1(0),m_f2_1(0),m_settings(t){};

	~Task(){}

	void operator()(){
		if(m_int1)			(*m_int1)();
		else if(m_string1)	(*m_string1)();
		else if(m_chararr1)	(*m_chararr1)();

		else if(m_f2_1)	(*m_f2_1)();

		//specialized
		else if(m_settings)	(*m_settings)();
	}

private:
	func1_t <int,slsDetector,int,int>* m_int1;
	func1_t <string,slsDetector,string,string>* m_string1;
	func1_t <char*,slsDetector,char*,string>* m_chararr1;
	func2_t <int,slsDetector,string,int,int>* m_f2_1;

	//specialized
	func1_t <detectorSettings,slsDetector,int,int>* m_settings;

};


