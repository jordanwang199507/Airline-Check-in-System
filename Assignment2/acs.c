/*
	University of Victoria
	Nov 5th 2017
	CSC 360 Assignment 2
	Jordan Wang
	V00786970
*/
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXCUSTOMER 200
#define MAX_INPUT_SIZE 1024
#define MICROSECONDS 100000
#define TRUE 1
#define FALSE 0

/////////////// Typedefs ///////////////
typedef struct customer {
	int id;
	float arrivalTime;
	float serviceTime;
} customer;

/////////////// Constants and global variables ///////////////
pthread_t threads[MAXCUSTOMER]; // Each thread executes one customer
pthread_mutex_t mutex;
pthread_cond_t convar;
struct timeval start;
int queueLength = 0;
int clerk = FALSE;
customer customers[MAXCUSTOMER];        // The input is parsed into an array of customers
customer* queue[MAXCUSTOMER];       // Stores waiting customers while transmission clerk is occupied

/////////////// Helper functions ///////////////
void sortQueue();
void removeFromQueue();
void insertIntoQueue(customer* c);
int readCustomerFile(char* customerPath, char customerContents[MAX_INPUT_SIZE][MAX_INPUT_SIZE]);
int compareCustomers(customer* c1, customer* c2);
void requestClerk(customer* c);
void releaseClerk(customer* c);
float getTimeDifference();
void* threadFunction(void* custItem);
void parseCusts(char fileContents[MAX_INPUT_SIZE][MAX_INPUT_SIZE], int numCusts) ;

////////////////////Main///////////////////////////////
//A simulator that schedules the transmission of customers
int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Error: use as follows ACS <customers text file>\n");
		exit(1);
	}
	// Read customers text file
	char fileContents[MAX_INPUT_SIZE][MAX_INPUT_SIZE];
	if (readCustomerFile(argv[1], fileContents) != 0) {
		printf("Error: Failed to read customers file\n");
		exit(1);
	}
	// Parse input into array of customers*
	int numCusts = atoi(fileContents[0]);
	parseCusts(fileContents, numCusts);
	// Initialize mutex and cond variable
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		printf("Error: failed to initialize mutex\n");
		exit(1);
	}
	if (pthread_cond_init(&convar, NULL) != 0) {
		printf("Error: failed to initialize conditional variable\n");
		exit(1);
	}
	// Create threads in joinable state
	pthread_attr_t attr;
	if (pthread_attr_init(&attr) != 0) {
		printf("Error: failed to initialize\n");
		exit(1);
	}
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0) {
		printf("Error: failed\n");
		exit(1);
	}
	gettimeofday(&start, NULL);
	// Create a thread for each customer
	int i;
	for (i = 0; i < numCusts; i++) {
		if (pthread_create(&threads[i], &attr, threadFunction, (void*)&customers[i]) != 0){
			printf("Error: failed to create pthread.\n");
			exit(1);
		}
	}
	// Wait for all threads to terminate
	for (i = 0; i < numCusts; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			printf("Error: failed to join pthread.\n");
			exit(1);
		}
	}
	// Destroy mutex and conditional variable
	if (pthread_attr_destroy(&attr) != 0) {
		printf("Error: failed to destroy attr\n");
		exit(1);
	}
	if (pthread_mutex_destroy(&mutex) != 0) {
		printf("Error: failed to destroy mutex\n");
		exit(1);
	}
	if (pthread_cond_destroy(&convar) != 0) {
		printf("Error: failed to destroy convar\n");
		exit(1);
	}
	pthread_exit(NULL);
	return 0;
}

/////////////// Helper functions ///////////////
//Sorts queue in place using Bubblesort
void sortQueue() {
	int startingIndex;
	int x;
	int y;
	if (clerk) {
		startingIndex = 1;
	} else {
		startingIndex = 0;
	}
	for (x = startingIndex; x < queueLength; x++) {
		for (y = startingIndex; y < queueLength-1; y++) {
			if (compareCustomers(queue[y], queue[y+1]) == 1) {
				customer* temp = queue[y+1];
				queue[y+1] = queue[y];
				queue[y] = temp;
			}
		}
	}
}

void removeFromQueue() {
	int x = 0;
	while (x < queueLength-1) {
		queue[x] = queue[x+1];
		x += 1;
	}
	queueLength -= 1;
}

//Inserts c into queue
void insertIntoQueue(customer* c) {
	queue[queueLength] = c;
	queueLength += 1;
}

//Reads the file at filePath into the string array fileContents
int readCustomerFile(char* customerPath, char customerContents[MAX_INPUT_SIZE][MAX_INPUT_SIZE]) {
	FILE *cp = fopen(customerPath, "r");
	if (cp != NULL) {
		int x = 0;
		while(fgets(customerContents[x], MAX_INPUT_SIZE, cp)) { x++; }
		fclose(cp);
		return 0;
	} else { return 1; }
}

///////////////	c1: the first customer to compare ///////////////
///////////////	c2: the second customer to compare ///////////////
///////////////	compare customers///////////////
int compareCustomers(customer* c1, customer* c2) {
	if (c1->arrivalTime > c2->arrivalTime) { return 1;
	} else if (c1->arrivalTime < c2->arrivalTime) { return -1; }

	else if (c1->serviceTime > c2->serviceTime) { return 1;
	} else if (c1->serviceTime < c2->serviceTime) { return -1; }

	else if (c1->id > c2->id) { return 1;
	} else if (c1->id < c2->id) { return -1; }

	else {
		// Should not get here
		printf("Error: failed to sort\n");
		return 0;
	}
}

//Acquires the clerk
void requestClerk(customer* c) {
	if (pthread_mutex_lock(&mutex) != 0) {
		printf("Error: failed to lock mutex\n");
		exit(1);
	}
	insertIntoQueue(c);
	sortQueue();
	if (c->id != queue[0]->id) {
		printf("Customer %2d waits for the finish of clerk %2d. \n", c->id, queue[0]->id);
	}
	while (queue[0]->id != c->id) {
		if (pthread_cond_wait(&convar, &mutex) != 0) {
			printf("Error: failed to wait\n");
			exit(1);
		}
	}
	clerk = TRUE;
	if (pthread_mutex_unlock(&mutex) != 0) {
		printf("Error: failed to unlock mutex\n");
		exit(1);
	}
}

void releaseClerk(customer* c) {
	if (pthread_mutex_lock(&mutex) != 0) {
		printf("Error: failed to lock mutex\n");
		exit(1); }
	if (pthread_cond_broadcast(&convar) != 0) {
		printf("Error: failed to broadcast\n");
		exit(1); }
	removeFromQueue();
	clerk = FALSE;
	if (pthread_mutex_unlock(&mutex) != 0) {
		printf("Error: failed to unlock mutex\n");
		exit(1); }
}

//Entry point for each thread created. Handles customer.
void* threadFunction(void* custItem) {
	customer* c = (customer*)custItem;
	// Wait for arrival 
	usleep(c->arrivalTime * MICROSECONDS);
	printf("A customer arrives: customer ID %2d. \n", c->id);
	printf("A customer enters a queue: the queue ID %1d, and lenghth of the queue %2d. \n", c->id, queueLength);
	requestClerk(c);
	// Sleep for transmission 
	printf("A clerk starts serving a customer: start time %2f, the customer ID %2d, the clerk ID %2d. \n", getTimeDifference(), c->id, clerk);
	usleep(c->serviceTime * MICROSECONDS);
	printf("A clerk finishes serving a customer: end time %2f, the customer ID %2d, the clerk ID %2d. \n", getTimeDifference(), c->id, clerk);

	releaseClerk(c);
	printf("The average waiting time of a customer is %2f. \n", getTimeDifference()/c->id);
	pthread_exit(NULL);
}

//Parses input into customer
void parseCusts(char fileContents[MAX_INPUT_SIZE][MAX_INPUT_SIZE], int numCusts) {
	int i;
	for (i = 1; i <= numCusts; i++) {
		int j = 0;
		int flowVector[4];
		char* token = strtok(fileContents[i], ",");
		while (token != NULL) {
			flowVector[j] = atoi(token);
			token = strtok(NULL, ",");
			j++;
		}
		customer c = {
			flowVector[0],  // id
			flowVector[1],  // arrivalTime
			flowVector[2],  // serviceTime
		};
		customers[i-1] = c;
	}
}

//Returns the time difference in microseconds between now and start
float getTimeDifference() {
	struct timeval nowTime;
	gettimeofday(&nowTime, NULL);
	long nowMicroseconds = (nowTime.tv_sec * 10 * MICROSECONDS) + nowTime.tv_usec;
	long startMicroseconds = (start.tv_sec * 10 * MICROSECONDS) + start.tv_usec;
	return (float)(nowMicroseconds - startMicroseconds) / (10 * MICROSECONDS);
}