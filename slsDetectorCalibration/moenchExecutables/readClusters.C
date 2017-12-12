#include "single_photon_hit.h"
//#include "etaVEL/etaInterpolationPosXY.h"

TH2F *readClusters(char *fname, int nx, int ny, TH2F *h2=NULL) {
  FILE *f=fopen(fname,"r");
  int iph=0; 
  int ns=4;
  double px, py;
  double left, right, top, bottom;
  if (f) {
    int x1,y1;
    if (h2==NULL)
      h2=new TH2F("h2",fname,nx, -0.5, nx-0.5, ny, -0.5, ny-0.5);
      h2mult=new TH2F("h2mult",fname,nx, -0.5, nx-0.5, ny, -0.5, ny-0.5);
    TH2F *hint=new TH2F("hint",fname,nx*ns, -0.5, nx-0.5, ny*ns, -0.5, ny-0.5); 
      TH2F *hff=new TH2F("hff","hff",ns, -0.5, 0.5, ns, -0.5, +0.5); 
    TH1F *hsp=new TH1F("hsp",fname,500,0,2000);
    TCanvas *c=new TCanvas();
    c->SetLogz(kTRUE);
    h2->Draw("colz");
    TCanvas *c1=new TCanvas();
    hsp->Draw();
    TCanvas *c2=new TCanvas();
    hint->Draw("colz");
    c2->SetLogz(kTRUE);
    single_photon_hit cl(3,3);
    double tot;
    int w;
    double phw=340, phs=62;
    while (cl.read(f)) {
      //cl.get_pixel(x1, y1);
      //cout << cl.iframe << " " << cl.x << " " << cl.y << endl;
      //if (cl.x>80 && cl.x<280 && cl.y>80 && cl.y<300) {
	tot=0;
	left=0;
	right=0;
	top=0;
	bottom=0;
	for (int ix=-1; ix<2; ix++)
	  for (int iy=-1; iy<2; iy++){
	    tot+=cl.get_data(ix,iy);
	    if (ix<0) left+=cl.get_data(ix,iy);
	    if (ix>0) right+=cl.get_data(ix,iy);
	    if (iy<0) bottom+=cl.get_data(ix,iy);
	    if (iy>0) top+=cl.get_data(ix,iy);
	    
	  }
	px=(-left+right)/tot;
	py=(-bottom+top)/tot;
	//max at 340
	if (tot>200) {
	  w=(tot+3.5*phs)/phw;
	} else
	  w=0;
	if (w) {
	  hsp->Fill(tot);
	  if (w==1) {
	    h2->Fill(cl.x,cl.y,w);
	    hint->Fill(px+cl.x,py+cl.y,w);
	    if (cl.y<350)
	      hff->Fill(px,py,w);
	  }
	 
	  h2mult->Fill(cl.x,cl.y,w);
	    
	//}
	  iph+=w;
	  if (iph%100000==0) {
	    // c->Modified();
	    // c->Update();
	    c1->Modified();;
	    c1->Update();
	    // c2->Modified();;
	    // c2->Update();
	  }
	}
	//	if (iph>0.5E7) break;
    }
    fclose(f);
    hff->Scale(hff->GetNbinsX()*hff->GetNbinsY()/hff->Integral());
    TH2F *hint2=(TH2F*)hint->Clone("hint2");
    double ff;
    for (int ibx=0; ibx<hint->GetNbinsX(); ibx++) {
      for (int iby=0; iby<hint->GetNbinsY(); iby++) {
    	ff=hff->GetBinContent(ibx%ns+1, iby%ns+1);
    	//	cout << ibx << " " << iby << " " << ibx%ns << " " << iby%ns << " " << ff << endl;
    	if (ff>0)
    	  hint2->SetBinContent(ibx+1, iby+1,hint->GetBinContent(ibx+1,iby+1)/ff); 
      }
    }


    



    return h2;
  
  } else
    cout << "could not open file " << fname << endl;
  return NULL;


}

// TH2F *getEta(char *fname, int nx, int ny, TH2F *h2=NULL) {
//   int ns=10;
//   slsInterpolation *inte=new etaInterpolationPosXY(nx,ny,ns,

//   FILE *f=fopen(fname,"r");
//   int iph=0; 
//   int ns=4;
//   double px, py;
//   double left, right, top, bottom;
//   if (f) {
//     int x1,y1;
//     if (h2==NULL)
//       h2=new TH2F("h2",fname,nx, -0.5, nx-0.5, ny, -0.5, ny-0.5);
//       h2mult=new TH2F("h2mult",fname,nx, -0.5, nx-0.5, ny, -0.5, ny-0.5);
//     TH2F *hint=new TH2F("hint",fname,nx*ns, -0.5, nx-0.5, ny*ns, -0.5, ny-0.5); 
//     //  TH2F *hff=new TH2F("hff","hff",ns, -0.5, 0.5, ns, -0.5, +0.5); 
//     TH1F *hsp=new TH1F("hsp",fname,500,0,2000);
//     TCanvas *c=new TCanvas();
//     c->SetLogz(kTRUE);
//     h2->Draw("colz");
//     TCanvas *c1=new TCanvas();
//     hsp->Draw();
//     TCanvas *c2=new TCanvas();
//     hint->Draw("colz");
//     c2->SetLogz(kTRUE);
//     single_photon_hit cl(3,3);
//     double tot;
//     int w;
//     double phw=340, phs=62;
//     while (cl.read(f)) {
//       //cl.get_pixel(x1, y1);
//       //cout << cl.iframe << " " << cl.x << " " << cl.y << endl;
//       //if (cl.x>80 && cl.x<280 && cl.y>80 && cl.y<300) {
// 	tot=0;
// 	left=0;
// 	right=0;
// 	top=0;
// 	bottom=0;
// 	for (int ix=-1; ix<2; ix++)
// 	  for (int iy=-1; iy<2; iy++){
// 	    tot+=cl.get_data(ix,iy);
// 	    if (ix<0) left+=cl.get_data(ix,iy);
// 	    if (ix>0) right+=cl.get_data(ix,iy);
// 	    if (iy<0) bottom+=cl.get_data(ix,iy);
// 	    if (iy>0) top+=cl.get_data(ix,iy);
	    
// 	  }
// 	px=(-left+right)/tot;
// 	py=(-bottom+top)/tot;
// 	//max at 340
// 	if (tot>200) {
// 	  w=(tot+3.5*phs)/phw;
// 	} else
// 	  w=0;
// 	if (w) {
// 	  hsp->Fill(tot);
// 	  if (w==1) {
// 	    h2->Fill(cl.x,cl.y,w);
// 	    hint->Fill(px+cl.x,py+cl.y,w);
// 	  }
	 
// 	  h2mult->Fill(cl.x,cl.y,w);
	    
// 	  // if (cl.y<350)
// 	  //   hff->Fill(px,py,w);
// 	//}
// 	  iph+=w;
// 	  if (iph%100000==0) {
// 	    // c->Modified();
// 	    // c->Update();
// 	    c1->Modified();;
// 	    c1->Update();
// 	    // c2->Modified();;
// 	    // c2->Update();
// 	  }
// 	}
// 	//	if (iph>0.5E7) break;
//     }
//     fclose(f);
//     // hff->Scale(hff->GetNbinsX()*hff->GetNbinsY()/hff->Integral());
//     // TH2F *hint2=(TH2F*)hint->Clone("hint2");
//     // double ff;
//     // for (int ibx=0; ibx<hint->GetNbinsX(); ibx++) {
//     //   for (int iby=0; iby<hint->GetNbinsY(); iby++) {
//     // 	ff=hff->GetBinContent(ibx%ns+1, iby%ns+1);
//     // 	//	cout << ibx << " " << iby << " " << ibx%ns << " " << iby%ns << " " << ff << endl;
//     // 	if (ff>0)
//     // 	  hint2->SetBinContent(ibx+1, iby+1,hint->GetBinContent(ibx+1,iby+1)/ff); 
//     //   }
//     // }


    



//     return h2;
  
//   } else
//     cout << "could not open file " << fname << endl;
//   return NULL;


// }
