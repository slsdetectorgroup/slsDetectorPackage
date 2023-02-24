#include "onlinedisp_zmq.h"
bool  hasallpede;
TH1F * his102;TH1F * his101;
int processedf;
sls::zmqHeader zHeader;
#define PEDEFNAME "current_pede.dat"
#define NPRO 50
#define NPRI 50

//#define JFSTRX
#ifdef JFSTRX
#include "jungfrauLGADStrixelsData.h"
#else
#include "jungfrauModuleData.h"
#endif



int main(int argc, char* argv[]) 
{
  goout=1;
  hasallpede=false;
  dophotonmap=true;  if ((argc<3)) {printf("USAGE: command photon_energy_(peakinADC) [rx_ip]  [port] \n"); return -1  ;}
  else { 
    phene=atoi(argv[1]);
    if (phene<0) dophotonmap=false;
    threshold=float (phene/2);
    printf( " \n");
    printf( "phene %d  \n",phene);
  }
  
  if (argc>=3) {
    strcpy(serverip,argv[2]);
    printf("ip is %s ",serverip);
  }

  portnum=30001;
  if (argc>=4 ){ portnum= atoi(argv[3]);
  }
  printf(", port number  is %d ",portnum);  printf(". \n"); 

  
#ifdef JFSTRX
    cout << "JFSTRX" << endl;
    jungfrauLGADStrixelsData *decoder = new jungfrauLGADStrixelsData();
    nx = 1024/5; ny= 512*5;
#else
    nx = 1024; ny= 512;
#endif




  gain_flag=false;
  pede_flag=false;
  bw_flag=false;
 
  HDraw_every=20;
  fixranges=false;

  
  hchptr = (short*) malloc(NCH*sizeof(short)); 

  startsocket(); //create and connect ZMQ 

  for (ipx=0;ipx<NCH;ipx++) hchptr[(ipx)]=0;
  
  
  //  cout<<   decoder->getValue((char*)(hchptr),279,130)<<endl;


  nonblock(NB_ENABLE);

  cout <<"opening the rootapp" <<endl;
 
  rootapp = new TApplication("Online JF display",&argc, argv);
 
  LoadPaletteFalse();

  char hname[100];



  his1000= new TH2F("his1000","2d , ev. pede corrected",nx,-0.5,nx-0.5,ny,-0.5,ny-0.5);
  his1000->SetOption("colz");
  his2000= new TH2F("his2000","2d gain ",nx,-0.5,nx-0.5,ny,-0.5,ny-0.5);
  his2000->GetZaxis()->SetRangeUser(0,4);

  if (dophotonmap) {
    his3000= new TH2F("his3000"," photon map  ",nx,-0.5,nx-0.5,ny,-0.5,ny-0.5);
  }
  else {
    his3000= new TH2F("his3000"," raw adc  ",nx,-0.5,nx-0.5,ny,-0.5,ny-0.5);
  }
 
  his4500= new TH2F("his45000","L vs R",101,-50,500,101,-50,500);
  hchip=new TH1I*[8];
  for (i=0;i<8;i++) {
    sprintf(hname,"hchip%d",i);
    hchip[i] = new TH1I(hname,hname,NBIN,MIN_POS,MAX_POS);
  }

  cout <<"end of histo booking" <<endl;
  if  (A2==NULL)  A2 = new TCanvas("A2","Plotting Canvas gain",150,10,500,250);
  if  (A3==NULL)  A3 = new TCanvas("A3","Plotting Canvas ADC",150,360,1200,550);
  if  (A4==NULL)  A4 = new TCanvas("A4","Plotting Canvas PHs",750,300,1000,800);
  A4->Clear();
  A4->Divide(4,2,0.005,0.005);
  if  (A5==NULL)  A5 = new TCanvas("A5","Plotting Canvas Photon Map",750,300,1000,600);
  if  (A6==NULL)  A6 = new TCanvas("A6","Plotting Canvas LvsR",650,250,650,660);

  gSystem->ProcessEvents();
  int running=0;
  char runc[15]="*\\-/|";
  printhelp();
 
 
 
  while (1==1) { // loop on streamed frames  

    if(!zmqSocket->ReceiveHeader(0,zHeader,  SLS_DETECTOR_JSON_HEADER_VERSION)){
      cout<< "Receiver stopped, waiting for new stream" << endl; 
      zmqSocket->Disconnect();
      zmqSocket->Connect();
    }
    else {
	
      //   if (((icount++)%10)==0) cout <<"recived frameindex "<<zHeader.frameIndex <<endl; 
      //cout <<"there" <<endl;
      zmqSocket->ReceiveData(0, (char *)(&image_data), NCH*2);
    }

    {

      framesinstream++;
      running++;
     
      fill1Ds=true; //alway fill 1d and LR plots 
      //if  (((framesinstream%(int(HDraw_every)))==(int (HDraw_every)-1))) {fill1Ds=true;}else{fill1Ds=false;} 
      if  (((framesinstream%(HDraw_every))==(HDraw_every)-1)) {show2Ds=true;}else{show2Ds=false;} 
      if  (((framesinstream%NPRI)==NPRI-1))  { cout<<"\r   "<<"frame (from start):   "<<framesinstream<<" " << runc[((running/NPRI)%5)]<<  "   discarded frames %=" << (1-float(processedf)/float(zHeader.frameIndex-frameIndex_old))*100  << " current framenumber= "  <<zHeader.frameIndex  << "       "<<std::flush; processedf=0;frameIndex_old=zHeader.frameIndex;}

 
      npacket=0;
      if  (show2Ds)  {
	his1000->Reset(); 
	his2000->Reset(); 
	if (!dophotonmap) his3000->Reset(); //FOR RAW ADC DISPLAY 
      }


      if ((fill1Ds)or(show2Ds)or(dophotonmap)) { // do something, otherwise skip to the next
	processedf++;

	for (i=0 ;i<NCH;i++) {
      
	  adcvalue= (image_data[i]) & 0x3fff;
     
	  if ((image_data[i] & 0xc000)!=0){ gain = (image_data[i]>>14) & 0x3;} else {gain=0;}
	  if (pede_flag){
	  
	    if (gain_flag)
	      {
		if ((gain==0)||(!hasallpede)) adcpedecorr=(adcvalue&0x3fff)*fgaind[i]-fpeded[i]*fgaind[i];
	  
		if ((gain==1)&&hasallpede)  adcpedecorr=(fpedeG1d[i]*fgaind[i]+G1Poffset-adcvalue*fgaind[i])*30.0; 
		if ((gain==3)&&hasallpede)  adcpedecorr=(fpedeG2d[i]*fgaind[i]+G2Poffset-adcvalue*fgaind[i])*340.0;
	      }
	    else

	      {
		  
		if ((gain==0)||(!hasallpede))  adcpedecorr=(adcvalue&0x3fff)-fpeded[i];
		if ((gain==1)&&hasallpede) adcpedecorr=(fpedeG1d[i]+G1Poffset-adcvalue)*30.0; 
		if ((gain==3)&&hasallpede) adcpedecorr=(fpedeG2d[i]+G2Poffset-adcvalue)*340.0;
	      }
	 

	  } else {adcpedecorr=float (adcvalue);}  //end of if pede 
       

      

	  if ((adcpedecorr>threshold)&&(pede_flag))  hchptr[(i)]= hchptr[(i)]+(int)((adcpedecorr+threshold)/phene); 


	  if (fill1Ds) {
	    if (((i%1024)<1004)&&((i%1024)>20)&&((i/1024)>20)) { //skip the pix near guardring for PH plots
	      ichip= i/(256*256*4)*4+((i/256)%4)  ;
	   
	      hchip[ichip]->Fill(adcpedecorr,1);
	    
	      if (((i%256)<253)&&((i%256)>2)) his4500->Fill(adcpedecorrold,adcpedecorr,1);
	      adcpedecorrold=adcpedecorr;
	    
	    
	    }
	  }//if (fill1Ds)


	

	  if ((show2Ds)) {
	    factor=2.0;
	    value=adcpedecorr;
	    if ((i%256==0)||(i%256==255)) value=int(value/factor);
	    if ((i/1024==255)||(i/1024==256)||(i/1024==767)||(i/1024==768)) value=int(value/factor);

	    his1000->Fill(float(i%1024),float(int (i/1024)),value);

	    if (!dophotonmap) his3000->Fill(float(i%1024),float(int (i/1024)) ,adcvalue);
	  
	    his2000->Fill(float(i%1024),float(int (i/1024)) ,gain);

	    value=(int)(hchptr[i]);
	 
	    if ((i%256==0)||(i%256==255)) value=int(value/factor);
	    if ((i/1024==255)||(i/1024==256)||(i/1024==767)||(i/1024==768)) value=int(value/factor);
	    if (dophotonmap) his3000->Fill(float(i%1024),float(int (i/1024)),float(value));

	  }
	}//   for (i=0 ;i<NCH-0;i++)
      }// /end of do something 
   


      if   ((show2Ds)) {

	for (ipx=0;ipx<NCH;ipx++) hchptr[(ipx)]=0;



	Plot2DHistos();  Plot1DHistos();  
      }
   
 
      ifp=kbhit();
      processifp(ifp);
     
      if  (((framesinstream%NPRO))==NPRO-1) gSystem->ProcessEvents();
	

    }

   

  

  }// end of infinite loop 

  rootapp->Run();
  nonblock(NB_DISABLE);
  return 0;
   

}
void processifp(int ifp){

  if (ifp!=0){
    c=fgetc(stdin);
    if (c=='s') {if (goout==0){goout=1;}else {myloop();}}
    if (c=='S') SetRanges();
    if (c=='+') { HDraw_every=HDraw_every*0.8;cout<< endl <<"Drawing every "<< HDraw_every<<" frames "<<endl; }
    if (c=='-') { HDraw_every=HDraw_every*1.25;cout<< endl <<"Drawing every "<< HDraw_every<<" frames "<<endl;}
    if (c=='G') {gain_flag=not gain_flag ;if (gain_flag) {cout<<"gain corr enab."<< endl;}else {cout<<"gain corr disab."<< endl;}}
    if (c=='[') { G1Poffset=G1Poffset-10;cout<< endl <<"G1Poffset "<<G1Poffset<<endl; }
    if (c==']') { G1Poffset=G1Poffset+10;cout<< endl <<"G1Poffset "<<G1Poffset<<endl; }
    if (c=='{') { G2Poffset=G2Poffset-10;cout<< endl <<"G2Poffset "<<G2Poffset<<endl; }
    if (c=='}') { G2Poffset=G2Poffset+10;cout<< endl <<"G2Poffset "<<G2Poffset<<endl; }
    if (c=='p') { //stopsocket();
      loadpede();//startsocket();  
    }
    if (c=='b') {LoadPaletteBW(1.1111);bw_flag=true; } 
    if (c=='B') {LoadPaletteBW(0.9);bw_flag=true; }
    if (c=='O')  savepede();
    if (c=='o')  readpede();
    if (c=='P')  loadallpede(); 
    if (c=='u')  his1000->SetOption("surf2z");
    if (c=='C')  his1000->SetOption("colz");

    if (c=='q') exit(0);
    if (c=='r') historeset();
    if (c=='R') axisreset();
  }

}
void loadallpede(){
  cout <<"not implemented "<< endl;
  // hasallpede=true;
 
  // system("./sls_detector_put  setbit 0x5d 12 "); //setting to FSG1 ;
  
  //  loadpede();
  //  loadpede(); 
  //  for (i=0;i<NCH;i++) {fpedeG1d[i]=fpeded[i];}

  // system("./sls_detector_put  setbit 0x5d 13 "); //setting to FSG0 ;
  // sleep(1);
  // loadpede(); 
  // loadpede(); 
  // for (i=0;i<NCH;i++) {fpedeG2d[i]=fpeded[i];}
  //   system("./sls_detector_put clearbit 0x5d 12  "); //setting to G0;
  //   system("./sls_detector_put clearbit 0x5d 13  "); //setting to G0;
  // sleep(2);

  // loadpede(); 
  // loadpede();





}
void loadpede(void){
  //startsocket(); 
  framesinstream=0;
  pede_flag=true;
  nframes=0;
 


  for (ipx=0;ipx<NCH;ipx++) fpeded[ipx]=0; 
  

  
  while (framesinstream<50) { // loop on files 


    if (!zmqSocket->ReceiveHeader(0,zHeader,  SLS_DETECTOR_JSON_HEADER_VERSION)){ 
      return;
    }

    cout <<"received frameindex "<<zHeader.frameIndex << endl; 
 
    zmqSocket->ReceiveData(0,  (char *)(&image_data), NCH*2);
    framesinstream++;nframes++;
    for (ipx=0;ipx<NCH;ipx++) fpeded[ipx]=(fpeded[ipx]*(nframes-1)+(float)(image_data[ipx]&0x3fff))/(float)(nframes);
  


  }

  for (ipx=0;ipx<NCH;ipx++) { ipeded[ipx]=(short)(fpeded[ipx]);
    if (ipx%60033==0) printf("i=%d pede= %d  %f .\n",ipx, ipeded[ipx],fpeded[ipx]);
  }
  printf("total frames for pede: %d \n",nframes);
  //stopsocket(); 

  printhelp();
}

int kbhit()
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
}
void myloop(void){ //wait doing nothing.    
  goout=0;
  int ifp;
 
 
  while (goout==0){
   
    ifp=kbhit();
    processifp(ifp);


    gSystem->ProcessEvents();
    usleep(5000);

  }
}
void printhelp(){
  cout<< endl << "s=start/pause| p/n=getnewpede/raw | o/O=read/save pede | n=nopede(raw) | r/R=rst His/Axis | +/- = faster/slower ref. |q=exit | U/C sUrf2/Colz  " <<endl;
}

void historeset(){
 
  his4500->Reset();
  his3000->Reset();
  for (i=0;i<8;i++) {
    hchip[i]->Reset();
  
  }
  Plot2DHistos();
  Plot1DHistos();

}

void SetRanges() {
  string str;
  std::cin.clear();
  //cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  cout<< endl;
  cout<<  " adc min " <<endl;
  //getline(cin, str);
  // adcmin= stoi(str);
  std::cin >> adcmin;
  cout<<  " adc max " <<endl;
  std::cin >> adcmax;
  cout<<  " p.map min " <<endl;
  std::cin >> pmmin;
  cout<<  " p.map max " <<endl;
  std::cin >> pmmax;

  fixranges=true;



}


void axisreset(){
  fixranges=false;
  his1000->GetXaxis()->UnZoom();

  his1000->GetYaxis()->UnZoom();
  his1000->GetZaxis()->UnZoom();
  his2000->GetXaxis()->UnZoom();
  his2000->GetYaxis()->UnZoom();

  his3000->GetZaxis()->UnZoom();
  
  for (i=0;i<8;i++) {
    hchip[i]->GetXaxis()->UnZoom();
    hchip[i]->GetYaxis()->UnZoom();
  }

  his4500->GetXaxis()->UnZoom(); 
  his4500->GetYaxis()->UnZoom(); 
  his4500->GetZaxis()->UnZoom(); 
  
  Plot2DHistos();
  Plot1DHistos();
  

}





void nonblock(int state)
{
  struct termios ttystate;
  //get the terminal state
  tcgetattr(STDIN_FILENO, &ttystate);
  if (state==NB_ENABLE)
    {
      //turn off canonical mode
      ttystate.c_lflag &= ~ICANON;
      //minimum of number input read.
      ttystate.c_cc[VMIN] = 1;
    }
  else if (state==NB_DISABLE)
    { //turn on canonical mode
      ttystate.c_lflag |= ICANON;
    }
  //set the terminal attributes.
  tcsetattr(STDIN_FILENO, TCSANOW, &ttystate); 
}


void LoadPaletteFalse(){
  const Int_t NRGBs = 5;
  const Int_t NCont = 90;

  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red,green, blue, NCont);
  gStyle->SetNumberContours(NCont);
  TColor::CreateGradientColorTable(NRGBs, stops, red,green  ,blue, NCont);
  gStyle->SetNumberContours(NCont);

}
void LoadPaletteBW(float gammatune){

  vgamma=vgamma*gammatune;
  cout<< "gamma is "<<vgamma<<endl;
  const Int_t NRGBs = 99;
  const Int_t NCont = 990;
  Double_t stops[NRGBs] ;
  Double_t red[NRGBs]  ;
  Double_t green[NRGBs];
  Double_t blue[NRGBs] ;

  for (int iRGB=0;iRGB<NRGBs;iRGB++){


    stops[iRGB] =(1/float(NRGBs)*float(iRGB));
    red[iRGB]   =  pow(stops[iRGB],vgamma);
    green[iRGB] = red[iRGB];
    blue[iRGB]  =red[iRGB];
    //  cout << iRGB<<"  "<< stops[iRGB] <<" " << red[iRGB]<<endl;

  }


  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);

  

  // TColor::SetPalette(52,0,1);

}

void Plot1DHistos(void){

  if (hchip[0]->GetXaxis()->GetLast()!=oldh0xlast){
    oldh0xlast=hchip[0]->GetXaxis()->GetLast();
    oldh0xfirst=hchip[0]->GetXaxis()->GetFirst();
    for (int ipad=1; ipad<8;ipad++) {
      hchip[ipad]->GetXaxis()->SetRange(oldh0xfirst,oldh0xlast);
     
    
    }
  }    
  

  for (int ipad=0; ipad<8;ipad++) {
    A4->cd(ipad+1);  
    gStyle->SetOptStat(1);    gPad->SetLogy();
     hchip[ipad%4+(1-int(ipad/4))*4]->Draw();
  
  }
  A4->cd();
  A4->Update();
}


void Plot2DHistos(void){
  gStyle->SetOptStat(0);
  A3->cd();

  //  if (bw_flag) LoadPaletteBW(1.0);

  if (fixranges) {

    his1000->GetZaxis()->SetRangeUser(float(adcmin),float(adcmax));
    his3000->GetZaxis()->SetRangeUser(float(pmmin),float(pmmax));
    

  }

  his1000->SetMinimum(-200);
  his1000->Draw();

  A3->Update();
  A2->cd();
  // if (bw_flag)  LoadPaletteFalse();
  his2000->GetXaxis()->SetRange(his1000->GetXaxis()->GetFirst(),his1000->GetXaxis()->GetLast());
  his2000->GetYaxis()->SetRange(his1000->GetYaxis()->GetFirst(),his1000->GetYaxis()->GetLast());

  his2000->Draw("colz"); 
  A2->Update(); 
  A5->cd();

  his3000->GetXaxis()->SetRange(his1000->GetXaxis()->GetFirst(),his1000->GetXaxis()->GetLast());
  his3000->GetYaxis()->SetRange(his1000->GetYaxis()->GetFirst(),his1000->GetYaxis()->GetLast());
  his3000->Draw("colz");
  A5->Update();
 
  A6->cd();
  his4500->Draw("colz");
  A6->Update();
  


}


void startsocket(void) {




  try {
    zmqSocket = new sls::ZmqSocket(serverip, portnum);

  } catch (...) {
    cprintf(RED,
	    "Error: Could not create Zmq socket on port %d with ip %s\n",
	    portnum, serverip);
    delete zmqSocket;
    return;
  }
  zmqSocket->SetReceiveHighWaterMark(3);
  zmqSocket->SetReceiveBuffer(1024*1024); 
  zmqSocket->Connect();


  cout<<"Zmq Client[] "<< zmqSocket->GetZmqServerAddress()<<endl;


  haveconnection=true;
 
 


}



 

void tryconnect(void) 

{
  int itry=0;
  cout<< endl;
 
  

  while (haveconnection==false) {
    sleep(1);
    cout<<"\r trying to (re)connect " <<itry++ << " " << endl ; 
    startsocket();
  }

}

void stopsocket(void) {
  // cout<<" cfd " << cfd << endl;; 
  
   

  delete zmqSocket;
  zmqSocket=0;
  //zmqSocket->~ZmqSocket ();

    

  haveconnection=false;

    
  

}
void savepede(void) {

  int pfd;

  pfd=open(PEDEFNAME,O_CREAT|O_WRONLY, 0666);
  if (pfd==-1) perror("open pede file");
   
  write(pfd,fpeded,2*NCH*sizeof(float));
  write(pfd,fpedeG1d,2*NCH*sizeof(float));
  write(pfd,fpedeG2d,2*NCH*sizeof(float));
  close(pfd);
 
}

void readpede(void) {

  int pfd;

  pfd=open(PEDEFNAME,O_RDONLY);
  if (pfd==-1) perror("open pede file");
   
  read(pfd,fpeded,NCH*2*sizeof(float));
  read(pfd,fpedeG1d,NCH*2*sizeof(float));
  read(pfd,fpedeG2d,NCH*2*sizeof(float));
   
  close(pfd);
  pede_flag=true;
  hasallpede=true;

}

