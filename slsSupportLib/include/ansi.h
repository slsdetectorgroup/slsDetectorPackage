#define RED    	 	"\x1b[31m"
#define GREEN   	"\x1b[32m"
#define YELLOW  	"\x1b[33m"
#define BLUE    	"\x1b[34m"
#define MAGENTA 	"\x1b[35m"
#define CYAN    	"\x1b[36m"
#define GRAY		"\x1b[37m"
#define DARKGRAY	"\x1b[30m"

#define BG_BLACK	"\x1b[48;5;232m"
#define BG_RED     	"\x1b[41m"
#define BG_GREEN   	"\x1b[42m"
#define BG_YELLOW  	"\x1b[43m"
#define BG_BLUE    	"\x1b[44m"
#define BG_MAGENTA 	"\x1b[45m"
#define BG_CYAN    	"\x1b[46m"
#define RESET   	"\x1b[0m"
#define BOLD    	"\x1b[1m"

//on background black
#define bprintf(code, format, ...)	printf(code BG_BLACK format RESET, ##__VA_ARGS__)

//normal printout
#define cprintf(code, format, ...) printf(code format RESET, ##__VA_ARGS__)
/*

Code examples

example 1 (a snippet):


#ifdef MARTIN
        cprintf(BLUE, "LL Write - Len: %2d - If: %X - Data: ",buffer_len, ll->ll_fifo_base);
        for (i=0; i < buffer_len/4; i++)
                cprintf(BLUE, "%.8X ",*(((unsigned *) buffer)+i));
        printf("\n");
#endif

#ifdef MARTIN
        cprintf(CYAN, "LL Read - If: %X - Data: ",ll->ll_fifo_base);
#endif



example 2:

int main()
{
	int i=1;
	printf("Normal %i\n", i);
	cprintf(RED, "Red\n");
	cprintf(GREEN, "Green\n");
	cprintf(YELLOW, "Yellow\n");
	cprintf(BLUE, "Blue\n");
	cprintf(MAGENTA, "Mangenta %i\n", i);
	cprintf(CYAN, "Cyan %i\n", i);
	cprintf(BOLD, "White %i\n", i);
	cprintf(RED BOLD, "Red %i\n", i);
	cprintf(GREEN BOLD, "Green\n");
	cprintf(YELLOW BOLD, "Yellow\n");
	cprintf(BLUE BOLD, "Blue\n");
	cprintf(MAGENTA BOLD, "Mangenta %i\n", i);
	cprintf(CYAN BOLD, "Cyan %i\n", i);
}

*/
