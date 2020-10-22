#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

int detach(int shmid, void *shmaddr);

#define PERMS 0644

struct msgbuf {
  long mtype;
  char mtext[100];
};

int main (int argc, char **argv)
{
  int *clocknano;                  // Shared memory segment for clock nanoseconds
  int clocknanoid = atoi(argv[2]); // ID for shared memory clock nanoseconds segment
  int *clocksec;                   // Shared memory segment for clock seconds
  int clocksecid = atoi(argv[1]);  // ID for shared memory clock seconds segment
  struct msgbuf buf;               // Struct for message queue
  int msgid;                       // ID for message queue
  key_t msgkey;                    // Key for message queue
  int runtime;                     // Simulated amount of time child should run before terminating
  int shouldrun = 1;               // Flag for this child to continue to run
  int *shmpid;                     // ID for shared memory pid segment
  int shmpidid = atoi(argv[3]);    // Shared memory segment for pid
  int targettime;                  // Time at which child should attempt to terminate

  // TODO: Read shared memory clock


  // TODO: Generate random number to represent runtime in nanoseconds for child between 1-1,000,000
  runtime = (rand() % 1000000) + 1;

  // TODO: To enter Critical Section - there should be a message in queue, receive the message and enter


  // TODO: Loop in Critical Section checking Shared Clock to see if it has exceeded it's run time
  //           -- If it has: Check 'shmpid' if it is '0' then place its pid inside ELSE loop until 'shmpid' is '0'


  /* * MESSAGE QUEUE * */
  // Retrieve key established in oss.c
  if ((msgkey = ftok("msgq.txt", 'B')) == -1)
  {
    perror("user.c - ftok");
    exit(1);
  }

  // Access message queue using retrived key
  if ((msgid = msgget(msgkey, PERMS)) == -1)
  {
    perror("user.c - msgget");
    exit(1);
  }

  /* * SHARED MEMORY * */
  if ((clocksec = (int *) shmat(clocksecid, NULL, 0)) == (void *) -1)
  {
    perror("Failed to attach to memory segment.");
    return 1;
  }

  if ((clocknano = (int *) shmat(clocknanoid, NULL, 0)) == (void *) -1)
  {
    perror("Failed to attach to memory segment.");
    return 1;
  }

  if ((shmpid = (int *) shmat(shmpidid, NULL, 0)) == (void *) -1)
  {
    perror("Failed to attach to memory segment.");
    return 1;
  }

  // Read shared clock and set target time to terminate
  targettime = runtime + *clocknano;



  // Receive a message from the queue
  if(msgrcv(msgid, &buf, sizeof(buf.mtext), 0, 0) == -1)
  {
    perror("user.c - msgrcv");
    exit(1);
  }

  while (shouldrun)
  {
    
    // If child has run for specified runtime and shmpid has no pid in it currently
    if(*clocknano >= targettime && *shmpid == 0)
    {
      shouldrun = 0;      // Set flag to stop loop
      *shmpid = getpid(); // Set shared pid to this pid about to terminate
    }
  }


  /* * CLEAN UP * */
  // Detach from all shared memory segments
  detach(clocksecid, clocksec);
  detach(clocknanoid, clocknano);
  detach(shmpidid, shmpid);

  return 0;
}

int detach(int shmid, void *shmaddr)
{
  int error = 0;

  if (shmdt(shmaddr) == -1)
    error = errno;
  if (!error)
  {
    printf("CHILD: Successfully detached from the shared memory segment - id: %d\n", shmid);
    return 0;
  }
  errno = error;
  perror("Error: ");
  return -1;
}
