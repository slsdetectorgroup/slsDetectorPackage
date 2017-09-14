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

template<typename _Ret>
class func00_t{
public:
	func00_t(_Ret (slsDetector::*fn)(),slsDetector* ptr):
		m_fn(fn),m_ptr(ptr){}
	~func00_t() {}
	void operator()() const {((m_ptr->*m_fn)());}
private:
	_Ret (slsDetector::*m_fn)();
	slsDetector* m_ptr;
};

template<typename _Ret>
class func0_t{
public:
	func0_t(_Ret (slsDetector::*fn)(),slsDetector* ptr, _Ret* sto):
		m_fn(fn),m_ptr(ptr),m_store(sto){}
	~func0_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)());}
private:
	_Ret (slsDetector::*m_fn)();
	slsDetector* m_ptr;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1>
class func1_t{
public:
	func1_t(_Ret (slsDetector::*fn)(_Arg1),slsDetector* ptr,_Arg1 arg1, _Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_store(sto){}
	~func1_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1, typename _Arg2>
class func2_t{
public:
	func2_t(_Ret (slsDetector::*fn)(_Arg1,_Arg2),slsDetector* ptr,_Arg1 arg1,_Arg2 arg2,_Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_store(sto){}
	~func2_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1,_Arg2);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1, typename _Arg2, typename _Arg3>
class func3_t{
public:
	func3_t(_Ret (slsDetector::*fn)(_Arg1,_Arg2,_Arg3),slsDetector* ptr,_Arg1 arg1,_Arg2 arg2,_Arg3 arg3,_Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_arg3(arg3),m_store(sto){}
	~func3_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2,m_arg3));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1,_Arg2,_Arg3);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Arg3 m_arg3;
	_Ret* m_store;
};

template<typename _Ret,typename _Arg1, typename _Arg2, typename _Arg3, typename _Arg4>
class func4_t{
public:
	func4_t(_Ret (slsDetector::*fn)(_Arg1,_Arg2,_Arg3,_Arg4),slsDetector* ptr,_Arg1 arg1,_Arg2 arg2,_Arg3 arg3,_Arg4 arg4,_Ret* sto):
		m_fn(fn),m_ptr(ptr),m_arg1(arg1),m_arg2(arg2),m_arg3(arg3),m_arg4(arg4),m_store(sto){}
	~func4_t() {}
	void operator()() const {*m_store = ((m_ptr->*m_fn)(m_arg1,m_arg2,m_arg3,m_arg4));}
private:
	_Ret (slsDetector::*m_fn)(_Arg1,_Arg2,_Arg3,_Arg4);
	slsDetector* m_ptr;
	_Arg1 m_arg1;
	_Arg2 m_arg2;
	_Arg3 m_arg3;
	_Arg4 m_arg4;
	_Ret* m_store;
};


class SuperTask:  public virtual slsDetectorDefs {
public:
	SuperTask():m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
protected:
	/** Function defined
	 * First argument is Return type, the remaining are arguments
	 */


	func00_t <void				>* m1;
	func0_t <int				>* m2;
	func1_t <int,				int>* m3;
	func1_t <string,			string>* m4;
	func1_t <detectorSettings,	int>* m5;
	func2_t <int,				int,int>* m6;
	func2_t <int,				string,int>* m7;
	func2_t <dacs_t,			dacIndex,int>* m8;
	func2_t <detectorSettings,	detectorSettings,int>* m9;
	func2_t <string,			networkParameter,string>* m10;
	func3_t <int,				int,int,int>* m11;
	func4_t <int,				trimMode,int,int,int>* m12;
	func4_t <int,				int,int,detectorSettings,int>* m13;
	func4_t <dacs_t,			dacs_t,dacIndex,int,int>* m14;
};

class Task:  public virtual SuperTask{
public:
	/** Constructors
	 * First argument is Return type, the remaining are arguments
	 */
	Task(func00_t <void				>* t):									SuperTask(),m1(t){};
	Task(func0_t <int				>* t):									m1(0),m2(t),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func1_t <int,				int>* t):								m1(0),m2(0),m3(t),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func1_t <string,			string>* t):							m1(0),m2(0),m3(0),m4(t),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func1_t <detectorSettings,	int>* t):								m1(0),m2(0),m3(0),m4(0),m5(t),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func2_t <int,				int,int>* t):	 						m1(0),m2(0),m3(0),m4(0),m5(0),m6(t),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func2_t <int,				string,int>* t):						m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(t),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func2_t <dacs_t,			dacIndex,int>* t):                  	m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(t),m9(0),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func2_t <detectorSettings,	detectorSettings,int>* t):				m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(t),m10(0),m11(0),m12(0),m13(0),m14(0){};
	Task(func2_t <string,			networkParameter,string>* t):			m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(t),m11(0),m12(0),m13(0),m14(0){};
	Task(func3_t <int,				int,int,int>* t): 						m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(t),m12(0),m13(0),m14(0){};
	Task(func4_t <int,				trimMode,int,int,int>* t):				m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(t),m13(0),m14(0){};
	Task(func4_t <int,				int,int,detectorSettings,int>* t): 		m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(t),m14(0){};
	Task(func4_t <dacs_t,			dacs_t,dacIndex,int,int>* t):       	m1(0),m2(0),m3(0),m4(0),m5(0),m6(0),m7(0),m8(0),m9(0),m10(0),m11(0),m12(0),m13(0),m14(t){};


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
		else if(m12)	(*m12)();
		else if(m13)	(*m13)();
		else if(m14)    (*m14)();
	}
};


