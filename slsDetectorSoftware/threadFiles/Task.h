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

template<typename _Ret, typename _Class, typename _Store>
class func0_t{
public:
	func0_t(_Ret (_Class::*fn)(),_Class* ptr, _Store* sto):
		m_fn(fn),m_ptr(ptr),m_store(sto){}
	~func0_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)());}
private:
	_Class* m_ptr;
	_Ret (_Class::*m_fn)();
	_Store* m_store;
};

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

template<typename _Ret, typename _Class,typename _Arg1, typename _Arg2, typename _Arg3, typename _Store>
class func3_t{
public:
	func3_t(_Ret (_Class::*fn)(_Arg1,_Arg2,_Arg3),_Class* ptr,_Arg1 arg1,_Arg2 arg2,_Arg3 arg3,_Store* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_arg3(arg3),m_store(sto){}
	~func3_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2,m_arg3));}
private:
	_Class* m_ptr;
	_Ret (_Class::*m_fn)(_Arg1,_Arg2,_Arg3);
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Arg3 m_arg3;
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
	/* Return: int, Param: int */
	Task(func1_t <int,slsDetector,int,int>* t):									m1(t),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0){};
	/* Return: int, Param: string,int */
	Task(func2_t <int,slsDetector,string,int,int>* t):							m1(0),m2(t),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0){};
	/* Return: string, Param: string */
	Task(func1_t <string,slsDetector,string,string>* t):						m1(0),m2(0),m3(t),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0){};
	/* Return: char*, Param: char* */
	Task(func1_t <char*,slsDetector,char*,string>* t):							m1(0),m2(0),m3(0),m4(t),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0){};
	/* Return: detectorSettings, Param: int */
	Task(func1_t <detectorSettings,slsDetector,int,int>* t):					m1(0),m2(0),m3(0),m4(0),m5(t),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0){};
	/* Return: detectorSettings, Param: detectorSettings,int */
	Task(func2_t <detectorSettings,slsDetector,detectorSettings,int,int>* t):	m1(0),m2(0),m3(0),m4(0),m5(0),m6(t),m7(0),m8(0),m9(0),m10(0),m11(0){};
	/* Return: int, Param: int,int */
	Task(func2_t <int,slsDetector,int,int,int>* t):	 							m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(t),m8(0),m9(0),m10(0),m11(0){};
	/* Return: int, Param: int,int */
	Task(func3_t <int,slsDetector,int,int,int,int>* t): 						m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(t),m9(0),m10(0),m11(0){};
	/* Return: int, Param: trimMode,int,int,int */
	Task(func4_t <int,slsDetector,trimMode,int,int,int,int>* t):				m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(t),m10(0),m11(0){};
	/* Return: int, Param: int */
	Task(func0_t <int,slsDetector,int>* t):										m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(t),m11(0){};
	/* Return: char*, Param: networkParameter,string,string */
	Task(func2_t <char*,slsDetector,networkParameter,string,string>* t):		m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(t){};
	~Task(){}

	void operator()(){
		if(m1)			(*m1)();
		else if(m2)		(*m2)();
		else if(m3)		(*m3)();
		else if(m4)		(*m4)();
		else if(m5)		(*m5)();
		else if(m6)		(*m6)();
		else if(m7)		(*m7)();
		else if(m8)		(*m8)();
		else if(m9)		(*m9)();
		else if(m10)	(*m10)();
		else if(m11)	(*m11)();
	}

private:
	/* Return: int, Param: int */
	func1_t <int,slsDetector,int,int>* m1;
	/* Return: int, Param: string,int */
	func2_t <int,slsDetector,string,int,int>* m2;
	/* Return: string, Param: string */
	func1_t <string,slsDetector,string,string>* m3;
	/* Return: char*, Param: char* */
	func1_t <char*,slsDetector,char*,string>* m4;
	/* Return: detectorSettings, Param: int */
	func1_t <detectorSettings,slsDetector,int,int>* m5;
	/* Return: detectorSettings, Param: detectorSettings,int */
	func2_t <detectorSettings,slsDetector,detectorSettings,int,int>* m6;
	/* Return: int, Param: int,int */
	func2_t <int,slsDetector,int,int,int>* m7;
	/* Return: int, Param: int,int */
	func3_t <int,slsDetector,int,int,int,int>* m8;
	/* Return: int, Param: trimMode,int,int,int */
	func4_t <int,slsDetector,trimMode,int,int,int,int>* m9;
	/* Return: int, Param: int */
	func0_t <int,slsDetector,int>* m10;
	/* Return: char*, Param: networkParameter,string,string */
	func2_t <char*,slsDetector,networkParameter,string,string>* m11;
};


