#! /bin/awk -f

# this is an awk script to start a run
# you first need to run inimodule.awk to initialize
# the pattern, set Vc and set the trimbits
#

#####################################################################
# revision history                                                  #
#####################################################################
# 31.10.2001 first version                                          #
#####################################################################
#                                                                   #
# Bernd Schmitt                                                     #
#                                                                   #
# bernd.schmitt@psi.ch                                              #
#                                                                   #
#####################################################################
#                                                                   #
# modifications:                                                    #
#                                                                   #
# 1.3.2002 BS adapted for use with DCB                              #
#                                                                   #
# 25.5.2002 BS adapted to new convert program                       #
#                                                                   #
# 29.5.2002 sleep -> usleep for meas. time , TS                     #
#                                                                   #
#####################################################################

BEGIN {

# initialize variables
   NPAR=3

	PAR[1]="nrun"
	PAR[2]="fn"
	PAR[3]="par"

# initialize default values
	


	PARVAL[1] = 100
	PARVAL[2] = "microstrip_july2007"
        PARVAL[3]=0



	

	    printf("\n\nnumber of command line arguments: %i (incl. command)\n\n", ARGC);

# read command line defined variables
	if (ARGC>1) {
	    printf("\n\nnumber of command line arguments: %i (incl. command)\n\n", ARGC);
	    for (i=1; i<=ARGC; i++) {
	      printf("%s \n", ARGV[i]);
		nsplit=split(ARGV[i],array,"=")
		VAR = array[1];
		VAL = array[2];

		for (j=1; j<=NPAR; j++) {
		    if ( VAR==PAR[j] ) {
			PARVAL[j] = VAL
		    }
		}
	    }
	}
	run=PARVAL[1]
        fn=PARVAL[2]
	par=PARVAL[3]


# print command line arguments
	for (i=1; i<=NPAR; i++){
	    printf("\t... %2i.\t%7s = %s\n", i, PAR[i], PARVAL[i] );
        }
	printf("\n\n");

# generate parameter file

	fnamep=fn".parab"
	  printf("header before\n")>> fnamep
	system("date >>"fnamep)
	printf("run=%i \n", run ) >> fnamep

#print detector parameters to file
	if (par==1) {
	
	command="sls_detector_get exptime| awk -F \" \" '{print $2}'"
	command | getline var
	printf("acquisition time = %11.6f second(s)\n", var) >> fnamep

	command="sls_detector_get settings| awk -F \" \" '{print $2}'"
	command | getline var
	printf("settings = %s\n", var) >> fnamep;

	command="sls_detector_get threshold| awk -F \" \" '{print $2}'"
	command | getline var
	printf("threshold energy = %d eV\n", var) >> fnamep;

	command="sls_detector_get badchannels| awk -F \" \" '{print $2}'"
	command | getline var
	printf("bad channel list = %s\n",var) >> fnamep;


	command="sls_detector_get angconv| awk -F \" \" '{print $2}'"
	command | getline var
	printf("angle calibration conversion = %s\n",var) >> fnamep;


	command="sls_detector_get globaloff| awk -F \" \" '{print $2}'"
	command | getline var
	printf("beamline offset = %f deg\n", var) >> fnamep;

	command="sls_detector_get fineoff| awk -F \" \" '{print $2}'"
	command | getline var
	printf("fine offset = %f deg\n", var) >> fnamep;

	command="sls_detector_get flatfield| awk -F \" \" '{print $2}'"
	command | getline var
        printf("Flat field corrections = %s\n",var) >> fnamep;

	command="sls_detector_get ratecorr| awk -F \" \" '{print $2}'"
	command | getline var
	printf("Dead time corrections tau = %d ns\n",var) >> fnamep;

	}
	
	
#print beamline parameters to file

#read detector position
	system("caget X04SA-ES2-TH2:RO.RBV >>"fnamep)
#read I0
	system("caget X04SA-ES2-SC:CH6>>"fnamep)
}





