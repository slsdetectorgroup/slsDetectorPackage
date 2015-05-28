#include <TFile.h>
#include <TThread.h>
#include <THStack.h>
#include "moenchReadData.C"

typedef struct task_s{
  char *fformat;
  char *tname;
  char *tdir;
  int runmin;
  int runmax;
  int treeIndex;
  double xTalk;
} Task;

void *moenchMakeTreeTask(void *p){
  TThread::Lock();
  char fname[1000];
  Task *t = (Task *)p;
  sprintf(fname,"%s%s_%i.root",t->tdir,t->tname,t->treeIndex);
  TFile *f = new TFile(fname,"RECREATE");
  cout << "Call moenchReadData(" << t->fformat << "," << t->tname << "," << t->runmin<< "," << t->runmax <<")" << endl;
  TThread::UnLock();
  THStack *s = moenchReadData(t->fformat,t->tname,t->runmin,t->runmax, 1500, -500, 1000, 1, t->xTalk);
  s->Write();
  if(f && f->IsOpen()){
    f->Close();
  }
  return 0;
}


void moenchReadDataMT(char *fformat, char *tit, char *tdir, int runmin, int runoffset, int nThreads, int treeIndexStart=0, double xTalk=0.044){
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
    t->xTalk = xTalk;
    cout << "start thread " << i << " start: " << t->runmin << " end " << t->runmax << endl;
    threads[i] = new TThread(threadName, moenchMakeTreeTask, t);
    threads[i]->Run();
  }

  for(int i = 0; i < nThreads; i++){
    threads[i]->Join();
  }

}




//to compile: g++ -DMYROOT -DMYROOT1 -g `root-config --cflags --glibs` -o moenchReadDataMT moenchReadDataMT.C
int main(int argc, char **argv){
  if(argc < 8){ 
    cout << "Usage: " << argv[0] << " fformat tit tdir runmin runoffset nThreads treeIndexStart [xTalkFactor]"  << endl; 
    exit(-1); 
  }

  char *fformat = argv[1];
  char *tit = argv[2];
  char *tdir = argv[3];
  int runmin = atoi(argv[4]);
  int runoffset = atoi(argv[5]);
  int nThreads = atoi(argv[6]);
  int treeIndexStart = atoi(argv[7]);
  double xTalkFactor = 0.044;

  if(argc == 9)
    xTalkFactor = atof(argv[8]);  

  moenchReadDataMT(fformat, tit, tdir,runmin,runoffset,nThreads,treeIndexStart, xTalkFactor);
  
}

