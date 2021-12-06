#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void check_error(int eread, int ewrite, int eclose, int ewait){
    if(eread < 0) {
        printf("Error: read\n");
        exit(-1);
    }
    if(ewrite < 0) {
        printf("Error: write\n");
        exit(-1);
    }
    if(eclose < 0) {
        printf("Error: close\n");
        exit(-1);
    }
    if(ewait < 0) {
        printf("Error: close\n");
        exit(-1);
    }
}

int main(int argc, char **argv){
	if(argc != 3) {
	    printf("Wrong input\n");
	    exit(-1);
	}
	
	int n=atoi(argv[1]);   //Converts the string argument str to an integer (type int)
	int k=atoi(argv[2]);  
	if(n == 0 || k == 0) { //If no valid conversion could be performed, atoi returns zero
	    printf("Input should be 2 numbers > 0\n"); 
	    exit(-1);
	}
	
	int pd[n][2]; //Xρησιμοποιουνται για την επιστροφη 2 περιγραφητων αρχειων που αναφερεονται στα ακρα των διοχετευσεων
	pid_t pid;           
	int counter,eread,ewrite,eclose,ewait;
	long int result,cread;
	
	if(n == 1) {
	    printf("Number of children should be bigger than 1\n");
	    exit(-1);
	}
	
	//Δημιουργουμε n διοχετευσεις, κανάλια δεδομένων μονής κατεύθυνσης που μπορεί να χρησιμοποιηθούν για επικοινωνία μεταξύ διεργασιών
	//Παραμετρος η διευθυνση του 1ου στοιχειου του πινακα ετσι ώστε στις 2 θεσεις του πινακα να μου δημιουργησει την διοχετευση και να εγγραψει στις 2 αυτές διευθυνσεις ακεραιων το ακρο αναγνωσης και το ακρο εγγραφης
	for(int i=0; i<n; i++) { //δημιουργω εδω τα pipes για να μπορουν να δουν ολα τα παιδια τα pipe descriptors
	    if(pipe(pd[i]) < 0){ //On failure, -1 is returned
	        printf("Error: pipe");
	        exit(-1);
	    }
	} 
	 
	for(int i=0; i<n; i++){ //create n children
		pid = fork();
		if (pid < 0) perror("fork");
		else if (pid == 0){
			counter = i+1;
			//Close all pipe ends that it doesnt need
			for (int j=0; j<n-1; j++){         //close pipes of other processes
			    if(j != i && j != i-1){
			        eclose = close(pd[j][0]);
			        check_error(0,0,eclose,0);
			        eclose = close(pd[j][1]);
			        check_error(0,0,eclose,0);
			    }
 			}
 			if(i != 0 && i != n-1){
			    eclose = close(pd[n-1][0]);
			    check_error(0,0,eclose,0);
			    eclose = close(pd[n-1][1]);
			    check_error(0,0,eclose,0);
			}
			if(i != 0){                      //close writing end with previous child
			    eclose = close(pd[i-1][1]); 
			    check_error(0,0,eclose,0);
			}
			if(i == 0){                      //close writing end of last child
			    eclose = close(pd[n-1][1]); 
			    check_error(0,0,eclose,0);
			}
			eclose = close(pd[i][0]);        //close reading end with next child
			check_error(0,0,eclose,0);
			while(1){
			    //Read from previous child
				if(i == 0){ //first child
					//read up to sizeof(cread) bytes from pd[][] into the buffer starting at cread
					eread = read(pd[n-1][0], &cread, sizeof(cread)); //κολλαει μέχρι να διαβάσει sizeof(cread) bytes
					check_error(eread,0,0,0);
				}
				else{ 
					eread = read(pd[i-1][0], &cread, sizeof(cread)); //read info from previous child
					check_error(eread,0,0,0);
				    
				}
				result = cread*counter;
				//Write to next child or print result and exit
				if(counter == k){ //we are done, print result
				    eclose = close(pd[i][1]); //close writing end of child
				    check_error(0,0,eclose,0);
					printf("%d! is %ld\n", k, result);
					break;
				}
				else if(counter > k){
				    eclose = close(pd[i][1]);
				    check_error(0,0,eclose,0);
				    break;
				}
				else{ //write to next child
					counter += n; 
					//παραμετροι:περιγραφητης,από ποια διευθυνση και μετα θα αρχισει να διαβαζει αυτα που θα γραψει,ποσα bytes θα γραψει
					ewrite = write(pd[i][1], &result, sizeof(result)); //write info to next child
					check_error(0,ewrite,0,0);
			    }
			}
			exit(0);
		}
	}
	//parent code
	for (int j=0; j<n-1; j++){
	    eclose = close(pd[j][0]);
	    check_error(0,0,eclose,0);
		eclose = close(pd[j][1]);
		check_error(0,0,eclose,0);
	}
	eclose = close(pd[n-1][0]); //pipe που διαβαζει το 1ο παιδι (ιδιο με pipe που επικοινωνει το 1ο παιδι με το τελευταιο)
	check_error(0,0,eclose,0);
	result = 1;
	ewrite = write(pd[n-1][1], &result, sizeof(result)); //με το που το δεχεται το 1ο παιδι ξεκολλαει απο το read του και ξεκινανε οι διεργασιες
	check_error(0,ewrite,0,0);
	eclose = close(pd[n-1][1]);
	check_error(0,0,eclose,0);
	for(int i=0; i<n; i++) { //περιμενε ολα τα παιδια να τελειωσουν
	    ewait = wait(NULL); 
	    check_error(0,0,0,ewait);
	}
	return 0;
}



