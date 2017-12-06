#include "EtaVEL.h"
#include <iomanip>


ClassImp(EtaVEL);

double Median(const TH1D * histo1) {

  int numBins = histo1->GetXaxis()->GetNbins();
  Double_t *x = new Double_t[numBins];
  Double_t* y = new Double_t[numBins];
  for (int i = 0; i < numBins; i++) {
    x[i] = histo1->GetBinCenter(i);
    y[i] = histo1->GetBinContent(i);
  }
  return TMath::Median(numBins, x, y);
}


double *EtaVEL::getPixelCorners(int x, int y){
  double tlX,tlY,trX,trY,blX,blY,brX,brY;
  tlX = xPPos[getCorner(x,y+1)];
  tlY = yPPos[getCorner(x,y+1)];
  trX = xPPos[getCorner(x+1,y+1)];
  trY = yPPos[getCorner(x+1,y+1)];
  blX = xPPos[getCorner(x,y)];
  blY = yPPos[getCorner(x,y)];
  brX = xPPos[getCorner(x+1,y)];
  brY = yPPos[getCorner(x+1,y)];

  //cout << "gPC: TL: " << getCorner(x,y+1) << " TR: " << getCorner(x+1,y+1) << " BL " << getCorner(x,y) << " BR " << getCorner(x+1,y) << endl;

  double *c = new double[8];
  c[0] = tlX; c[1] = trX; c[2] = brX; c[3] = blX; 
  c[4] = tlY; c[5] = trY; c[6] = brY; c[7] = blY;
  return c;
}


int EtaVEL::findBin(double xx, double yy){

  double tlX,tlY,trX,trY,blX,blY,brX,brY;
  /********Added by anna ******/
  // if (xx<min) xx=min+1E-6; 
  // if (xx>max) xx=max-1E-6;
  // if (yy<min) yy=min+1E-6;
  // if (yy>max) yy=max-1E-6;
  /**************/


  int bin = -1;
  for(int x = 0; x < nPixels; x++){
    for(int y = 0; y < nPixels; y++){
      double *c = getPixelCorners(x,y);
      tlX = c[0]; trX = c[1]; brX = c[2]; blX = c[3];
      tlY = c[4]; trY = c[5]; brY = c[6]; blY = c[7];

      ///if(y == 0){
	//	cout << "x: " << x << " blY " << blY << " brY " << brY << endl;
	//}
      
      int out = 0;
      
      double tb = 0;
      double bb = 0;
      double lb = 0;
      double rb = 0;
      
      if((trX-tlX)>0.)
	tb = (trY - tlY)/(trX-tlX);
     
      if((brX-blX)>0.)
	bb = (brY - blY)/(brX-blX);	
      
      if((tlY-blY)>0.)
	lb = (tlX - blX)/(tlY-blY);
      
      if((trY-brY)>0.)
	rb = (trX - brX)/(trY-brY);
      
      double ty = tlY + tb * (xx - tlX);
      double by = blY + bb * (xx - blX);
      
      double lx = blX + lb * (yy - blY);
      double rx = brX + rb * (yy - brY);
      
      


      if(yy >= ty) out++;
      if(yy <  by) out++;
      if(xx <  lx) out++;
      if(xx >= rx) out++;
	
      //cout << "ty " << ty << endl;
      //cout << "by " << by << endl;
      //cout << "lx " << lx << endl;
      //cout << "rx " << rx << endl;

      //double dist = (xx - xPPos[getBin(x,y)]) * (xx - xPPos[getBin(x,y)]) + (yy - yPPos[getBin(x,y)]) * (yy - yPPos[getBin(x,y)]);
      //cout << "x " << x << " y " << y << " out " << out << " ty " << ty  << endl;
      //cout << "tl " << tlX << "/" << tlY << " tr " << trX << "/" << trY << endl;
      //cout << "bl " << blX << "/" << blY << " br " << brX << "/" << brY << endl;

      //cout << " tb " << tb << endl;


      delete[] c;
      if(out == 0){ return getBin(x,y); }
    }
  }

  return -1;
}

void EtaVEL::createLogEntry(){
  if(it >= nIterations){
    cerr << "log full" << endl;
  }
  log[it].itN = it;
  log[it].xPos = new double[nPixels*nPixels+1];
  log[it].yPos = new double[nPixels*nPixels+1];
  log[it].binCont = new double[nPixels*nPixels+1];
  for(int x = 0; x < nPixels; x++)
    for(int y = 0; y < nPixels; y++){
      log[it].xPos[getBin(x,y)] = xPPos[getBin(x,y)];
      log[it].yPos[getBin(x,y)] = yPPos[getBin(x,y)];
      log[it].binCont[getBin(x,y)] = binCont[getBin(x,y)];
    }
  it++;
}

void EtaVEL::updatePixelCorner(){
  double w = 20;
  int rows = (nPixels+1)*(nPixels+1) + 4 + 4 * 4;//(4*(nPixels+1))-4;
  int cols = (nPixels+1)*(nPixels+1);

  double *rVx = new double[rows];
  double *rVy = new double[rows];
  
  double *posMat = new double[rows*cols];
  for(int i = 0 ; i < rows*cols; i++) posMat[i] = 0;
  int boundaryPoint = 0;

  cout << "linear sys stuff" << endl;

  double minELength = 100000000000000; int minX=-1, minY=-1;

  for(int y = 0; y < nPixels+1; y++){
    for(int x = 0; x < nPixels+1; x++){
      double bx = 0, by = 0;

      //boundary conditions

      if((x == 0 && y % 5 == 0) || 
	 (x == nPixels && y % 5 == 0) || 
	 (y == 0 && x % 5 == 0) || 
	 (y == nPixels && x % 5 == 0)){ 

	bx = xPPos[getCorner(x,y)]; 
	//cout << "bP " << boundaryPoint << " bx " << bx << endl;
	by = yPPos[getCorner(x,y)]; 
	rVx[(nPixels+1)*(nPixels+1) + boundaryPoint] = bx*w;
	rVy[(nPixels+1)*(nPixels+1) + boundaryPoint] = by*w;
	posMat[(nPixels+1)*(nPixels+1)*cols + boundaryPoint * cols + getCorner(x,y)-1] = w;
	boundaryPoint++;
      }

      double tot = 4 - (x == 0) - (y == 0) - (x == nPixels) - (y == nPixels);
      //cout << "totW: " << tot << endl;
      //tot = 4.;
      double eLength = 0;
      if(x != 0) eLength += edgeL[getEdgeX(x-1,y)];
      if(y != 0) eLength += edgeL[getEdgeY(x,y-1)];
      if(x != nPixels) eLength += edgeL[getEdgeX(x,y)];
      if(y != nPixels) eLength += edgeL[getEdgeY(x,y)];

      /*cout << "Corner X:" <<x << " Y: " << y ;
      cout << " C# " << getCorner(x,y);
      cout << " eXl " << getEdgeX(x-1,y) << "(C# " << getCorner(x-1,y) << " ) ";
      cout << " eXr " << getEdgeX(x,y) << "(C# " << getCorner(x+1,y) << " ) ";
      cout << " eYb " << getEdgeY(x,y-1) << "(C# " << getCorner(x,y-1) << " ) ";
      cout << " eYt " << getEdgeY(x,y) << "(C# " << getCorner(x,y+1) << " ) " << endl; */
	//" totW: " << tot << " totE: " << eLength << endl;

      if(eLength < minELength & tot == 4){
	minELength = eLength;
	minX = x; minY = y;
      }


      //matrixes updated
      if(x != 0) posMat[y*(nPixels+1)*cols+x*cols+getCorner(x-1,y)-1] = -edgeL[getEdgeX(x-1,y)]/eLength;
      if(y != 0) posMat[y*(nPixels+1)*cols+x*cols+getCorner(x,y-1)-1] = -edgeL[getEdgeY(x,y-1)]/eLength;;
      if(x != nPixels) posMat[y*(nPixels+1)*cols+x*cols+getCorner(x+1,y)-1] = -edgeL[getEdgeX(x,y)]/eLength;;
      if(y != nPixels) posMat[y*(nPixels+1)*cols+x*cols+getCorner(x,y+1)-1] = -edgeL[getEdgeY(x,y)]/eLength;;

      posMat[y*(nPixels+1)*cols+x*cols+getCorner(x,y)-1] = 1.;
      rVx[getCorner(x,y)-1] = 0.;
      rVy[getCorner(x,y)-1] = 0.;


    }
  }

  cout << "Min Corner X: " <<minX << " Y: " << minY  << " C# " << getCorner(minX,minY) << " length " << minELength << endl;

  TMatrixD *k = new TMatrixD(rows,cols);
  TVectorD *fx = new TVectorD(rows,rVx);
  TVectorD *fy = new TVectorD(rows,rVy);
  //  f->Print();
  k->SetMatrixArray(posMat);
  //  k->Print();


  //solve linear system

  Bool_t ok;
  TDecompSVD  *s = new TDecompSVD(*k);
  s->Solve(*fx);
  s->Solve(*fy);

  double *fxA = fx->GetMatrixArray();
  double *fyA = fy->GetMatrixArray();
  

  for(int y = 0; y < nPixels+1; y++){
    for(int x = 0; x < nPixels+1; x++){      
      //do not update boundaries

      if(!(x == 0 || 
	   x == nPixels|| 
	   y == 0 || 
	   y == nPixels)){ 
	xPPos[getCorner(x,y)] = fxA[getCorner(x,y)-1];
	yPPos[getCorner(x,y)] = fyA[getCorner(x,y)-1];
      }
    }
  }
}

void EtaVEL::updatePixelPos(){
  double xMov, yMov, d1Mov, d2Mov;
  createLogEntry();
  double *chMap = getChangeMap();
  int ch =0;

  cout << "update edge lengths" << endl;
  for(int x = 0; x < nPixels; x++)
    for(int y = 0; y < nPixels; y++){
      

      /*cout << "Pixel X:" <<x << " Y: " << y << " P# " << getBin(x,y) << " eXb " << getEdgeX(x,y);
      cout << " eXt " << getEdgeX(x,y+1) << " eYl " << getEdgeY(x,y) << " eYr " << getEdgeY(x+1,y) << endl;
      */

      edgeL[getEdgeX(x,y)] *= chMap[getBin(x,y)];
      edgeL[getEdgeX(x,y+1)] *= chMap[getBin(x,y)];
      edgeL[getEdgeY(x,y)] *= chMap[getBin(x,y)];
      edgeL[getEdgeY(x+1,y)] *= chMap[getBin(x,y)];

      //cout << "Pixel x: " << x << " y: " << y << " Ch: " << chMap[getBin(x,y)] << " counts: " << binCont[getBin(x,y)] << endl;
      //cout << "BE " << getEdgeX(x,y) << endl;
      //cout << "TE " << getEdgeX(x,y+1) << endl;
      //cout << "LE " << getEdgeY(x,y) << endl;
      //cout << "RE " << getEdgeY(x+1,y) << endl;
      binCont[getBin(x,y)] = 0;
    }
  
  updatePixelCorner();
  
  //double *pSize = getSizeMap();
  double totEdgeLength = 0;
  for(int e = 1; e < 2*nPixels*(nPixels+1)+1; e++){
    totEdgeLength += edgeL[e];
  }
  cout << "tot edge Length: " << totEdgeLength << endl;

  totCont = 0.;
  
}

double *EtaVEL::getSizeMap(){
  double tlX,tlY,trX,trY,blX,blY,brX,brY;
  double *szMap = new double[nPixels*nPixels+1];
  for(int x = 1; x < nPixels-1; x++)
    for(int y = 1; y < nPixels-1; y++){
      double *c = getPixelCorners(x,y);
      tlX = c[0]; trX = c[1]; brX = c[2]; blX = c[3];
      tlY = c[4]; trY = c[5]; brY = c[6]; blY = c[7];
      
      //double area = dtl * dtr / 2. + dtr * dbr / 2. + dbr * dbl / 2. + dbl * dtl / 2.;

      //http://en.wikipedia.org/wiki/Shoelace_formula
      double sl1 = tlX * trY + trX * brY + brX * blY + blX * tlY;
      double sl2 = tlY * trX + trY * brX + brY * blX + blY * tlX;
      double area = 1./2. * (- sl1 + sl2);
      if(area < 0.){
	cout << "negative area: X " << x << " Y " << y << " area " << endl;
	edgeL[getEdgeX(x,y)] *= 2.;
	edgeL[getEdgeX(x,y+1)] *= 2.;
	edgeL[getEdgeY(x,y)] *= 2.;
	edgeL[getEdgeY(x+1,y)] *= 2.;

      }
      szMap[getBin(x,y)] = area / (max - min) / (max - min) * nPixels * nPixels;
      delete[] c;
      
    }
  return szMap;
}

double *EtaVEL::getChangeMap(){
  double *chMap = new double[nPixels*nPixels+1];
  double avg = totCont/(double)(nPixels*nPixels);
  // TH1D *hmed=getCounts();
  // double med = Median(hmed);
  // delete hmed;
  double acc = TMath::Sqrt(avg);
  cout << "totC: " << totCont << " avg " << avg  << "  acc: " << acc << endl;//<< " med " << med 
  double totOffAcc = 0.;
  int totInRange03s = 0;
  int totInRange07s = 0;
  int totInRange12s = 0;
  int totInRange20s = 0;
  int totInRange25s = 0;
  double dd;
  int totInBins = 0;

  //double 
  chi_sq=0;

  int maxC = 0, maxX=-1, maxY=-1;
  double minC = 1000000000000000, minX, minY;

  for(int x = 0; x < nPixels; x++){
    for(int y = 0; y < nPixels; y++){
      totInBins += binCont[getBin(x,y)];
      double r = (double)binCont[getBin(x,y)];
      if(r > 0. & totCont > 0.){
	dd=sqrt(r/avg);
	/**Added by Anna */
	if (dd>2.) dd=1.5;
	if (dd<0.5) dd=0.75;
	chMap[getBin(x,y)] = dd;
	/** */
	//if( chMap[getBin(x,y)] < 1.){ chMap[getBin(x,y)] = 1/1.2; }
	//if( chMap[getBin(x,y)] > 1.){ chMap[getBin(x,y)] = 1.2; }
	//if( chMap[getBin(x,y)] < 1/1.2){ chMap[getBin(x,y)] = 1/1.2; }
	//if( chMap[getBin(x,y)] > 1.2){ chMap[getBin(x,y)] = 1.2; }
      }else if(totCont > 0.){
	chMap[getBin(x,y)] =0.5; //1/1.2;
      }else{
	chMap[getBin(x,y)] = 1.;
      }
      
      //if(r < avg + 2*acc && r > avg - 2*acc){ totInRange++;}//	chMap[getBin(x,y)] = 1.; }

      /** Commente away by Anna
      if(converged == 0 && r < med+20*acc){ chMap[getBin(x,y)] = 1.; }
      if(converged == 2 && r < med+20*acc && r > med-03*acc){ chMap[getBin(x,y)] = 1.; }
      if(r < med+03*acc){ totInRange03s++;  }
      if(r < med+07*acc){ totInRange07s++;  }
      if(r < med+12*acc){ totInRange12s++;  }
      if(r < med+20*acc){ totInRange20s++;  }
      if(r < med+25*acc){ totInRange25s++;  }
      */

      //cout << "x " << x << " y " << y << " r " << r << " ch " << chMap[getBin(x,y)] << endl;
      //      if(r - avg >  acc){ totOffAcc += r-avg;}
      //if(r - avg < -acc){ totOffAcc += avg-r;}
      totOffAcc += (avg-r)*(avg-r);
      chi_sq+=(avg-r)*(avg-r)/r;
      //cout << " x " << x << " y " << y << " bC " << binCont[x*nPixels+y]   <<  " r " << r << endl;

      if(r > maxC){ maxC = r; maxX = x; maxY = y; }
      if(r < minC){minC = r; minX = x; minY = y; }

    }
  }
  //  cout << "totInBins " << totInBins << " zero Bin " << binCont[0] << endl;
  cout << "AvgOffAcc: " << sqrt(totOffAcc/(double)(nPixels*nPixels)) << endl;
  cout << "***********Reduced Chi Square: " << chi_sq/((double)(nPixels*nPixels)) << endl;
  // cout << "totInRange03 (<" << med+03*acc << "): " << totInRange03s << endl;
  // cout << "totInRange07 (<" << med+07*acc << "): " << totInRange07s << endl;
  // cout << "totInRange12 (<" << med+12*acc << "): " << totInRange12s << endl;
  // cout << "totInRange20 (<" << med+20*acc << "): " << totInRange20s << endl;
  // cout << "totInRange25 (<" << med+25*acc << "): " << totInRange25s << endl;
  double maxSig = (maxC - avg)*(maxC - avg) / avg;//acc; 
  double minSig = (avg - minC)*(avg - minC) / avg;//acc; 
  cout << "Max Pixel X: " << maxX << " Y: " << maxY << " P# " << getBin(maxX,maxY) << " count: " << maxC << " sig: "<< maxSig << endl;
  cout << "Min Pixel X: " << minX << " Y: " << minY << " P# " << getBin(minX,minY) << " count: " << minC << " sig: "<< minSig << endl;

  //  if(maxSig <= 25){ converged = 2; cout << "reached first converstion step!!!" << endl; }
  //if(minSig <= 7 && converged == 2) { converged = 1; }
  if (chi_sq<3) converged=2;
  if (chi_sq<1) converged=1;
  cout << "Conversion step "<< converged << endl;
  return chMap;
}

TH2D *EtaVEL::getContent(int it, int changeType){
  TH2D *cont = new TH2D("cont","cont",nPixels,min,max,nPixels,min,max);
  double *chMap = NULL;
  if(changeType ==1) chMap = getChangeMap();
  double *szMap = getSizeMap();
  for(int x = 0; x < nPixels; x++)
    for(int y = 0; y < nPixels; y++){
      if(changeType ==2 ){	  
	  cont->SetBinContent(x+1,y+1,szMap[getBin(x,y)]);
      }
      if(changeType ==1 ){	  
	cont->SetBinContent(x+1,y+1,chMap[getBin(x,y)]);
      }
      if(changeType ==0 ){	  
	if(it == -1){	  
	  cont->SetBinContent(x+1,y+1,binCont[getBin(x,y)]);	
	  //cout << "x " << x << " y " << y << " cont " << binCont[getBin(x,y)] << endl;
	}
	else{cont->SetBinContent(x+1,y+1,log[it].binCont[getBin(x,y)]);}
      }
    }
  return cont;
}

TH1D *EtaVEL::getCounts(){
  TH1D *ch = new TH1D("ch","ch",500,0,totCont/(nPixels*nPixels)*4);
  for(int x = 0; x < nPixels; x++)
    for(int y = 0; y < nPixels; y++){
      ch->Fill(binCont[getBin(x,y)]);
    }
  return ch;

}

void EtaVEL::printGrid(){

  double *colSum = new double[nPixels+1];
  double *rowSum = new double[nPixels+1];

  for(int i = 0; i < nPixels+1; i++){
    colSum[i] = 0.;
    rowSum[i] = 0.;
    for(int j = 0; j < nPixels; j++){
      rowSum[i] += edgeL[getEdgeX(j,i)];
      colSum[i] += edgeL[getEdgeY(i,j)];
    }
  }

  cout << endl;

  cout.precision(3); cout << fixed;
  cout << "     ";
  for(int x = 0; x < nPixels+1; x++){
    cout << setw(2) <<  x << " (" << colSum[x] << ")                ";
  }
  cout << endl;
  for(int y = 0; y < nPixels+1; y++){
    cout << setw(2) << y << " ";
    for(int x = 0; x < nPixels+1; x++){
      cout << "(" << xPPos[getCorner(x,y)] << "/" << yPPos[getCorner(x,y)] << ") " ;
      if(x < nPixels) cout << " -- " << edgeL[getEdgeX(x,y)]/rowSum[y]*(max-min) << " -- ";
    }
    cout << " | " << rowSum[y] << endl;

    if(y < nPixels){
      cout << "      ";
      for(int x = 0; x < nPixels+1; x++){
	cout << edgeL[getEdgeY(x,y)]/colSum[x]*(max-min) << "                      ";
      }
      cout << endl;
    }

  }
  delete[] colSum;
  delete[] rowSum;

}

TMultiGraph *EtaVEL::plotPixelBorder(int plotCenters){
  TMultiGraph *mg = new TMultiGraph();
  double cx[5], cy[5];
  for(int x = 0; x < nPixels; x++)
    for(int y = 0; y < nPixels; y++){
      double *c = getPixelCorners(x,y);
      cx[0]=c[0]; cx[1]=c[1]; cx[2]=c[2]; cx[3]=c[3]; cx[4]=c[0];
      cy[0]=c[4]; cy[1]=c[5]; cy[2]=c[6]; cy[3]=c[7]; cy[4]=c[4];


      TGraph *g = new TGraph(5,cx,cy);
      mg->Add(g);
      if(plotCenters){
	g = new TGraph(1,&(xPPos[getBin(x,y)]),&(yPPos[getBin(x,y)]));
	mg->Add(g);
      }
      delete[] c;
    }
  return mg;
}

TMultiGraph *EtaVEL::plotLog(int stepSize, int maxIt){
  int mIt;
  TMultiGraph *mg = new TMultiGraph();
  double **xposl = new double*[nPixels*nPixels+1];
  double **yposl = new double*[nPixels*nPixels+1];
  if(maxIt==-1){ mIt = it; } else{ mIt = maxIt; };
  cout << "mIt " << mIt << " steps " << mIt/stepSize << endl;
  for(int x = 0; x < nPixels; x++){
    for(int y = 0; y < nPixels; y++){
      xposl[getBin(x,y)] = new double[mIt/stepSize];
      yposl[getBin(x,y)] = new double[mIt/stepSize];
      for(int i = 0; i < mIt/stepSize; i++){
	xposl[getBin(x,y)][i] = log[i*stepSize].xPos[getBin(x,y)];
	yposl[getBin(x,y)][i] = log[i*stepSize].yPos[getBin(x,y)];
      }
      TGraph *g = new TGraph(mIt/stepSize,xposl[getBin(x,y)],yposl[getBin(x,y)]);
      g->SetLineColor((x*y % 9) + 1);
      
      if(x == 0) g->SetLineColor(2);
      if(y == 0) g->SetLineColor(3);
      if(x == nPixels-1) g->SetLineColor(4);
      if(y == nPixels-1) g->SetLineColor(5);
      mg->Add(g);
    }
  }
  return mg;
}

void EtaVEL::serialize(ostream &o){
  //      b.WriteVersion(EtaVEL::IsA());
  char del = '|';
  o << min << del;
  o << max << del;
  o << ds << del;
  o << nPixels << del;
  o << it << del;
  o << totCont << del;
  for(int i = 0; i < (nPixels+1)*(nPixels+1)+1; i++){
    o << xPPos[i] << del;
    o << yPPos[i] << del;	
  }
  for(int i = 0; i < nPixels*nPixels+1; i++){
    o << binCont[i] << del;
  }

  for(int i = 0; i < it; i++){
    o << log[i].itN << del;
    for(int j = 0; j < (nPixels+1)*(nPixels+1)+1; j++){
      o << log[i].xPos[j] << del;
      o << log[i].yPos[j] << del;
    }	
    for(int j = 0; j < nPixels*nPixels+1; j++){
      o << log[i].binCont[j] << del;
    }	
  }
}

void EtaVEL::deserialize(istream &is){
  delete[] xPPos;
  delete[] yPPos;
  delete[] binCont;

  char del;

  is >> min >> del;
  is >> max >> del;
  is >> ds >> del;
  is >> nPixels >> del;
  is >> it >> del;
  is >> totCont >> del;
  
  xPPos = new double[(nPixels+1)*(nPixels+1)+1];
  yPPos = new double[(nPixels+1)*(nPixels+1)+1];
  binCont = new double[nPixels*nPixels+1];

  cout << "d";

  for(int i = 0; i < (nPixels+1)*(nPixels+1)+1; i++){
    is >> xPPos[i] >> del;
    is >> yPPos[i] >> del;	
  }

  cout << "d";

  for(int i = 0; i < nPixels*nPixels+1; i++){
    is >> binCont[i] >> del;
  }
  
  cout << "d";

  for(int i = 0; i < it; i++){
    is >> log[i].itN >> del;
    log[i].xPos = new double[(nPixels+1)*(nPixels+1)+1];
    log[i].yPos = new double[(nPixels+1)*(nPixels+1)+1];
    log[i].binCont = new double[nPixels*nPixels+1];
    
    for(int j = 0; j < (nPixels+1)*(nPixels+1)+1; j++){
      is >> log[i].xPos[j] >> del;
      is >> log[i].yPos[j] >> del;
    }	
    for(int j = 0; j < nPixels*nPixels+1; j++){
      is >> log[i].binCont[j] >> del;
    }
    cout << "d";
  }
  cout << endl;
}

void EtaVEL::Streamer(TBuffer &b){
   if (b.IsReading()) {
      Version_t v = b.ReadVersion();
      
      delete[] xPPos;
      delete[] yPPos;
      delete[] binCont;

      b >> min;
      b >> max;
      b >> ds;
      b >> nPixels;
      b >> it;
      b >> totCont;

      xPPos = new double[(nPixels+1)*(nPixels+1)+1];
      yPPos = new double[(nPixels+1)*(nPixels+1)+1];
      binCont = new double[nPixels*nPixels+1];

      for(int i = 0; i < (nPixels+1)*(nPixels+1)+1; i++){
	b >> xPPos[i];
	b >> yPPos[i];	
      }
      for(int i = 0; i < nPixels*nPixels+1; i++){
	b >> binCont[i];
      }

      for(int i = 0; i < it; i++){
	b >> log[i].itN;
	log[i].xPos = new double[(nPixels+1)*(nPixels+1)+1];
	log[i].yPos = new double[(nPixels+1)*(nPixels+1)+1];
	log[i].binCont = new double[nPixels*nPixels+1];

	for(int j = 0; j < (nPixels+1)*(nPixels+1)+1; j++){
	  b >> log[i].xPos[j];
	  b >> log[i].yPos[j];
	}	
	for(int j = 0; j < nPixels*nPixels+1; j++){
	  b >> log[i].binCont[j];
	}
      }

   } else {
      b.WriteVersion(EtaVEL::IsA());
      b << min;
      b << max;
      b << ds;
      b << nPixels;
      b << it;
      b << totCont;
      for(int i = 0; i < (nPixels+1)*(nPixels+1)+1; i++){
	b << xPPos[i];
	b << yPPos[i];	
      }
      for(int i = 0; i < nPixels*nPixels+1; i++){
	b << binCont[i];
      }

      for(int i = 0; i < it; i++){
	b << log[i].itN;
	for(int j = 0; j < (nPixels+1)*(nPixels+1)+1; j++){
	  b << log[i].xPos[j];
	  b << log[i].yPos[j];
	}	
	for(int j = 0; j < nPixels*nPixels+1; j++){
	  b << log[i].binCont[j];
	}	
      }
   }
}

