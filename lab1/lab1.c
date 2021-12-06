#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <fcntl.h>
#include<unistd.h>
#include<string.h>
//File Descriptor for stdin and stdrerr
#define FD_STDERR 2
//User read-write, group read, others read
#define PERMS 0644 //the owner of the file has read and write access, while the group members and other users on the system only have read access
//Colors
#define DEFAULT "\033[30;1m"
#define RED "\033[31;1m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
//stat filename chmod +rwx filename

//Συναρτησεις για εκτυπωση διαδικασιων

void print1(int fd_in, int k){ 
    printf(CYAN "[Child1] Started. PID= %d PPID = %d" WHITE "\n", getpid(), getppid());
    char buffer[50]; 
    int l=0;
    int p = getpid();
    snprintf(buffer, 15 + sizeof(p) , "message from %d\n", p); //μορφοποιεί και αποθηκεύει μια σειρά χαρακτήρων και τιμών στον πινακα buffer
    //snprintf(buffer, 6, "%d\n", p);
    while(l<k){
        time_t seconds; 
        seconds = time(NULL);
	    printf(CYAN "[Child1] Heartbeat PID= %d Time= %ld x = %d" WHITE "\n", getpid(), seconds, l);
        l=l+2;
        sleep(1);
        //write in file
        write(fd_in, buffer, 14 + sizeof(p));
	    //write(fd_in, "message from ", 13);
	    //write(fd_in, buffer, sizeof(p));
        //write(fd_in, "\n", 1);
    }
    printf(RED "[Parent] Waiting for child" WHITE "\n");
    printf(CYAN "[Child1] Terminating!" WHITE "\n");
    printf(RED "[Parent] Child with PID = %d terminated" WHITE "\n", getpid());
} 
void print2(int fd_in, int k){
    printf(YELLOW "[Child2] Started. PID= %d PPID = %d" WHITE "\n", getpid(), getppid());
    char buffer[50];
    int l=1;
    int p = getpid();
    snprintf(buffer, 15 + sizeof(p) , "message from %d\n", p);
    while(l<k){
    	time_t seconds; 
        seconds = time(NULL);
        printf(YELLOW "[Child2] Heartbeat PID= %d Time= %ld x = %d" WHITE "\n", getpid(), seconds, l);
        l=l+2;
        sleep(1);
        //write in file
        write(fd_in, buffer, 14 + sizeof(p));
    }
    printf(RED "[Parent] Waiting for child" WHITE "\n");
    printf(YELLOW "[Child2] Terminating!" WHITE "\n");
    printf(RED "[Parent] Child with PID = %d terminated" WHITE "\n", getpid());
}
void printP(int fd_in, int k){
    int l;
    char buffer[50]; 
    int p = getpid();
    snprintf(buffer, 15 + sizeof(p) , "message from %d\n", p);
    for (l=0; l<k/2; l++){
        time_t seconds; 
        seconds = time(NULL);
        printf(RED "[Parent] Heartbeat PID= %d Time= %ld" WHITE "\n", getpid(), seconds);
        sleep(1);
        //write in file
        write(fd_in, buffer, 14 + sizeof(p));
    }
}
  
int main(int argc, char **argv) {
    int status;
    pid_t child[1];	                //πινακας 2 θεσεων για τα παιδια
    int k = strtol(argv[2], NULL, 10); //convert char to int, decimal system: base is 10. The endpointer argument will be set to the "first invalid character", i.e. the first non-digit. 
                                       //If you don't care, set the argument to NULL instead of passing a pointer, as shown.   
    
    // Open file and store the file descriptor in the variable fd_in
    int BUFFER_SIZE = (k + k/2)*17;    //address of the area of memory that data is to be written out
    int n_read, n_write;
    char buffer[BUFFER_SIZE];  
    int fd_in = open(argv[1], O_RDWR | O_CREAT | O_TRUNC); //O_RDWR: opens file for read and write, O_CREAT: creates the file if it doesnt exist, �_TRUNC: truncates the size to zero
    if (fd_in == -1) {                            //file descriptor: είναι μια αφηρημένη ένδειξη που χρησιμοποιείται για πρόσβαση σε ένα αρχείο                                
    	perror("open");
        exit(-1);                                 //Delete Child1
	}
	
    child[0]=fork();                              //create Child1              
    if(child[0] < 0) perror("Child1");
    else if (child[0]==0) {                       //Child1 code            
	print1(fd_in, k);                            
	exit(0);                                  //delete process Child1
    }
    else {
        child[1]=fork();                          //create Child2  
        if(child[1] < 0) perror("Child2");
        else if (child[1]==0){                    //Child2 code 
            print2(fd_in, k); 
            exit(0);                              //delete process Child2
        } 
        else{                                     //Parent code        
            printP(fd_in, k);
            wait(&status);
            wait(&status);  
            printf(RED "[Parent] PID= %d Reading file:" WHITE "\n", getpid());
	    
            int f1 = lseek(fd_in, 0, SEEK_SET);   //go to the start of the file      
            if (f1 == -1) {
                perror("lseek");
                exit(-1);
            }
            do {		
                //Returns number of bytes read
                n_read = read(fd_in, buffer, BUFFER_SIZE);  //read at most BUFFER-SIZE bytes from the file pointed to by file descriptor fd, store in buffer
                if (n_read == -1) {
                    perror("read");      
                    exit(-1);
                }
                // We store the count of bytes in n_read variable and then use the variable in the write() to print exactly the same number of bytes on the screen.
                n_write = write(FD_STDERR, buffer, n_read); //write to display, FD_STDERR: file descriptor for standard error output(separate error messages from regular output)
                                                            //n_read is the amount of data to copy, n_write is the actual amount of data written 
                if (n_write < n_read) { //είναι μια αφηρημένη ένδειξη που χρησιμοποιείται για πρόσβαση σε ένα αρχείο 
                    perror("write");                        
                    exit(-1);           
                }
            } while (n_read>0); //while there still exists data to copy to the screen
			                    
            close(fd_in);      //All the files associated with the process are closed
            exit(0);
        }
    }
    return 0;
}
