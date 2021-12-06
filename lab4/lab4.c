#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //close
#include <fcntl.h>
#include <sys/types.h>  //socket
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>
#include <sys/select.h> //include the whole family
#include <sys/socket.h> //socket
#include <arpa/inet.h>
#include <netdb.h>      //gethostbyname
#include <time.h>
#include <errno.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

//check if input is integer
bool check_get(char str[]) {
   for (int i = 0; i < strlen(str); i++)
        if ((isdigit(str[i]) || str[i]=='\n' || str[i]==' ') == false)
            return false; //when one non numeric value is found, return false   
   return true;
}


int main(int argc, char** argv) {
    char *hostname;    //Το όνομα του συστήματος
    int port;
    bool debug = false;
    
    if (argc == 1) {
        hostname="lab4-server.dslab.os.grnetcloud.net"; 
        port = 18080;
    }
    
    else if (argc == 2) {
        if (!(strncmp(argv[1],"--debug",7))) {
            debug = true;
            hostname="lab4-server.dslab.os.grnetcloud.net"; 
            port = 18080;
        }   
        else {
            printf("Wrong input\n");
            exit(-1);
        } 
    }
    
    else if (argc == 3) {
        if(!(strncmp(argv[1],"--host",6))) {
            hostname = argv[2];
            port = 18080;
        }
        else if(!(strncmp(argv[1],"--port",6))) {
            port = atoi(argv[2]);
            hostname="lab4-server.dslab.os.grnetcloud.net"; 
        }
        else {
            printf("Wrong input\n");
            exit(-1);
        }
    }
    
    else if (argc == 4) {
        if(!(strncmp(argv[1],"--host",6))) {
            hostname = argv[2];
            port = 18080;
        }
        else if(!(strncmp(argv[1],"--port",6))) {
            port = atoi(argv[2]);
            hostname="lab4-server.dslab.os.grnetcloud.net"; 
        }
        else {
            printf("Wrong input\n");
            exit(-1);
        }
        if (!(strncmp(argv[3],"--debug",7))) {
            debug = true;
        } 
        else {
            printf("Wrong input\n");
            exit(-1);
        } 
    }
    
    else if ((argc == 5) || (argc == 6)) {
        if (!(strncmp(argv[1],"--host",6)) && !(strncmp(argv[3],"--port",6))) {
            hostname = argv[2];
            port = atoi(argv[4]);
        }   
        else {
            printf("2Wrong input\n");
            exit(-1);
        } 
        if ((argc == 6) && !(strncmp(argv[5],"--debug",7))) {
            debug = true;
        } 
        else if ((argc == 6) && (strncmp(argv[5],"--debug",7))){
            printf("3Wrong input\n");
            exit(-1);
        } 
    }
    
    else {
        printf("4Wrong input\n");
        exit(-1);
    }
   
    // Create socket
    int domain = AF_INET;   //Διευθυνσιοδότηση των sockets
    int type = SOCK_STREAM; //Μεταδοση δεδομενων πανω στο καναλι επικοινωνιας
    int protocol = 0;       //Αφηνουμε το συστημα να επιλεξει το πρωτοκολλο το οποιο θα υλοποιησει την  επικοινωνια πανω στο καναλι 

    int sock_fd;
    //Αν πανε ολα καλα δημιουργει ενα ακρο επικοινωνιας μεσω sockets                             
    if ((sock_fd = socket(domain,type,0)) < 0){
        printf("Cannot create socket\n"); 
        exit(-1); 
    }
    
    //DEFINE SOCKET-PORT BIND FOR CLIENT
    struct sockaddr_in sin;
    sin.sin_family = AF_INET; //Ιδιο με την παραμετρο domain κατα την δημιουργια του socket
    sin.sin_port = htons(0); // Let the system choose, σε ποιο port θα ακουει
    sin.sin_addr.s_addr = htonl(INADDR_ANY); //IP διεύθυνση που αντιστοιχεί στο παρόν σύστημα της μηχανής στην οποιά θα δέχεται συνδέσεις το socket  

    int binding; //Με την κλήση αυτή το socket αποκτά διεύθυνση

    if ((binding = bind(sock_fd, &sin, sizeof(sin))) < 0){ 
        printf("Cannot give address to socket\n");
        exit(-1);
    }      
    
	/*Ποιά είναι η διεύθυνση του συστήματος host*/ 
	//Από το ονομα του σερβερ θα βρει την ip διευθυνση, αυτους τους 4 αριθμους τους βαζει σε έναν ακεραιο, 4 bytes κολλημενα μεταξυ τους, και αυτόν τον αριθμο μετα τον κανει κοπι μεσα στο struct  στο οποιο θα συνδεθει 
	struct hostent *server_host; //Η διεύθυνση του συστήματος, gonna use this struct to get host details based on hostname
	if ((server_host = gethostbyname(hostname)) == NULL) {
		printf("Unknown host %s\n", hostname); 
        exit(-1); 
	}
	
	/* Set up struct sockaddr_in */ 
	struct sockaddr_in server; //Η διεύθυνση του socket
    server.sin_family = AF_INET; //internet  
    server.sin_port = htons((u_short) port); //port number
    bcopy(server_host->h_addr, &server.sin_addr, server_host->h_length); //βαζει τα 4 bytes με τον τροπο που πρεπει στην διευθυνση στην οποια εγω θα συνδεθω, does not have return value, κάνει copy bytes από μια θέση μνήμης σε μια άλλη

    //Timeout for connect
    struct timeval timeout;
    timeout.tv_sec  = 5;  // after 5 seconds connect() will timeout
    timeout.tv_usec = 0;
    //SOL_SOCKET: to set options at the socket level, SO_SNDTIMEO: option_name, specify the amount of time that an output function blocks
    int sso; ;
    if ((sso = setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))) < 0) {
        printf("Error: setsockopt\n"); 
        exit(-1);
    }
    
    //Σε αυτό το σημείο στον server υπάρχει η πλήρης διεύθυνση του ζητούμενου εξυπηρετητή, οπότε με χρήση της connect() ζητείται σύνδεση
    int connection; 
    
    /*Στο σημείο αυτό -αν βέβαια η connect() έχει επιτύχει το socket sd του πελάτη έχει συνδέθει με ένα αξιόπιστο καναλι επικοινωνιας με το socket που περιγραφει στο server*/ 
    if ((connection = connect(sock_fd, &server, sizeof(server))) < 0) {//error checking!
        printf("Cannot connect to server\n"); 
        exit(-1);
    }
    
    else {
        printf("Connecting!\n");
        printf("Connected to %s:%d\n", hostname, port);
    }
    
    //GET INPUT
    char buffer[501]; 
    char *a; 

    while (1) {
        fd_set fdset; 
        int nfds; 

        FD_ZERO(&fdset);                // we must initialize before each call to select
        FD_SET(STDIN_FILENO, &fdset);   // select will check for input from terminal
        FD_SET(sock_fd, &fdset);        // select will check for input from socket descriptor (Client)
        
        //select only considers file descriptors that are smaller than nfds
        nfds = MAX(STDIN_FILENO,sock_fd) + 1; //the highest-numbered file descriptor in any of the three sets, plus 1

        // wait until any of the input file descriptors are ready to receive
        int selectt; 
        //επιστρέφει 0 αν γινει timeout, -1 αν συμβει λάθος και μεγαλύτερο του 0 αν υπαρξει δυνατοτητα εισοδου/εξοδου
        if ((selectt = select(nfds, &fdset, NULL, NULL, NULL)) <= 0) {
            perror("select\n");
            continue;                                       //just try again
        }

        //User has typed something, we can read from stdin without blocking
        if (FD_ISSET(STDIN_FILENO, &fdset)) { //επιστρεφει 1 αν ο fd ειναι στην fdset, 
            int readd = read(STDIN_FILENO, buffer, 500); //error checking! 
            if (readd == -1) { // read() attempts to read up to count bytes from STDIN into the buffer starting at buffer.
                perror("read\n");
                exit(-1);
            }

            buffer[readd] = '\0'; //Για να σβήσει ολα τα επόμενα στοιχεια του πινακα απο το τελευταιο input
            
            // New-line is also read from the stream, discard it.
            if (readd > 0 && buffer[readd-1] == '\n') 
                buffer[readd-1] = '\0'; 
            
            // user typed 'exit...'
            if (readd >= 4 && strncmp(buffer, "exit", 4) == 0) { 
                int shut = shutdown(sock_fd,2);   //2 giati den thelw oute read oute write
                if (shut < 0) perror("shutdown"); //gia na apofugoyme sysswreusi dedomenwn ston buffer wspou na aporifthoun meta to kleisimo tis sindesis
                                    
                close(sock_fd);  //kleisimo kanaliou epikoinwnias
                exit(0);
            }
            
            // user typed 'get...'
            else if (readd >= 3 && strncmp(buffer, "get", 3) == 0) {
                int writee=write(sock_fd,buffer,3); //write() writes up to 3 bytes 

                if (writee<0) {                         //from the buffer to sd sock_fd
                    perror("write");
                    exit(-1);
               }
                if (debug) printf("[DEBUG] sent '%s'\n",buffer);
            }
            
            // user typed 'help...'
            else if (readd >= 4 && strncmp(buffer, "help", 4) == 0) { 
                printf("Available commands:\n");
                printf("* 'help'                    : Print this help message\n");
                printf("* 'exit'                    : Exit\n");
                printf("* 'get'                     : Retrieve sensor data\n");
                printf("* 'N name surname reason'   : Ask permission to go out\n");
            }
            
            else if (readd >=1) {
                int writee=write(sock_fd,buffer,readd); //writes up to readd bytes
                if (writee<0){
                    perror("write");
                    exit(-1);
                }
                if (debug) printf("[DEBUG] sent '%s'\n",buffer);
            }
        }

        if (FD_ISSET(sock_fd, &fdset)) { //test if a file descriptor is still present in a set
            char buffer2[500]; 
            int read2 = read(sock_fd,buffer2,500); // read() attempts to read up to 500 bytes from socket descr into the buffer2

            if (strncmp(buffer2,"try again",strlen("try again")-1 ) == 0 ) { 
                printf("%s",buffer2);
                memset(buffer2, 0, sizeof(buffer2)); //clear buffer2
            }
            
            else if (read2 >= 1 && check_get(buffer2)) {
                if (debug) printf("[DEBUG] read '%s'\n",buffer2);
            
                for(int i=0;i<20;++i) printf("-");
                printf("\n");
                char first [40];
                char temp [4];
                char light [40];
                char time [40];
            
                a=buffer2; //pointer to 1st char element of buffer2
                strncpy(first,a,1);
                if (atoi(first)==0) printf("boot (0)\n");

                if (atoi(first)==1) printf("setup(1)\n");

                if (atoi(first)==2) printf("interval (2)\n");

                if (atoi(first)==3) printf("button (3)\n");

                if (atoi(first)==4) printf("motion (4)\n");

            
                strncpy(light,a +2,3);
                printf("Light level is: %d\n",atoi(light) );

                strncpy(temp,a +6,4);
                printf("temperature is : %.2f\n",atof(temp)/100.0);

                strncpy(time,a+11,10);
                time_t rawtime1 = atoi(time); 
                struct tm *info;
          
                info = localtime( &rawtime1 );
                printf("Current local time and date: %s", asctime(info));
                memset(buffer2, 0, sizeof(buffer2)); 
            }
            
            else if (read2 >= 1 && strncmp(buffer2,"ACK",strlen("ACK")-1 ) == 0 ){
                if (debug) printf("[DEBUG] read '%s'\n",buffer2 );
                printf("Response:'%s'\n",buffer2);
                memset(buffer2, 0, sizeof(buffer2)); 
            }
            
            else if (read2 >=1){ //verification code (αφού δεν μπήκε στις άλλες else if)
                if (debug) printf("[DEBUG] read '%s'\n",buffer2);
                printf("Send verification code: %s\n",buffer2);
                memset(buffer2, 0, sizeof(buffer2)); 
            }
        } 
    } 
    close(sock_fd);  
    return 0;
}

