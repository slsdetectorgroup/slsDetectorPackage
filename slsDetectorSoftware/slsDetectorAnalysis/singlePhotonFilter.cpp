  /********************************************//**
 * @file singlePhotonFilter.cpp
 * @short single photon filter using trees
 ***********************************************/
#include "singlePhotonFilter.h"


/*#define MOENCH_FRAME_INDEX_MASK 0xFFFFFF00
#define MOENCH_PACKET_INDEX_MASK 0xFF
#define MOENCH_FRAME_INDEX_OFFSET 8
*/

singlePhotonFilter::singlePhotonFilter(int x, int y, vector <vector<int16_t> >m, vector <vector<int16_t> >s, int d):
#ifdef MYROOT
										myTree(NULL),
										myFile(NULL),
#else
										myFile(NULL),
#endif
										nChannelsX(x),
										nChannelsY(y),
										nClusterX(CLUSTER_SIZE),
										nClusterY(CLUSTER_SIZE),
										map(m),
										dataSize(d),
										mask(s),
										nped(DEFAULT_NUM_PEDESTAL),
										nsigma(DEFAULT_SIGMA),
										nbackground(DEFAULT_BACKGROUND),
										outcorr(DEFAULT_CORRECTION),
										fnum(0),
										pnum(0),
										ptot(0),
										f0(0),
										frame_index_mask(0x00000000),
										packet_index_mask(0x00000000),
										frame_index_offset(0),
										packet_index_offset(0),
										packets_per_frame(0){


	if (x == 1)
		nClusterX = 1;

	stat.resize(x);
	for(int i=0; i<x; i++)
		stat[i].resize(y);

	//struct
	myPhotonHit.data.resize(nClusterX);
	for(int i=0; i<nClusterX; i++)
		myPhotonHit.data[i].resize(nClusterY);
	myPhotonHit.x 		= 0;
	myPhotonHit.y 		= 0;
	myPhotonHit.rms 	= 0;
	myPhotonHit.ped 	= 0;
	myPhotonHit.iframe 	= -1;
}





void singlePhotonFilter::initTree(char *outdir, char *fname){
	//path
	char outfname[MAX_STR_LENGTH];
	sprintf(outfname, "%s/%s.root", outdir, fname);

#ifdef MYROOT
	char c1[10],c2[10],cdata[100];
	sprintf(c1,"%d",nClusterX);
	sprintf(c2,"%d",nClusterY);
	sprintf(cdata,"data[%s][%s]/D",c1,c2);

	//file
	myFile = new TFile(outfname, "RECREATE"); /** later  return error if it exists */
	//tree
	myTree = new TTree(fname, fname);
	myTree->Branch("x",&myPhotonHit.x,"x/I");
	myTree->Branch("y",&myPhotonHit.y,"y/I");
	myTree->Branch("data",myPhotonHit.data,cdata);
	myTree->Branch("pedestal",&myPhotonHit.ped,"pedestal/D");
	myTree->Branch("rms",&myPhotonHit.rms,"rms/D");
#else
	myFile = fopen(outfname, "w");
#endif
	//initialize
	for (int ir=0; ir<nChannelsX; ir++){
		for (int ic=0; ic<nChannelsY; ic++){
			stat[ir][ic].Clear();
			stat[ir][ic].SetN(nbackground);
		}
	}
}



int singlePhotonFilter::writeToFile(){
#ifdef MYROOT
	if((myTree) && (myFile)){
		myTree->Write();

		myFile = myTree->GetCurrentFile();
		myFile->Close();
		delete myFile;
	}
#else
	if(myFile){ /*&& (number of structs?)*/
		fwrite((void*)(&myPhotonHit), 1, sizeof(myPhotonHit), myFile);
		fclose(myFile);
		delete myFile;
		return OK;
	}
#endif
	return FAIL;
}






int singlePhotonFilter::findHits(int16_t *myData, int myDataSize){

	int nHits = 0;
	int hits[nChannelsX][nChannelsY];


	int ir,ic; // for indexing row, column
	int dum;
	double tot; // total value of pixel

	//initialize to 0
	for (ir=0; ir<nChannelsX; ir++)
		for (ic=0; ic<nChannelsY;ic++)
			hits[ir][ic]=0;


	//for each pixel
	for (ir=0; ir<160; ir++){
		for (ic=0; ic<160;ic++){

			//validate mapping
			if ((map[ir][ic] < 0) || (map[ir][ic] >= (myDataSize))){
				cout << "Bad Channel Mapping index: " << map[ir][ic] << endl;
				continue;
			}


			//if frame within pedestal number
			if (myPhotonHit.iframe < nped)
				stat[ir][ic].Calc((double)(mask[ir][ic] ^ myData[map[ir][ic]]));

			// frame outside pedestal number
			else{

				//if hit wasnt registered
				if (hits[ir][ic] == 0){

					myPhotonHit.rms = stat[ir][ic].StandardDeviation();
					myPhotonHit.ped	= stat[ir][ic].Mean();

					dum = 1;
					tot = 0;

					//for 1d and 2d
					int c = 1;
					int d = 1;
					if (nChannelsX == 1)
						c = 0;

					//center pixel
					myPhotonHit.data[c][d] = ((double)(mask[ir][ic] ^ myData[map[ir][ic]])) - myPhotonHit.ped;

					//check if neighbours are involved
					for (int ih = -c; ih <= c ; ih++){
						for (int iv = -d; iv <= d; iv++){
							//validate neighbouring pixels (not really needed)
							if (((ir+ih) >= 0) && ((ir+ih) < nChannelsX) && ((ic+iv) >= 0) && ((ic+iv) < nChannelsY)) {
								//validate mapping
								if ((map[ir+ih][ic+iv] < 0) || (map[ir+ih][ic+iv] >= myDataSize)){
									cout << "Bad Channel Mapping index: " << map[ir][ic] << endl;
									continue;
								}

								myPhotonHit.data[iv+d][ih+c] = (double)(mask[ir][ic] ^ myData[map[ir+ih][ic+iv]])-stat[ir+ih][ic+iv].Mean();
								tot += myPhotonHit.data[iv+c][ih+d];
								if (myPhotonHit.data[iv+c][ih+d] > myPhotonHit.data[c][d])
									dum = 2;
							}
						}
					}


					if (tot < CLUSTER_SIZE * nsigma * myPhotonHit.rms)
						dum = 0;

					if (myPhotonHit.data[c][d] < nsigma * myPhotonHit.rms && dum != 0)
						dum = 3;

					if (myPhotonHit.data[c][d] > -nsigma * myPhotonHit.rms &&
							myPhotonHit.data[c][d] < nsigma * myPhotonHit.rms &&
							dum == 0){
						//Appriximated running average
						stat[ir][ic].Calc((double)(mask[ir][ic]^myData[map[ir][ic]]));
					}
					else if (dum == 1){
						myPhotonHit.x = ic;
						myPhotonHit.y = ir;
#ifdef MYROOT
						myTree->Fill();
#endif
						hits[ir][ic] = 1;
						nHits++;
					}

				}
			}
		}
	}

	if (myPhotonHit.iframe%1000 == 0)
		cout << "Frame: " << myPhotonHit.iframe << " Hits: " << nHits << endl;

	return nHits;
}


/*
//inside the receiver
//pop fifo

int16_t myData[160*160];
if (singlePhotonFilter::verifyFrame(wbuf,rc, myData,firstTimeHere))
if(firstTimeHere){
	firsttimeHere = 0;
	//f0 = fnum;
}
//singlePhotonFilter:SetFrameNumber(fnum - f0); singlePhotonFilter:SetFrameNumber(int i){myPhotonHit.iframe = i;};
	singlePhotonFilter.findhits(myData,1286*40);
*/


void singlePhotonFilter::initialize(int fmask, int pmask, int foffset, int poffset, int pperf){
	fnum = 0; pnum = 0; ptot = 0; f0 = 0;
	frame_index_mask = fmask;
	packet_index_mask = pmask;
	frame_index_offset = foffset;
	packet_index_offset = poffset;
	packets_per_frame = pperf;
}



int singlePhotonFilter::verifyFrame(char *inData, int inDataSize, int16_t* myData, int firstTime){
	int offset = 0;
	int fIndex = 0;
	int pIndex = 0;


	while(offset < inDataSize){
		fIndex = ((((int)(*((int*)inData))) & (frame_index_mask)) >> frame_index_offset);
		pIndex = ((((int)(*((int*)inData))) & (packet_index_mask)) >> packet_index_offset);

		offset += 4;

		//check validity of packet index
		if ((pIndex < 0) && (pIndex >= packets_per_frame)){
			cout << "cannot decode packet index:" << pIndex << endl;
			//its already dealt with cuz this frame will be discarded in the end
		}

		//put first packet last
		if (pIndex == 0)
			pIndex = packets_per_frame;


		//copy packet to correct spot in outData
		memcpy(((char*)(myData+(pIndex-1)*640)), (inData + offset), 1280);
		offset += 1280 + 2;

		//firsttime
		if (firstTime){
			firstTime = 0;
			f0 = fIndex;
			fnum = fIndex;
			pnum = pIndex - 1; //should b 0 at first
			ptot = 0;
		}

		//if it is not matching withthe frame number
		if (fIndex != fnum){
			cout << "Missing Packet! " << fnum << " "
					"Expected f" << fnum << " p " << pnum + 1 << " received f " << fnum << " p " << pIndex << endl;
			fnum = fIndex;
			pnum = pIndex - 1;
			ptot = 0;
		}

		//if missing a packet, discard
		if (pIndex != pnum + 1){
			cout << "Missing Packet! " << fnum << " "
					"Expected f" << fnum << " p " << pnum + 1 << " received f " << fnum << " p " << pIndex << endl;
			pnum = pIndex;
			ptot = 0;/** should it be 0 or just not incrememnted */
		}
		//no missing packet
		else{
			pnum++;
			ptot++;
		}


		//if its the last index
		if (pIndex == packets_per_frame){
			//got all packets
			if (ptot == packets_per_frame){
				myPhotonHit.iframe = fnum - f0;/** ?? */
				findHits(myData,nChannelsX*nChannelsY*2);
				return 1;
			}else{
				cout << "* Some packets have been missed!" << fnum << endl;
				ptot = 0;
				pnum = 0;
				fnum = fIndex + 1;
			}
		}

		//index not 40, but total  is 40.. strange
		else if (ptot == packets_per_frame){
			cout << "** Some packets have been missed! " << fnum << endl;
			ptot = 0;
			pnum = pIndex - 1;
			fnum = fIndex;
		}
	}

	delete myData;

	return 0;
}

/*
void singlePhotonFilter::makeRunTree(char *fformat, int runmin, int runmax, char *tname, int off) {

	char buff[1286];
	int *header=(int*)buff;
	char *footer=buff+1284;
	Short_t mydata[160*160];//=(Short_t*)(buff+4);
	int pnum;
	int fnum;
	int ifr;
	int ipa;
	Double_t dd[3][3];
	int ncol=colwidth*4;//=160;
	int nch=nrow*ncol;
	char fname[1000];
	Int_t chan[160][160];
	Int_t nHits=0;
	char vtype[100];
	int afifo_length;
	ifstream filebin;
	int ptot=0;


	for (int irun=runmin; irun<=runmax; irun++) {
		sprintf(fname,fformat,irun);
		cout << "file name " << fname << " " << tall->GetEntries() << endl;
		filebin.open((const char *)(fname), ios::in | ios::binary);
		if (filebin.is_open()) {
			while (readNextFrame(filebin, mydata, fnum)) {
				//to get rid of the first frame number
				if (iii==0) {
					f0=fnum;
					iii = 1;
				}
				iframe=fnum-f0;
				findHits(mydata, tall);
			}
			if (filebin.is_open())
				filebin.close();
		} else
			cout << "could not open file " << fname << endl;
	}
}




int readNextFrame(ifstream &filebin, int16_t *mydata, int &ifr){

	char buff[1286];
	int *header=(int*)buff;
	char *footer=buff+1284;
	//  Short_t mydata[160*160];//=(Short_t*)(buff+4);
	int pnum;
	int fnum;
	// int ifr;
	int ipa;
	int iii=0;
	int nrow=160;
	int colwidth=40;
	//  int nsigma=5;
	int ncol=colwidth*4;//=160;
	int nch=nrow*ncol;
	int f0;
	char fname[1000];
	int ptot=0;
	if (filebin.is_open()) {
		while (filebin.read(buff,sizeof(4))) {
			//  while (filebin.read(buff,sizeof(buff))) {
			pnum=(*header)&0xff;//packet num
			fnum=((*header)&(0xffffff00))>>8;//frame num
			if (pnum==0)
				pnum=40;
			filebin.read((char*)(mydata+(pnum-1)*640),1280);
			filebin.read(footer,2);
			if (iii==0) {
				//	  cout << "fnum " << fnum << " pnum " << pnum << coff << endl;
				ifr=fnum;
				ipa=pnum-1;
				f0=fnum;//not used
				iii=1;
				ptot=0;
			}
			if (fnum!=ifr) {
				cout << "Missing packet! "<< ifr << " ";
				cout << "Expected f " <<ifr << " p " << ipa+1 << " received f " << fnum << " p " << pnum << endl;
				ifr=fnum;
				ipa=pnum-1;
				ptot=0;
			}
			if (pnum!=ipa+1) {
				cout << "Missing packet! "<< ifr << " ";
				cout << "Expected f " <<ifr << " p " << ipa+1 << " received f " << fnum << " p " << pnum << endl;
				ipa=pnum;
				ptot=0;
			} else {
				ipa++;
				ptot++;
			}

			if (pnum==40) {
				if (ptot==40) {
					return 1;
				} else {
					cout << "* Some packets have been missed! " << ifr << endl;
					ptot=0;
					ipa=0;
					ifr=fnum+1;
					//   return 0;
				}
			}  else if (ptot==40) {
				cout << "** Some packets have been missed! " << ifr << endl;
				ptot=0;
				ipa=pnum-1;
				ifr=fnum;
			}
		}
	}

	return 0;
}



*/
/*
  int nch=nrow*ncol;

  int f0;

  char fname[1000];

  int ptot=0;

    if (filebin.is_open()) {

      while (filebin.read(buff,sizeof(4))) {
      //  while (filebin.read(buff,sizeof(buff))) {

	pnum=(*header)&0xff;
	fnum=((*header)&(0xffffff00))>>8;

	if (pnum==0)
	  pnum=40;


	filebin.read((char*)(mydata+(pnum-1)*640),1280);
	filebin.read(footer,2);

	if (iii==0) {
	  //	  cout << "fnum " << fnum << " pnum " << pnum << coff << endl;
	  ifr=fnum;
	  ipa=pnum-1;
	  f0=fnum;
	  iii=1;
	  ptot=0;

	}

	if (fnum!=ifr) {
	  cout << "Missing packet! "<< ifr << " ";
	  cout << "Expected f " <<ifr << " p " << ipa+1 << " received f " << fnum << " p " << pnum << endl;
	  ifr=fnum;
	  ipa=pnum-1;
	  ptot=0;
	}


	if (pnum!=ipa+1) {
	  cout << "Missing packet! "<< ifr << " ";
	  cout << "Expected f " <<ifr << " p " << ipa+1 << " received f " << fnum << " p " << pnum << endl;
	  ipa=pnum;
	  ptot=0;
	} else {
	  ipa++;
	  ptot++;
	}

	if (pnum==40) {
	  if (ptot==40) {
	    return 1;
	  } else {
	    cout << "* Some packets have been missed! " << ifr << endl;
	    ptot=0;
	    ipa=0;
	    ifr=fnum+1;
	    //   return 0;
	  }
	}  else if (ptot==40) {
	  cout << "** Some packets have been missed! " << ifr << endl;
	  ptot=0;
	  ipa=pnum-1;
	  ifr=fnum;
	}
      }
    }

    return 0;
}

*/

