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
        NPAR=2

	PAR[1]="nrun"
	PAR[2]="par"


# initialize default values
	PARVAL[1] = 100
	PARVAL[2] = "none"



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
        par=PARVAL[2]
	



# print command line arguments
	for (i=1; i<=NPAR; i++){
	    printf("\t... %2i.\t%7s = %s\n", i, PAR[i], PARVAL[i] );
        }
#execute action hereafter

}





