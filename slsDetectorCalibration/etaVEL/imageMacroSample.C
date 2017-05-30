{
  // TFile ff("/local_zfs_raid/tomcat_20160528/trees/img_blank_eta.root");
  // TH2D *hff=(TH2D*)ff.Get("imgHR");
  // TFile ff("/local_zfs_raid/tomcat_20160528/trees/img_blank_eta_gmap_v2.root");
  // TH2D *hff=(TH2D*)ff.Get("blankHR");
  // hff->SetName("imgBlank");
  // TH2D *hpixel=new TH2D("hpixel","hpixel",25,0,25,25,0,25);
  // for (int ibx=10*25; ibx<hff->GetNbinsX()-10*25; ibx++) {
  //   for (int iby=20*25; iby<hff->GetNbinsY()-20*25; iby++) {
  //     hpixel->Fill((ibx-12)%25,(iby-12)%25,hff->GetBinContent(ibx+1,iby+1));
  //   }
  // }
  // hpixel->Scale(1./hpixel->GetBinContent(13,13));

  // // new TCanvas();
  // // hpixel->Draw("colz"); 

  // TH2D *hraw=(TH2D*)hff->Clone("hraw");
  // TH2D *hpix=(TH2D*)hff->Clone("hpix");

  // for (int ibx=0; ibx<hff->GetNbinsX(); ibx++) {
  //   for (int iby=0; iby<hff->GetNbinsY(); iby++) {
  //     hpix->SetBinContent(ibx+1,iby+1,hpixel->GetBinContent(hpixel->GetXaxis()->FindBin((ibx-12)%25),hpixel->GetXaxis()->FindBin((iby-12)%25)));
  //   }
  // }

  // hff->Divide(hpix);

  // Double_t nf=hff->Integral(100,hff->GetNbinsX()-100,hff->GetYaxis()->FindBin(130), hff->GetYaxis()->FindBin(140))/((hff->GetNbinsX()-200)*250);
  // hff->Scale(1./nf);

  //  new TCanvas();
  // hff->Draw("colz");


  // TFile fg("/local_zfs_raid/tomcat_20160528/trees/img_grating_2d_eta.root");
  // TH2D *hg=(TH2D*)fg.Get("imgHR");
  TFile fg("/mnt/moench_data/tomcat_20160528_img/img_sample_eta_gmap_v2.root");
  TH2D *hg=(TH2D*)fg.Get("sampleHR");
  hg->SetName("imgSample");

  // hg->Divide(hpix);




  // hg->Divide(hff);
  //hsg->Divide(hff);

  Double_t ng=hg->Integral(100,hg->GetNbinsX()-100,hg->GetYaxis()->FindBin(130), hg->GetYaxis()->FindBin(140))/((hg->GetNbinsX()-200)*250);
 
   hg->Scale(1/ng);

   // new TCanvas();
   // hsg->SetMaximum(1.);
   // hsg->Draw("colz");
   
  //  TH1D *pg=hg->ProjectionX("pg",hg->GetYaxis()->FindBin(174.5)+1,hg->GetYaxis()->FindBin(175.5));
  //  TH1D *psg=hsg->ProjectionX("psg",hsg->GetYaxis()->FindBin(174.5)+1,hsg->GetYaxis()->FindBin(175.5));
  //  psg->SetLineColor(2);
  // new TCanvas();
  
  // pg->Draw("l");
  // psg->Draw("l same");
   TH2D *hpixel1=new TH2D("hpixel1","hpixel1",25,0,25,25,0,25);
  for (int ibx=10*25; ibx<35*25; ibx++) {
    for (int iby=25*25; iby<50*25; iby++) {
      hpixel1->Fill((ibx-12)%25,(iby-12)%25,hg->GetBinContent(ibx+1,iby+1));
    }
  }
  hpixel1->Scale(1./hpixel1->GetBinContent(13,13));

  // new TCanvas();
  // hpixel->Draw("colz"); 

  TH2D *hpix1=(TH2D*)hg->Clone("hpix1");

  for (int ibx=0; ibx<hg->GetNbinsX(); ibx++) {
    for (int iby=0; iby<hg->GetNbinsY(); iby++) {
      hpix1->SetBinContent(ibx+1,iby+1,hpixel1->GetBinContent(hpixel1->GetXaxis()->FindBin((ibx-12)%25),hpixel1->GetXaxis()->FindBin((iby-12)%25)));
    }
  }

  hg->Divide(hpix1);

  new TCanvas();
  hg->SetMaximum(4);
  hg->SetMinimum(0.5);
  hg->Draw("colz");
  

  int nbx=hg->GetNbinsX();
  int nby=hg->GetNbinsY();
  char buffer[nbx];
  cout << "Size of short int is "<<sizeof(char)<< endl;
  cout << "width is "<<nbx<< endl;
  cout << "height is "<<nby<< endl;
  ofstream myFile ("sample.bin", ios::out | ios::binary);
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
