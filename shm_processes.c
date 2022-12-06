#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

void ClientProcess(int[], sem_t*);

int main(int argc, char * argv[]) {
  int ShmID;
  int *ShmPTR;
  pid_t pid;
  int status;
  sem_t *mutex;

  // seed random with current time
  time_t t;
  srandom((unsigned) time( & t));

  ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  if (ShmID < 0) {
    printf("*** shmget error (server) ***\n");
    exit(1);
  }

  ShmPTR = (int * ) shmat(ShmID, NULL, 0);
  if ( * ShmPTR == -1) {
    printf("*** shmat error (server) ***\n");
    exit(1);
  }
  
  /* create, initialize semaphore */
 if ((mutex = sem_open("examplesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }
  
  // bank account is at position 0 and turn at position 1 
  *ShmPTR = 0;

  pid = fork();
  if (pid < 0) {
    exit(1);
  } else if (pid == 0) {
    ClientProcess(ShmPTR, mutex);
    exit(0);
  }
  
  // parent process continues

  int localBalance;
  int balance;

  while (1) {

    // sleep some random amount of time between 0 and 5
    sleep(random() % 6);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    sem_wait(mutex);
    localBalance = *ShmPTR;
    
    balance = random() % 101;
    
//     if(balance % 2 == 0) {
//       if(localBalance < 100) {
//         // try to deposit money
//         localBalance += balance;
//       } else {
//         printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
//       }
//     } else {
//       printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
//     }

    if (localBalance <= 100) {
      // try to deposit money

      // number between 0 and 100
      balance = random() % 101;

      if (balance % 2 == 0) {
        localBalance += balance;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, localBalance);
      } else {
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
    } else {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
    }
    
    *ShmPTR = localBalance;
    sem_post(mutex);
  }

  wait( & status);
  printf("Server has detected the completion of its child...\n");
  shmdt((void * ) ShmPTR);
  printf("Server has detached its shared memory...\n");
  shmctl(ShmID, IPC_RMID, NULL);
  printf("Server has removed its shared memory...\n");
  printf("Server exits...\n");
  exit(0);
}

void ClientProcess(int SharedMem[], sem_t* mutex) {
  int localBalance;
  int balance;
  int i;

  while (1) {
    // sleep some random amount of time between 0 and 5
    sleep(random() % 6);
    printf("Poor Student: Attempting to Check Balance\n");
    
    sem_wait(mutex);
    localBalance = *SharedMem;
    
    // number between 0 and 50
    balance = random() % 51;
    
    printf("Poor Student needs $%d\n", balance);
    
    if (balance <= localBalance) {
      localBalance -= balance;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, localBalance);
    } else {
      printf("Poor Student: Not Enough Cash ($%d)\n", localBalance );
    }
    
    *SharedMem = localBalance;
    sem_post(mutex);
  }
}