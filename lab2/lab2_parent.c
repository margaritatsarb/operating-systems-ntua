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

int fpid; //Στη main αναθετουμε το pid του parent

int sign1=0; 
void funcSIGUSR1(int sign){
    if (getpid()==fpid) sign1=2;
}

int sign3=0;
void funcSIGTERM(int sign){
    if (getpid()==fpid) sign3=2;
}

int sign4=0;
void funcSIGCHLD(int sign){
    sign4=1;
}

void checks(int a){
    if (a == 0) printf("Arguments should be 2\n");
    else if(a == 1) printf("Árguments must be t or f\n");
    exit(EXIT_FAILURE);
}

int describe_wait_status(pid_t pid, int status){
    int w;
    if (pid == 0) w=0;
    if (WIFSTOPPED(status)) w=1;        //child stopped
    else if (WIFSIGNALED(status)) w=2;  //child terminated by signal 
    return w;
}

int findID(int a, int *b, int m){
    for (int q=0; q<m; q++){
        if (b[q] == a) return q;
    } 
}

int main(int argc, char **argv){
    struct sigaction sig1,sig3,sig4; 
    sig1.sa_flags = SA_RESTART; //Μολις λαβεις συγκεκριμενες κλησεις συστηματος κανε αυτοματα restart 
    sig3.sa_flags = SA_RESTART; //Ξεμπλοκαρει απο wait αν λαβω σημα
    sig4.sa_flags = SA_RESTART;
    sig1.sa_handler =funcSIGUSR1; //Η διευθυνση στην οποια ξεκιναει η funcSIGUSR1
    sig3.sa_handler =funcSIGTERM;
    sig4.sa_handler =funcSIGCHLD;
    
    sigaction(SIGUSR1, &sig1, NULL); 
    sigaction(SIGTERM, &sig3, NULL);
    sigaction(SIGCHLD, &sig4, NULL);
    
    if (argc =! 2) checks(0);
    
    int N = strlen(argv[1]);
    int statusX;
    pid_t pid[N];   //πινακας N θεσεων για τα παιδια
    int child[N];   //pids of children 
    int ret1[N];    //value for waitpid
    int status[N];  //statuses of childre, use in wait
    int check1[N];
    
    fpid=getpid();
    char gate[N];
    for(int j=0; j<N; j++){
        if(argv[1][j] == 't' || argv[1][j] == 'f') gate[j]= argv[1][j];
        else {
            printf("Expected t or f as arguments\n"); 
            exit(-1);
        }
    }
    
    //child code	
    for (int i=0; i<N; i++) {
	pid[i] = fork();
        if (pid[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0){ 
            printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getppid(), i, getpid(), argv[1][i]);
	    char ids[N], states[N];
	    sprintf(ids, "%d", i);
	    sprintf(states, "%c", argv[1][i]);
	    char *const argv[] = {"./child", ids, states, NULL};
            int status = execv(argv[0], argv); //κληση συστηματος μετα απο fork για να αντικαταστησει τον χωρο μνημης μιας διεργασιας με ενα νεο κωδικα προγραμματος 
            if (status < 0) { //τρεχει μονο αν αποτυχει η execv
	        perror("Error running executable");
	        exit(-1);
	    }
    	} 
	else child[i]=pid[i];
        
    }
    
    //ελεγχος sign πατερα
    int q=0;
    while(q==0) {
    	if (sign1 == 2) {
            for(int o=0; o<N; o++) kill(child[o],SIGUSR1);
            sign1=0;
	}
	else if (sign3 == 2) {
            for(int k=0; k<N; k++) kill(child[k],SIGTERM);
            q=1;
		}
        else if (sign4 == 1 && sign3 != 2){ //child stopped or terminated by signal but not SIGTERM for all
            pid_t pid = wait(&statusX);   
            if (pid<0) {
        	perror("waitpid");
    	        exit(EXIT_FAILURE);
            }
	    int d = describe_wait_status(pid, statusX);
	    if (d==1) { //αν σταματησε
                kill(pid, SIGCONT);
                sign4=0;
	    } 
            else if (d==2) { //αν τερματισε
                int r = findID(pid,child,N);
                //int r=0;
                printf("[PARENT/PID=%d] Child %d with PID=%d exited\n", getpid(), r, pid);
                sign4 = 0;
 	        pid_t pid1 = fork();
	        if (pid1 == -1) perror("fork");
	            else if (pid1 == 0) {
                        printf("[PARENT/PID=%d] Created new child for gate %d (PID=%d) and initial state '%c'\n)", getppid(), r, getpid(), argv[1][r]);
	            	 char ids[N], states[N];
	            	 sprintf(ids, "%d", r);
	            	 sprintf(states, "%c", argv[1][r]);
                	 char *const argv[] = {"./child", ids, states, NULL};
	             	 execv(argv[0], argv);
                    }
	            else child[r] = pid1;
            }    
        }
    }
    for (int g=0; g<N; g++) {
        ret1[g]=wait(&status[g]); 
        if (ret1[g] == -1) {
            perror("wait");
	    exit(EXIT_FAILURE);
	}
        int j = N-g-1;
        printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), j); 
        printf("[PARENT/PID=%d] Child with PID=%d terminated!\n", getpid(), ret1[g]);
    }	
    printf("[PARENT/PID=%d] All children exited, terminating as well\n", getpid()); 
    return 0;
}
    
