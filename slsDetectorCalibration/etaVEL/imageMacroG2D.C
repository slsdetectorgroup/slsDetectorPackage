{ 
  TColor::InitializeColors();
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;//90;

  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  Double_t gray[NRGBs]  = { 1., 0.34, 0.61, 0.84, 1.00};
  Double_t zero[NRGBs]  = { 0., 0.0,0.0, 0.0, 0.00};
  //TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
   TColor::CreateGradientColorTable(NRGBs, stops, gray, gray, gray, NCont);
 
  gStyle->SetNumberContours(NCont);

  gStyle->SetPadTopMargin(0);
  gStyle->SetPadRightMargin(0);
  gStyle->SetPadBottomMargin(0);
  gStyle->SetPadLeftMargin(0);



  gROOT->ForceStyle();

  // TFile ff("/local_zfs_raid/tomcat_20160528/trees/img_blank_eta.root");
  // TH2D *hff=(TH2D*)ff.Get("imgHR");
  TFile ff("/mnt/moench_data/tomcat_20160528_img/img_blank_eta_nb25.root");
  TH2D *hff=(TH2D*)ff.Get("blankHR");
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

  TH2D *hraw=(TH2D*)hff->Clone("hraw");
  TH2D *hpix=(TH2D*)hff->Clone("hpix");

  for (int ibx=0; ibx<hff->GetNbinsX(); ibx++) {
    for (int iby=0; iby<hff->GetNbinsY(); iby++) {
      hpix->SetBinContent(ibx+1,iby+1,hpixel->GetBinContent(hpixel->GetXaxis()->FindBin((ibx-12)%25),hpixel->GetXaxis()->FindBin((iby-12)%25)));
    }
  }

  hff->Divide(hpix);

  //  new TCanvas();
  // hff->Draw("colz");


  // TFile fg("/local_zfs_raid/tomcat_20160528/trees/img_grating_2d_eta.root");
  // TH2D *hg=(TH2D*)fg.Get("imgHR");
  TFile fg("/mnt/moench_data/tomcat_20160528_img/img_grating_2d_eta_nb25.root");
  TH2D *hg=(TH2D*)fg.Get("grating_2dHR");
  hg->SetName("imgGrating");

  hg->Divide(hpix);



  Double_t nf=hff->Integral(100,hff->GetNbinsX()-100,hff->GetYaxis()->FindBin(130), hff->GetYaxis()->FindBin(140))/((hff->GetNbinsX()-200)*250);
  hff->Scale(1./nf);

  // hg->Divide(hff);
  //hsg->Divide(hff);

  Double_t ng=hg->Integral(100,hg->GetNbinsX()-100,hg->GetYaxis()->FindBin(130), hg->GetYaxis()->FindBin(140))/((hg->GetNbinsX()-200)*250);
 
   hg->Scale(1/ng);

   new TCanvas("c1","c1",800,800);
   hg->SetMaximum(2.);
   Double_t xmin=hg->GetXaxis()->GetXmin();
   Double_t xmax=hg->GetXaxis()->GetXmax();
   Double_t ymin=hg->GetYaxis()->GetXmin();
   Double_t ymax=hg->GetYaxis()->GetXmax();
   hg->SetTitle(";mm;mm;Normalized intensity (a.u.)");
   hg->GetXaxis()->Set(hg->GetXaxis()->GetNbins(),xmin*0.025-180.*0.025+0.1,xmax*0.025-180.*0.025+0.1);
   hg->GetYaxis()->Set(hg->GetYaxis()->GetNbins(),ymin*0.025-180.*0.025-1.5,ymax*0.025-180*0.025-1.5);
   
   hg->GetXaxis()->SetRangeUser(0,0.5);
   hg->GetYaxis()->SetRangeUser(0,0.5);
   
   hg->Draw("col");
   hg->SetStats(kFALSE);
   // new TCanvas();
   // hsg->SetMaximum(1.);
   // hsg->Draw("colz");
   
  //  TH1D *pg=hg->ProjectionX("pg",hg->GetYaxis()->FindBin(174.5)+1,hg->GetYaxis()->FindBin(175.5));
  //  TH1D *psg=hsg->ProjectionX("psg",hsg->GetYaxis()->FindBin(174.5)+1,hsg->GetYaxis()->FindBin(175.5));
  //  psg->SetLineColor(2);
  // new TCanvas();
  
  // pg->Draw("l");
  // psg->Draw("l same");
  
  int nbx=hg->GetNbinsX();
  int nby=hg->GetNbinsY();
  char buffer[nbx];
  cout << "Size of short int is "<<sizeof(char)<< endl;
  cout << "width is "<<nbx<< endl;
  cout << "height is "<<nby<< endl;
  ofstream myFile ("grating_2d.bin", ios::out | ios::binary);
  for (int iy=0; iy<nby; iy++) {
    for (int ix=0; ix<nbx; ix++) {
      buffer[ix]=hg->GetBinContent(ix+1,iy+1);
    }
    myFile.write((char*)buffer, nbx*sizeof(char));
  }
  myFile.close();


// NAME
//        raw2tiff - create a TIFF file from a raw data

// SYNOPSIS
//        raw2tiff [ options ] input.raw output.tif

// DESCRIPTION
//        raw2tiff  converts  a  raw byte sequence into TIFF.  By default, the TIFF image is created with data samples packed (PlanarConfiguration=1), compressed with the PackBits algorithm (Compression=32773),
//        and with each strip no more than 8 kilobytes.  These characteristics can overridden, or explicitly specified with the options described below.

// OPTIONS
//        -H number
//               size of input image file header in bytes (0 by default). This amount of data just will be skipped from the start of file while reading.

//        -w number
//               width of input image in pixels (can be guessed, see GUESSING THE IMAGE GEOMETRY below).

//        -l number
//               length of input image in lines (can be guessed, see GUESSING THE IMAGE GEOMETRY below).

//        -b number
//               number of bands in input image (1 by default).

//        -d data_type
//               type of samples in input image, where data_type may be:
//               byte     8-bit unsigned integer (default),
//               short    16-bit unsigned integer,
//               long     32-bit unsigned integer,
//               sbyte    8-bit signed integer,
//               sshort   16-bit signed integer,
//               slong    32-bit signed integer,
//               float    32-bit IEEE floating point,
//               double   64-bit IEEE floating point.

//        -i config
//               type of samples interleaving in input image, where config may be:
//               pixel   pixel interleaved data (default),
//               band    band interleaved data.

//        -p photo
//               photometric interpretation (color space) of the input image, where photo may be:
//               miniswhite   white color represented with 0 value,
//               minisblack   black color represented with 0 value (default),
//               rgb          image has RGB color model,
//               cmyk         image has CMYK (separated) color model,
//               ycbcr        image has YCbCr color model,
//               cielab       image has CIE L*a*b color model,
//               icclab       image has ICC L*a*b color model,
//               itulab       image has ITU L*a*b color model.

//        -s     swap bytes fetched from the input file.

//        -L     input data has LSB2MSB bit order (default).

//        -M     input data has MSB2LSB bit order.

//        -c     Specify a compression scheme to use when writing image data: -c none for no compression, -c packbits for the PackBits compression algorithm (the default), -c jpeg for the baseline JPEG compres-
//               sion algorithm, -c zip for the Deflate compression algorithm, and -c lzw for Lempel-Ziv & Welch.

//        -r number
//               Write data with a specified number of rows per strip; by default the number of rows/strip is selected so that each strip is approximately 8 kilobytes.
//width is 10000
//height is 5000


}
