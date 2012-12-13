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
        NPAR=7

	PAR[1]="nrun"
	PAR[2]="fn"
	PAR[3]="par"
	PAR[4]="sv0"
	PAR[5]="sv1"
	PAR[6]="p0"
	PAR[7]="p1"

# initialize default values
	PARVAL[1] = 100
	PARVAL[2] = "myfname"
	PARVAL[3] = 1
	PARVAL[4] = 0
	PARVAL[5] = 0
	PARVAL[6] = "none"
	PARVAL[7] = "none"



# read command line defined variables
	if (ARGC>1) {
	    printf("\n\nnumber of command line arguments: %i (incl. command)\n\n", ARGC);
	    for (i=1; i<=ARGC; i++) {

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
	"par"PAR[3]=
	sv0=PAR[4]
	sv1=PAR[5]
	p0=PAR[6]
	p1=PAR[7]



# print command line arguments
	for (i=1; i<=NPAR; i++){
	    printf("\t... %2i.\t%7s = %s\n", i, PAR[i], PARVAL[i] );
        }
#	printf("\n\n");
 #       system("close_shutter_g95")

# execute your actions hereafter
	if (par==1) {
	  #open shutter
	} else {
	  #close shutter
	}

}





