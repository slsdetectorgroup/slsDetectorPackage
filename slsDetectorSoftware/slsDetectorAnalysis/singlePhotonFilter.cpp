  /********************************************//**
 * @file singlePhotonFilter.cpp
 * @short single photon filter using trees
 ***********************************************/
#include "singlePhotonFilter.h"



singlePhotonFilter::singlePhotonFilter(int nx, int ny,
		int fmask, int pmask, int foffset, int poffset, int pperf, int iValue,
		vector <vector<int16_t> > m, vector <vector<int16_t> > s, int d):
#ifdef MYROOT
										myTree(NULL),
										myFile(NULL),
#else
										myFile(NULL),
#endif
										nChannelsX(nx),
										nChannelsY(ny),
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
										frame_index_mask(fmask),
										packet_index_mask(pmask),
										frame_index_offset(foffset),
										packet_index_offset(poffset),
										packets_per_frame(pperf),
										incrementValue(iValue),
										enable(false),
										firstTime(true),
										ret(0),
										pIndex(0),
										fIndex(0){


	if (nChannelsX)
		nClusterX = 1;

	stat.resize(nChannelsX);
	for(int i=0; i<nChannelsX; i++)
		stat[i].resize(nChannelsY);

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





void singlePhotonFilter::initTree(char *outfname){

#ifdef MYROOT
	outfname = string(outfname).replace(".raw",".root");
	//fName.replace(".raw",".png");
	//sprintf(outfname, "%s/%s.root", outdir, fname);

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
	;/*myFile = fopen(outfname, "w");*/
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
	;
	/*if(myFile){ //&& (number of structs?)
		fwrite((void*)(&myPhotonHit), 1, sizeof(myPhotonHit), myFile);
		fclose(myFile);
		delete myFile;
		return OK;
	}*/
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





void singlePhotonFilter::setupAcquisitionParameters(){
	fnum = 0; pnum = 0; ptot = 0; f0 = 0; firstTime = true;

}



int singlePhotonFilter::verifyFrame(char *inData){
	ret = 0;
	pIndex = 0;
	fIndex = 0;
	fIndex = (((uint32_t)(*((uint32_t*)inData)))& frame_index_mask) >> frame_index_offset;
	pIndex = (((uint32_t)(*((uint32_t*)inData)))& packet_index_mask) >> packet_index_offset;

	//check validity of packet index
	if ((pIndex < 0) && (pIndex >= packets_per_frame)){
		cout << "cannot decode packet index:" << pIndex << endl;
		//its already dealt with cuz this frame will be discarded in the end
	}
	pIndex += incrementValue;

	//for moench, put first packet last
	if (pIndex == 0)
		pIndex = packets_per_frame;
#ifdef VERYVERBOSE
	cout<<"fi:"<<hex<<fIndex<< " pi:"<< pIndex << endl;
#endif
	//firsttime
	if (firstTime){
		firstTime = false;
		f0 = fIndex;
		fnum = fIndex;
		pnum = pIndex - 1; //should b 0 at first
		ptot = 0;
	}

	//if it is not matching withthe frame number
	if (fIndex != fnum){
		/*cout << "**Frame number doesnt match:Missing Packet! " << fnum << " "
				"Expected f " << fnum << " p " << pnum + 1 << " received f " << fIndex << " p " << pIndex << endl;*/
		fnum = fIndex;
		pnum = pIndex;
		ptot = 1;
		ret = -2; //dont return here.. if is the end of packets, for gotthard uve toreturn -1
	}

	//if missing a packet, discard
	else if (pIndex != pnum + 1){/**else */
	/*	cout << "**packet number doesnt match:Missing Packet! " << fnum << " "
				"Expected f" << fnum << " p " << pnum + 1 << " received f " << fnum << " p " << pIndex << endl;*/
		pnum = pIndex;
		ptot++;
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
			/*myPhotonHit.iframe = fnum - f0;//??
			if (enable)
				;*//*findHits(myData,inDataSize * ptot);*/
			fnum = fIndex + 1;
			ptot = 0;
			pnum = 0;
			ret = 1;
		}else{
		/*	cout << "* Some packets have been missed!*************************************" << fnum << " " << pnum<<" " << ptot<<endl;*/
			ptot = 0;
			pnum = 0;
			fnum = fIndex + 1;
			ret = -1;
		}
	}

	//index not 40, but total  is 40.. strange
	else if (ptot == packets_per_frame){
		cout << "***** Some packets have been missed! Shouldnt be here " << fnum << " " << pnum<< endl;
		ptot = 0;
		pnum = pIndex - 1;
		fnum = fIndex;
		ret = -1;
	}

	return ret;
}




