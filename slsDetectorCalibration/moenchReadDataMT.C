#include <TFile.h>
#include <TThread.h>
#include "moenchReadData.C"

typedef struct task_s{
  char *fformat;
  char *tname;
  char *tdir;
  int runmin;
  int runmax;
  int treeIndex;
} Task;

void *moenchMakeTreeTask(void *p){
  TThread::Lock();
  char fname[1000];
  Task *t = (Task *)p;
  sprintf(fname,"%s%s_%i.root",t->tdir,t->tname,t->treeIndex);
  TFile *f = new TFile(fname,"RECREATE");
  cout << "Call moenchReadData(" << t->fformat << "," << t->tname << "," << t->runmin<< "," << t->runmax <<")" << endl;
  TThread::UnLock();
  moenchReadData(t->fformat,t->tname,t->runmin,t->runmax);
  f->Close();
  return 0;
}


void moenchReadDataMT(char *fformat, char *tit, char *tdir, int runmin, int runoffset, int nThreads, int treeIndexStart=0){
  char threadName[1000];
  TThread *threads[nThreads];
  for(int i = 0; i < nThreads; i++){
    sprintf(threadName,"t%i",i);
    Task *t = (Task *)malloc(sizeof(Task));
    t->fformat = fformat;
    t->tname = tit;
    t->tdir = tdir;
    t->runmin = runmin + i*runoffset;
    t->runmax = runmin + (i+1)*runoffset - 1;
    t->treeIndex = treeIndexStart + i;
    cout << "start thread " << i << " start: " << t->runmin << " end " << t->runmax << endl;
    threads[i] = new TThread(threadName, moenchMakeTreeTask, t);
    threads[i]->Run();
  }

  for(int i = 0; i < nThreads; i++){
    threads[i]->Join();
  }

}



