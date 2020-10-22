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
  int *clocknano;
  int clocknanoid = atoi(argv[2]);
  int *clocksec;
  int clocksecid = atoi(argv[1]);
  struct msgbuf buf;
  int msgid;
  key_t msgkey;
  int *shmpid;
  int shmpidid = atoi(argv[3]);



  // TODO: Read shared memory clock

  // TODO: Generate random number to represent runtime in nanoseconds for child between 1-1,000,000

  // TODO: To enter Critical Section - there should be a message in queue, receive the message and enter

  // TODO: Loop in Critical Section checking Shared Clock to see if it has exceeded it's run time
  //           -- If it has: Check 'shmpid' if it is '0' then place its pid inside ELSE loop until 'shmpid' is '0'



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

  // TESTING
  printf("CHILD: pid %i\n", getpid());
  *shmpid = getpid();

  // Receive a message from the queue
  if(msgrcv(msgid, &buf, sizeof(buf.mtext), 0, 0) == -1)
  {
    perror("user.c - msgrcv");
    exit(1);
  }

  printf("recvd: '%s' \n", buf.mtext);

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
