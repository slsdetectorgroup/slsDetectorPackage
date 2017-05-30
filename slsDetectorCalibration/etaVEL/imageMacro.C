TH2D *imageMacro(char *name) {

  //TH2D *makeNorm(){

  //TFile ff("/local_zfs_raid/tomcat_20160528/trees/img_blank_eta_gmap.root");
  //TH2D *hff=(TH2D*)ff.Get("imgHR");
    TFile *ff=new TFile("/mnt/moench_data/tomcat_20160528_img/img_blank_eta_nb25.root");
  //  TFile ff("/local_zfs_raid/tomcat_20160528/trees/img_blank_eta_gcorr_nb25.root");
  TH2D *hff=(TH2D*)ff->Get("blankHR");
  hff->SetName("imgBlank");
  TH2D *hpixel=new TH2D("hpixel","hpixel",25,0,25,25,0,25);
  for (int ibx=10*25; ibx<hff->GetNbinsX()-10*25; ibx++) {
    for (int iby=20*25; iby<hff->GetNbinsY()-20*25; iby++) {
      hpixel->Fill((ibx-12)%25,(iby-12)%25,hff->GetBinContent(ibx+1,iby+1));
    }
  }
  hpixel->Scale(1./hpixel->GetBinContent(13,13));

  // new TCanvas();
  // hpixel->Draw("colz"); 

  //  TH2D *hraw=(TH2D*)hff->Clone("hraw");
  TH2D *hpix=(TH2D*)hff->Clone("hpix");

  for (int ibx=0; ibx<hff->GetNbinsX(); ibx++) {
    for (int iby=0; iby<hff->GetNbinsY(); iby++) {
      hpix->SetBinContent(ibx+1,iby+1,hpixel->GetBinContent(hpixel->GetXaxis()->FindBin((ibx-12)%25),hpixel->GetXaxis()->FindBin((iby-12)%25)));
    }
  }
  //  return hpix;
  //}

  //void imageMacro(char *name,TH2D *hpix=NULL){
  // hff->Divide(hpix);

  //  new TCanvas();
  // hff->Draw("colz");

  // if (hpix==NULL)
  //  hpix=makeNorm();
  TH2D *hg;

  char nn[1000];
  if (strcmp(name,"blank")==NULL) {
    hg=hff;
  }    else {
    
  
  sprintf(nn,"/mnt/moench_data/tomcat_20160528_img/img_%s_eta_nb25.root", name);
  // if (strcmp(name,"blank"))
    TFile *fg=new TFile(nn);
    // else
    // TFile fg=gDirectory;

  //TFile fg("/local_zfs_raid/tomcat_20160528/trees/img_grating_1d_eta_gmap.root");
  //TH2D *hg=(TH2D*)fg.Get("imgHR");
  //   TFile fg("/local_zfs_raid/tomcat_20160528/trees/img_grating_1d_eta_nb25.root");
  //  TFile fg("/local_zfs_raid/tomcat_20160528/trees/img_sample_eta_nb25.root");
  // TFile fg("/local_zfs_raid/tomcat_20160528/trees/img_grating_1d_eta_gcorr_nb25.root");
   sprintf(nn,"%sHR",name);
   TH2D *hg=(TH2D*)fg->Get(nn);
   // hg->SetName("imgGrating");

  //hg->Divide(hff);
  }
   if (hpix)
     hg->Divide(hpix);
   new TCanvas();
   hg->Draw("colz");

   return hg;
}


void imageMacro(TH2D *hg){

  Double_t imageData[200*400*25*25];
  const int nsigma=5.;
  int ip=0;
  int max=0;
  Double_t avg=0, rms=0;
    for (int iby=0; iby<hg->GetNbinsY(); iby++) {
   for (int ibx=0; ibx<hg->GetNbinsX(); ibx++) {
     imageData[ip]=hg->GetBinContent(ibx+1,iby+1);
     if (imageData[ip]>max) max=imageData[ip];
     //  if (imageData[ip]>3000) imageData[ip]=3000.;
     avg+=((Double_t)imageData[ip])/(hg->GetNbinsY()*hg->GetNbinsX());
     rms+=((Double_t)imageData[ip])*((Double_t)imageData[ip])/(hg->GetNbinsY()*hg->GetNbinsX());
     ip++;
    }
  }  
    rms=TMath::Sqrt(rms-avg*avg);
    ip=0;
    for (int iby=0; iby<hg->GetNbinsY(); iby++) {
      for (int ibx=0; ibx<hg->GetNbinsX(); ibx++) {
     if (imageData[ip]>avg+nsigma*rms) imageData[ip]=avg+nsigma*rms;
     //  if (imageData[ip]>3000) imageData[ip]=3000.;
     ip++;
    }
  }
    cout << "MAXIMUM IS "<< max << endl;
    cout << "AVERAGE IS "<< avg << endl;
    cout << "RMS IS "<< rms << endl;


 int nbx=hg->GetNbinsX();
int nby=hg->GetNbinsY();
Short_t *buffer=new Short_t[nbx];
//   // cout << "Size of short int is "<<sizeof(char)<< endl;
//   // cout << "width is "<<nbx<< endl;
//   // cout << "height is "<<nby<< endl;
  sprintf(nn,"%s_HR.bin", name);
 ofstream myFile (nn, ios::out | ios::binary);
 ip=0;
 for (int iy=0; iy<nby; iy++) {
   for (int ix=0; ix<nbx; ix++) {
     buffer[ix]=imageData[ip]*65535/nsigma/avg;
     ip++;
   }
   myFile.write((char*)buffer, nbx*sizeof(Short_t));
 }
 myFile.close();





 TASImage *img=new TASImage 	( "img",imageData, 400*25, 200*25);//		TImagePalette *  	palette = 0 
    new TCanvas();
    img->SetImageCompression(0);
    img->SetImageQuality(TAttImage::kImgBest);
    //  img->Gray(kTRUE);
    img->Draw();

  sprintf(nn,"%s_HR.tiff", name);

    img->WriteImage(nn, TImage::kTiff);	

    // new TCanvas();
    // hg->SetMaximum(3000);
    // hg->DrawCopy("colz");


    hg->Rebin2D(25,25);

    Double_t imageDataLR[200*400];

    ip=0; max=0;avg=0; rms=0;
    for (int iby=0; iby<hg->GetNbinsY(); iby++) {
   for (int ibx=0; ibx<hg->GetNbinsX(); ibx++) {
     imageDataLR[ip]=hg->GetBinContent(ibx+1,iby+1);
     if (imageDataLR[ip]>max) max=imageDataLR[ip];
     avg+=((Double_t)imageDataLR[ip])/(hg->GetNbinsY()*hg->GetNbinsX());
     rms+=((Double_t)imageDataLR[ip])*((Double_t)imageDataLR[ip])/(hg->GetNbinsY()*hg->GetNbinsX());
     ip++;
    }
    }
    rms=TMath::Sqrt(rms-avg*avg);
    ip=0;
    for (int iby=0; iby<hg->GetNbinsY(); iby++) {
      for (int ibx=0; ibx<hg->GetNbinsX(); ibx++) {
     if (imageDataLR[ip]>avg+nsigma*rms) imageDataLR[ip]=avg+nsigma*rms;
     //  if (imageData[ip]>3000) imageData[ip]=3000.;
     ip++;
    }
  }
    cout << "MAXIMUM IS "<< max << endl;
    cout << "AVERAGE IS "<< avg << endl;
    cout << "RMS IS "<< rms << endl;

    TASImage *imgLR=new TASImage 	( "imgLR",imageDataLR, 400, 200);//		TImagePalette *  	palette = 0 
    new TCanvas();
    imgLR->SetImageCompression(0);
    imgLR->SetImageQuality(TAttImage::kImgBest);
    // imgLR->Gray(kTRUE);
    imgLR->Draw();


    sprintf(nn,"%s_LR.tiff", name);

    imgLR->WriteImage(nn, TImage::kTiff);	


 int nbx1=hg->GetNbinsX();
int nby1=hg->GetNbinsY();
Short_t *buffer1=new Short_t[nbx1];
//   // cout << "Size of short int is "<<sizeof(char)<< endl;
//   // cout << "width is "<<nbx<< endl;
//   // cout << "height is "<<nby<< endl;
  sprintf(nn,"%s_LR.bin", name);
 ofstream myFile1 (nn, ios::out | ios::binary);
 ip=0;
 for (int iy=0; iy<nby1; iy++) {
   for (int ix=0; ix<nbx1; ix++) {
     buffer1[ix]=imageDataLR[ip]*256/nsigma/avg;
     ip++;
   }
   myFile1.write((char*)buffer1, nbx1*sizeof(Short_t));
 }
 myFile1.close();

 cout << sizeof(Short_t) << endl;
    //for grating2D the gratings are in the pixels (150-260)x(80-190)




//   //  hg->Draw("colz");
//   int off=13;
//   // hg->Divide(hpix);
//   int ix,iy, sbx, sby, ibx2, iby2, sbx2, sby2;
//   TH2F *hh13b=new TH2F("hh13b","hh13b",400*24,0,400,200*24,100,300);
//   TH2F *hh13=new TH2F("hh13","hh13",400*23,0,400,200*23,100,300);
//   TH2F *hh=new TH2F("hh","hh",400*21,0,400,200*21,100,300);
//   TH2F *h1=new TH2F("h1","h1",400*23,0,400,200*23,100,300);
//   for (int ibx=0; ibx<hg->GetNbinsX(); ibx++) {
//     for (int iby=0; iby<hg->GetNbinsY(); iby++) {
//       ix=(ibx-off)/25;
//       iy=(iby-off)/25;
//       sbx=(ibx-off)%25;
//       sby=(iby-off)%25;
//       sbx2=sbx-1;
//       sby2=sby-1;
//       if (sbx2<0) sbx2=0;
//       if (sby2<0) sby2=0;
//       if (sbx2>22) sbx2=22;
//       if (sby2>22) sby2=22;
//       ibx2=ix*23+(sbx2+off);
//       iby2=iy*23+(sby2+off);

//       hh13->Fill(hh13->GetXaxis()->GetBinCenter(ibx2+1),hh13->GetYaxis()->GetBinCenter(iby2+1),hg->GetBinContent(ibx+1,iby+1));
//       //  h1->Fill(h1->GetXaxis()->GetBinCenter(ibx2+1),h1->GetYaxis()->GetBinCenter(iby2+1),hpix->GetBinContent(ibx+1,iby+1));

//       off=13;

//       ix=(ibx-off)/25;
//       iy=(iby-off)/25;
//       sbx=(ibx-off)%25;
//       sby=(iby-off)%25;
//       sbx2=sbx-1;
//       sby2=sby-1;
//       // if (sbx2<0) sbx2=0;
//       // if (sby2<0) sby2=0;
//       ibx2=ix*24+(sbx2+off);
//       iby2=iy*24+(sby2+off);

// 	if (sbx2<0 && sby2>=0) {
// 	  ibx2=ix*24+(sbx2+off);
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.5*hg->GetBinContent(ibx+1,iby+1));
// 	  ibx2=ix*24+(sbx2+off)+1;
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.5*hg->GetBinContent(ibx+1,iby+1));
// 	} else if (sby2<0 && sbx2>=0){
// 	  iby2=iy*24+(sby2+off);
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.5*hg->GetBinContent(ibx+1,iby+1));
// 	  iby2=iy*24+(sby2+off)+1;
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.5*hg->GetBinContent(ibx+1,iby+1));
// 	}  else if (sby2<0 && sbx2<0){
// 	  iby2=iy*24+(sby2+off);
// 	  ibx2=ix*24+(sbx2+off);
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.25*hg->GetBinContent(ibx+1,iby+1));
// 	  iby2=iy*24+(sby2+off)+1;
// 	  ibx2=ix*24+(sbx2+off);
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.25*hg->GetBinContent(ibx+1,iby+1));
// 	  iby2=iy*24+(sby2+off);
// 	  ibx2=ix*24+(sbx2+off)+1;
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.25*hg->GetBinContent(ibx+1,iby+1));
// 	  iby2=iy*24+(sby2+off)+1;
// 	  ibx2=ix*24+(sbx2+off)+1;
// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),0.25*hg->GetBinContent(ibx+1,iby+1));
// 	}else {


// 	  hh13b->Fill(hh13b->GetXaxis()->GetBinCenter(ibx2+1),hh13b->GetYaxis()->GetBinCenter(iby2+1),hg->GetBinContent(ibx+1,iby+1));

// 	}

      

//       ix=(ibx-off)/25;
//       iy=(iby-off)/25;
//       sbx=(ibx-off)%25;
//       sby=(iby-off)%25;
//       sbx2=sbx-2;
//       sby2=sby-2;
//       if (sbx2<0) sbx2=-1;
//       if (sby2<0) sby2=-1;
//       if (sbx2>20) sbx2=-1;
//       if (sby2>20) sby2=-1;
//       ibx2=ix*21+(sbx2+off);
//       iby2=iy*21+(sby2+off);
//       if (sbx2>=0 && sby2>=0)  hh->Fill(hh->GetXaxis()->GetBinCenter(ibx2+1),hh->GetYaxis()->GetBinCenter(iby2+1),hg->GetBinContent(ibx+1,iby+1));
      
//     }
//   }  

//   new TCanvas();
//   hg->GetXaxis()->SetRangeUser(84,87);
//   hg->GetYaxis()->SetRangeUser(120,124);
//   hg->Draw("colz");
  




//   new TCanvas();
//   //  hh->Divide(h1);
//   hh->GetXaxis()->SetRangeUser(84,87);
//   hh->GetYaxis()->SetRangeUser(120,124);
//   hh->Draw("colz");


//   new TCanvas();
//   //  hh->Divide(h1);
//   hh13b->GetXaxis()->SetRangeUser(84,87);
//   hh13b->GetYaxis()->SetRangeUser(120,124);
//   hh13b->Draw("colz");

//   new TCanvas();
//   //  hh->Divide(h1);
//   hh13->GetXaxis()->SetRangeUser(84,87);
//   hh13->GetYaxis()->SetRangeUser(120,124);
//   hh13->Draw("colz");

//   // //TFile fsg("/local_zfs_raid/tomcat_20160528/trees/img_sample_grating_1d_eta_gmap.root");
//   // //TH2D *hsg=(TH2D*)fsg.Get("sample_grating_1dHR");
//   // TFile fsg("/local_zfs_raid/tomcat_20160528/trees/img_sample_grating_1d_eta.root");
//   // TH2D *hsg=(TH2D*)fsg.Get("imgHR");
//   // hsg->SetName("imgGratingSample");


//   // hsg->Divide(hpix);

//   // Double_t nf=hff->Integral(100,hff->GetNbinsX()-100,hff->GetYaxis()->FindBin(130), hff->GetYaxis()->FindBin(140))/((hff->GetNbinsX()-200)*250);
//   // hff->Scale(1./nf);

//   // // hg->Divide(hff);
//   // //hsg->Divide(hff);

//   // Double_t ng=hg->Integral(100,hg->GetNbinsX()-100,hg->GetYaxis()->FindBin(130), hg->GetYaxis()->FindBin(140))/((hg->GetNbinsX()-200)*250);
//   //  Double_t nsg=hsg->Integral(100,hsg->GetNbinsX()-100,hsg->GetYaxis()->FindBin(130), hsg->GetYaxis()->FindBin(140))/((hsg->GetNbinsX()-200)*250);

//   //  hg->Scale(1/ng);
//   //  hsg->Scale(1/nsg);

//   //  // new TCanvas();
//   //  // hg->SetMaximum(1.);
//   //  // hg->Draw("colz");
//   //  // new TCanvas();
//   //  // hsg->SetMaximum(1.);
//   //  // hsg->Draw("colz");
   
//   // //  TH1D *pg=hg->ProjectionX("pg",hg->GetYaxis()->FindBin(174.5)+1,hg->GetYaxis()->FindBin(175.5));
//   // //  TH1D *psg=hsg->ProjectionX("psg",hsg->GetYaxis()->FindBin(174.5)+1,hsg->GetYaxis()->FindBin(175.5));
//   // //  psg->SetLineColor(2);
//   // // new TCanvas();
  
//   // // pg->Draw("l");
//   // // psg->Draw("l same");
  
//   // int nbx=hg->GetNbinsX();
//   // int nby=hg->GetNbinsY();
//   // char buffer[nbx];
//   // cout << "Size of short int is "<<sizeof(char)<< endl;
//   // cout << "width is "<<nbx<< endl;
//   // cout << "height is "<<nby<< endl;
//   // ofstream myFile ("grating_1d.bin", ios::out | ios::binary);
//   // for (int iy=0; iy<nby; iy++) {
//   //   for (int ix=0; ix<nbx; ix++) {
//   //     buffer[ix]=hg->GetBinContent(ix+1,iy+1);
//   //   }
//   //   myFile.write((char*)buffer, nbx*sizeof(char));
//   // }
//   // myFile.close();


//   // nbx=hsg->GetNbinsX();
//   // nby=hsg->GetNbinsY();

//   // cout << "Size of short int is "<<sizeof(char)<< endl;
//   // cout << "width is "<<nbx<< endl;
//   // cout << "height is "<<nby<< endl;

//   // ofstream myFile1 ("sample_grating_1d.bin", ios::out | ios::binary);
//   // for (int iy=0; iy<nby; iy++) {
//   //   for (int ix=0; ix<nbx; ix++) {
//   //     buffer[ix]=hsg->GetBinContent(ix+1,iy+1);
//   //   }
//   //   myFile1.write((char*)buffer, nbx*sizeof(char));
//   // }
//   // myFile1.close();

// // NAME
// //        raw2tiff - create a TIFF file from a raw data

// // SYNOPSIS
// //        raw2tiff [ options ] input.raw output.tif

// // DESCRIPTION
// //        raw2tiff  converts  a  raw byte sequence into TIFF.  By default, the TIFF image is created with data samples packed (PlanarConfiguration=1), compressed with the PackBits algorithm (Compression=32773),
// //        and with each strip no more than 8 kilobytes.  These characteristics can overridden, or explicitly specified with the options described below.

// // OPTIONS
// //        -H number
// //               size of input image file header in bytes (0 by default). This amount of data just will be skipped from the start of file while reading.

// //        -w number
// //               width of input image in pixels (can be guessed, see GUESSING THE IMAGE GEOMETRY below).

// //        -l number
// //               length of input image in lines (can be guessed, see GUESSING THE IMAGE GEOMETRY below).

// //        -b number
// //               number of bands in input image (1 by default).

// //        -d data_type
// //               type of samples in input image, where data_type may be:
// //               byte     8-bit unsigned integer (default),
// //               short    16-bit unsigned integer,
// //               long     32-bit unsigned integer,
// //               sbyte    8-bit signed integer,
// //               sshort   16-bit signed integer,
// //               slong    32-bit signed integer,
// //               float    32-bit IEEE floating point,
// //               double   64-bit IEEE floating point.

// //        -i config
// //               type of samples interleaving in input image, where config may be:
// //               pixel   pixel interleaved data (default),
// //               band    band interleaved data.

// //        -p photo
// //               photometric interpretation (color space) of the input image, where photo may be:
// //               miniswhite   white color represented with 0 value,
// //               minisblack   black color represented with 0 value (default),
// //               rgb          image has RGB color model,
// //               cmyk         image has CMYK (separated) color model,
// //               ycbcr        image has YCbCr color model,
// //               cielab       image has CIE L*a*b color model,
// //               icclab       image has ICC L*a*b color model,
// //               itulab       image has ITU L*a*b color model.

// //        -s     swap bytes fetched from the input file.

// //        -L     input data has LSB2MSB bit order (default).

// //        -M     input data has MSB2LSB bit order.

// //        -c     Specify a compression scheme to use when writing image data: -c none for no compression, -c packbits for the PackBits compression algorithm (the default), -c jpeg for the baseline JPEG compres-
// //               sion algorithm, -c zip for the Deflate compression algorithm, and -c lzw for Lempel-Ziv & Welch.

// //        -r number
// //               Write data with a specified number of rows per strip; by default the number of rows/strip is selected so that each strip is approximately 8 kilobytes.
// //width is 10000
// //height is 5000


}
