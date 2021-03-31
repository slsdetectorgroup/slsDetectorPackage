
#include "clogger.h"
#include "common.h"
#include "mythen3.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <string.h>

/*
// Common C/C++ structure to handle pattern data
typedef struct __attribute__((packed)) {
    uint64_t word[MAX_PATTERN_LENGTH];
    uint64_t ioctrl;
    uint32_t limits[2];
    // loop0 start, loop0 stop .. loop2 start, loop2 stop
    uint32_t loop[6];
    uint32_t nloop[3];
    uint32_t wait[3];
    uint64_t waittime[3];
} patternParameters;
*/

int chipStatusRegister=0;

int setBit(int ibit, int patword) { return patword |= (1 << ibit); }

int clearBit(int ibit, int patword) { return patword &= ~(1 << ibit); }

extern enum TLogLevel trimmingPrint ;


int getChipStatusRegister(){
  return chipStatusRegister;
}

patternParameters *setChipStatusRegister(int csr) {
  int iaddr=0;
  int  nbits=18;
  int error=0;
  //int start=0, stop=MAX_PATTERN_LENGTH, loop=0;
  int patword=0;
  
  patternParameters *pat = malloc(sizeof(patternParameters));
  memset(pat, 0, sizeof(patternParameters));
  
  patword=setBit(SIGNAL_STATLOAD,patword);
  for (int i=0; i<2; i++)
    pat->word[iaddr++]=patword;
  patword=setBit(SIGNAL_resStorage,patword);
  patword=setBit(SIGNAL_resCounter,patword);
  for (int i=0; i<8; i++)
    pat->word[iaddr++]=patword;
  patword=clearBit(SIGNAL_resStorage,patword);
  patword=clearBit(SIGNAL_resCounter,patword);
  for (int i=0; i<8; i++)
    pat->word[iaddr++]=patword;
  //#This version of the serializer pushes in the MSB first (compatible with the CSR bit numbering)
  for (int ib=nbits-1; ib>=0; ib--) {
    if (csr&(1<<ib))
      patword=setBit(SIGNAL_serialIN,patword);
    else
      patword=clearBit(SIGNAL_serialIN,patword);
    for (int i=0; i<4; i++)
      pat->word[iaddr++]=patword;
    patword=setBit(SIGNAL_CHSclk,patword);
    pat->word[iaddr++]=patword;
    patword=clearBit(SIGNAL_CHSclk,patword);
    pat->word[iaddr++]=patword;
  }

  patword=clearBit(SIGNAL_serialIN,patword);
  for (int i=0; i<2; i++)
    pat->word[iaddr++]=patword;
  patword=setBit(SIGNAL_STO,patword);
  for (int i=0; i<5; i++)
    pat->word[iaddr++]=patword;
  patword=clearBit(SIGNAL_STO,patword);
  for (int i=0; i<5; i++)
    pat->word[iaddr++]=patword;
  patword=clearBit(SIGNAL_STATLOAD,patword);
  for (int i=0; i<5; i++)
    pat->word[iaddr++]=patword;

  if (iaddr >= MAX_PATTERN_LENGTH) {
    LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n",
		   iaddr, MAX_PATTERN_LENGTH));
    error = 1;
  }
  // set pattern wait address
  for (int i = 0; i <= 2; i++)
    pat->wait[i]=MAX_PATTERN_LENGTH - 1;
  // pattern loop
  for (int i = 0; i <= 2; i++) {
    //int stop = MAX_PATTERN_LENGTH - 1, nloop = 0;
    pat->loop[i * 2 + 0]=MAX_PATTERN_LENGTH - 1;
    pat->loop[i * 2 + 1]=MAX_PATTERN_LENGTH - 1;
    pat->nloop[i]=0;
  }
  
  // pattern limits
  {
    pat->limits[0]=0;
    pat->limits[1]=iaddr;
  }
  
  if (error != 0) {
    free(pat);
    return NULL;
  }
  chipStatusRegister=csr;
  return pat;

}

patternParameters *setInterpolation(int mask) {
  int csr;
  if (mask)
    csr=chipStatusRegister|(1<< CSR_interp);
  else
    csr=chipStatusRegister & ~(1<< CSR_interp);

  return setChipStatusRegister(csr);
}



patternParameters *setPumpProbe(int mask) {
  int csr;
  if (mask)
    csr=chipStatusRegister|(1<< CSR_pumprobe);
  else
    csr=chipStatusRegister & ~(1<< CSR_pumprobe);

  return setChipStatusRegister(csr);

}
patternParameters *setDigitalPulsing(int mask) {

  int csr;
  if (mask)
    csr=chipStatusRegister|(1<< CSR_dpulse);
  else
    csr=chipStatusRegister & ~(1<< CSR_dpulse);

  return setChipStatusRegister(csr);
  
}
patternParameters *setAnalogPulsing(int mask){

  int csr;
  if (mask)
    csr=chipStatusRegister|(1<< CSR_apulse);
  else
    csr=chipStatusRegister & ~(1<< CSR_apulse);

  return setChipStatusRegister(csr);
  
}
patternParameters *setNegativePolarity(int mask){

  int csr;
  if (mask)
    csr=chipStatusRegister|(1<< CSR_invpol);
  else
    csr=chipStatusRegister & ~(1<< CSR_invpol);

  return setChipStatusRegister(csr);
  
}

int setGainPreamp(int pgain, int *csr) {

  
  
  switch (pgain) {
  case Cp_0:
  case Cp_15:
    *csr=(*csr)&~(1<<CSR_C10pre);
    break;
  case Cp_10:
  case Cp_45:
    *csr=(*csr)|(1<<CSR_C10pre);
    break;
  default:
    LOG(logERROR, ("Bad preamp gain"));
    return 1;
  }
  
  switch (pgain) {
  case Cp_0:
  case Cp_10:
    *csr=(*csr)&~(1<<CSR_C15pre);
    break;
  case Cp_15:
  case Cp_45:
    *csr=(*csr)|(1<<CSR_C15pre);
    break;
  default:
    LOG(logERROR, ("Bad preamp gain"));
    return 1;
  }
  
  return 0;

}
int setGainShaper(int shgain, int *csr){
  
  
  switch (shgain) {
  case Csh_0:
  case Csh_30:
  case Csh_50:
  case Csh_80:
    *csr=(*csr)&~(1<<CSR_C15sh);
    break;
  case Csh_15:
  case Csh_45:
  case Csh_65:
  case Csh_95:
    *csr=(*csr)|(1<<CSR_C15sh);
    break;
  default:
    LOG(logERROR, ("Bad shaper gain"));
    return 1;
  }
  
  switch (shgain) {
  case Csh_0:
  case Csh_15:
  case Csh_50:
  case Csh_65:
    *csr=(*csr)&~(1<<CSR_C30sh);
    break;
  case Csh_30:
  case Csh_45:
  case Csh_80:
  case Csh_95:
    *csr=(*csr)|(1<<CSR_C30sh);
    break;
  default:
    LOG(logERROR, ("Bad shaper gain"));
    return 1;
  }
  switch (shgain) {
  case Csh_0:
  case Csh_15:
  case Csh_30:
  case Csh_45:
    *csr=(*csr)&~(1<<CSR_C50sh);
    break;
  case Csh_80:
  case Csh_50:
  case Csh_65:
  case Csh_95:
    *csr=(*csr)|(1<<CSR_C50sh);
    break;
  default:
    LOG(logERROR, ("Bad shaper gain"));
    return 1;
  }
  
  return 0;
}

int setGainAC(int acgain, int *csr){

  switch (acgain) {
  case Cac_450:
    *csr=(*csr)&~(1<<CSR_C225ACsh);
    break;
  case Cac225:
    *csr=(*csr)|(1<<CSR_C225ACsh);
    break;
  default:
    LOG(logERROR, ("Bad ac gain"));
    return 1;
  }
  return 0;
}



patternParameters *setChipGain(int pgain, int shgain, int acgain){
  int csr=chipStatusRegister;
  
  if (setGainPreamp(pgain,&csr))
    return NULL;
  if (setGainShaper(shgain,&csr))
    return NULL;
  if (setGainAC(acgain,&csr))
    return NULL;
  
  return setChipStatusRegister(csr);
}




patternParameters *setChannelRegisterChip(int ichip, int *mask, int *trimbits) {

  patternParameters *pat = malloc(sizeof(patternParameters));
  memset(pat, 0, sizeof(patternParameters));
  

    // validate
  for (int ichan = ichip * NCHAN_1_COUNTER * NCOUNTERS; ichan < ichip * NCHAN_1_COUNTER * NCOUNTERS+NCHAN_1_COUNTER*NCOUNTERS; ichan++) {
    if (trimbits[ichan]<0)  {
      LOG(logERROR, ("Trimbit value (%d) for channel %d is invalid - setting it to 0\n",
		     trimbits[ichan], ichan));
      trimbits[ichan]=0;
    }
    if (trimbits[ichan] > 63) {
      LOG(logERROR, ("Trimbit value (%d) for channel %d is invalid - settings it to 63\n",
		     trimbits[ichan], ichan));
      trimbits[ichan]=63;
    }
  }
  LOG(logINFO, ("Trimbits validated\n"));
  trimmingPrint = logDEBUG5;

    
  // trimming
  int error = 0;
  uint64_t patword = 0;
  int iaddr = 0;
  
  LOG(logDEBUG1, (" Chip %d\n", ichip));
  iaddr = 0;
  patword = 0;
  pat->word[iaddr++]=patword;
  
  // chip select
  patword = setBit(SIGNAL_TBLoad_1 + ichip, patword);
  pat->word[iaddr++]=patword;
  
  // reset trimbits
  patword = setBit(SIGNAL_resStorage, patword);
  patword = setBit(SIGNAL_resCounter, patword);
  pat->word[iaddr++]=patword;
  pat->word[iaddr++]=patword;
  patword = clearBit(SIGNAL_resStorage, patword);
  patword = clearBit(SIGNAL_resCounter, patword);
  pat->word[iaddr++]=patword;
  pat->word[iaddr++]=patword;
  
  // select first channel
  patword = setBit(SIGNAL_CHSserialIN, patword);
  pat->word[iaddr++]=patword;
  // 1 clk pulse
  patword = setBit(SIGNAL_CHSclk, patword);
  pat->word[iaddr++]=patword;
  patword = clearBit(SIGNAL_CHSclk, patword);
  // clear 1st channel
  pat->word[iaddr++]=patword;
  patword = clearBit(SIGNAL_CHSserialIN, patword);
  // 2 clk pulses
  for (int i = 0; i < 2; i++) {
    patword = setBit(SIGNAL_CHSclk, patword);
    pat->word[iaddr++]=patword;
    patword = clearBit(SIGNAL_CHSclk, patword);
    pat->word[iaddr++]=patword;
  }
  
  // for each channel (all chips)
  for (int ich = 0; ich < NCHAN_1_COUNTER; ich++) {
    LOG(logDEBUG1, (" Chip %d, Channel %d\n", ichip, ich));
    int val = trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS +
		       NCOUNTERS * ich] +
      trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS +
	       NCOUNTERS * ich + 1] *
      64 +
      trimbits[ichip * NCHAN_1_COUNTER * NCOUNTERS +
	       NCOUNTERS * ich + 2] *
      64 * 64;
    
    // push 6 0 bits
    for (int i = 0; i < 3; i++) {
      patword = clearBit(SIGNAL_serialIN, patword);
      patword = clearBit(SIGNAL_clk, patword);
      pat->word[iaddr++]=patword;
      patword = setBit(SIGNAL_clk, patword);
      pat->word[iaddr++]=patword;
    }
    
    for (int i = 0; i < 3; i++) {
      if (mask[i])
	patword = setBit(SIGNAL_serialIN, patword);
      else
	patword = clearBit(SIGNAL_serialIN, patword);
      patword = clearBit(SIGNAL_clk, patword);
      pat->word[iaddr++]=patword;
      patword = setBit(SIGNAL_clk, patword);
      pat->word[iaddr++]=patword;
    }
    
    // deserialize
    for (int i = 0; i < 18; i++) {
      if (val & (1 << i)) {
	patword = setBit(SIGNAL_serialIN, patword);
      } else {
	patword = clearBit(SIGNAL_serialIN, patword);
      }
      patword = clearBit(SIGNAL_clk, patword);
      pat->word[iaddr++]=patword;
      
      patword = setBit(SIGNAL_clk, patword);
      pat->word[iaddr++]=patword;
    }
    pat->word[iaddr++]=patword;
    pat->word[iaddr++]=patword;
    
    // move to next channel
    for (int i = 0; i < 3; i++) {
      patword = setBit(SIGNAL_CHSclk, patword);
      pat->word[iaddr++]=patword;
      patword = clearBit(SIGNAL_CHSclk, patword);
      pat->word[iaddr++]=patword;
    }
  }
  // chip unselect
  patword = clearBit(SIGNAL_TBLoad_1 + ichip, patword);
  pat->word[iaddr++]=patword;
  
  // last iaddr check
  if (iaddr >= MAX_PATTERN_LENGTH) {
    LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n",
		   iaddr, MAX_PATTERN_LENGTH));
    error = 1;
  }
  
  if (iaddr >= MAX_PATTERN_LENGTH) {
    LOG(logERROR, ("Addr 0x%x is past max_address_length 0x%x!\n",
		   iaddr, MAX_PATTERN_LENGTH));
    error = 1;
  }
  // set pattern wait address
  for (int i = 0; i <= 2; i++)
    pat->wait[i]=MAX_PATTERN_LENGTH - 1;
  // pattern loop
  for (int i = 0; i <= 2; i++) {
    //int stop = MAX_PATTERN_LENGTH - 1, nloop = 0;
    pat->loop[i * 2 + 0]=MAX_PATTERN_LENGTH - 1;
    pat->loop[i * 2 + 1]=MAX_PATTERN_LENGTH - 1;
    pat->nloop[i]=0;
  }
  
  // pattern limits
  {
    pat->limits[0]=0;
    pat->limits[1]=iaddr;
  }
  
  trimmingPrint = logINFO;
  if (error == 0) {
    
    LOG(logINFO, ("All trimbits have been loaded\n"));
  } else {
    free(pat);
    return NULL;
  }
  return pat;
}
