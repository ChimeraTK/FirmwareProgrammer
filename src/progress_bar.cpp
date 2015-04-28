#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>
#include <term.h>

int last_value = 0;

int my_putchar(int c)
{
    return putchar(c);
}

void ProgressBar(double TotalToDownload, double NowDownloaded)
{
    int total_barlength;
    int col;
    struct winsize size;
    
    setupterm(NULL,fileno(stdout),(int*)0);

    ioctl( 0, TIOCGWINSZ, (char *) &size );			//get terminal size
    col = size.ws_col;
    total_barlength = col - 10;

    int progress = lround((NowDownloaded / TotalToDownload) * 100);
    int barlength = lround(((float)progress / 100) * total_barlength);
    int ii=0;

    if(progress != last_value)
    {
	tputs(clr_eol, 1, my_putchar);			//clear line
	printf("%4d%% [", progress);
	for ( ; ii < barlength;ii++) {
	    printf("=");
	}
	for ( ; ii < total_barlength;ii++) {
	    printf(" ");
	}
	printf("]\r");
	fflush(stdout);
	
	last_value = progress;
    }
}
