#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

int cpid;

int sign1=0; 
void funcSIGUSR1(int sign){
    if (getpid() == cpid) sign1=1;
}

int sign2=0;
void funcSIGUSR2(int sign) {
    sign2=1;
}

int sign5=1; //για να τυπωθει και στα 0s
void funcSIGALRM(int sign){
    sign5=1;
}


int main(int argc, char **argv) {
    struct sigaction sig5,sig1,sig2;
    sig5.sa_flags = SA_RESTART; 
    sig1.sa_flags = SA_RESTART;  
    sig2.sa_flags = SA_RESTART;
    sig5.sa_handler =funcSIGALRM;
    sig1.sa_handler =funcSIGUSR1; 
    sig2.sa_handler =funcSIGUSR2; 
      
    sigaction(SIGALRM, &sig5, NULL);
    sigaction(SIGUSR1, &sig1, NULL); 
    sigaction(SIGUSR2, &sig2, NULL);
    
    cpid = getpid();
    char *gates;
    int k = strtol(argv[1], NULL, 10);
    time_t start, end;
    int time_spent;
    time(&start);
    char buffer[1];
    if(argv[2][0] == 't' || argv[2][0] == 'f') buffer[0] = argv[2][0];
    
    alarm(15);
    
    while (1) {
        if (sign5 == 1){
            time(&end);
            time_spent = difftime(end, start);
            if (buffer[0] == 't') gates = "open";
            else gates = "closed";
	    printf("[ID=%d/PID=%d/TIME=%ds] The gates are %s!\n", k, getpid(), time_spent, gates);
	    alarm(15); //reschedule, μολις περασουν 15" θα ληθφει σημα SIGALRM
	    sign5=0;
	}
	if (sign1 == 1) {
            time(&end);
            time_spent = difftime(end, start);
            if (buffer[0] == 't') gates = "open";
            else gates = "closed";
            printf("[ID=%d/PID=%d/TIME=%ds] The gates are %s!\n", k, getpid(), time_spent, gates);
	    sign1=0;
        }
	else if (sign2 == 1) {
            time(&end);
            time_spent = difftime(end, start);
            if (buffer[0]  == 't') {
                buffer[0]  = 'f';
                gates = "closed";
                    }
            else if (buffer[0]  == 'f') {
                buffer[0]  = 't';
                gates = "open";
            }
            printf("[ID=%d/PID=%d/TIME=%ds] The gates are %s!\n", k, getpid(), time_spent, gates);
	    sign2=0;
	}
    }
    exit(0);
}

		
