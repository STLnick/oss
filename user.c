#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PERMS 0644

struct msgbuf {
  long mtype;
  char mtext[100];
};

int main ()
{
  key_t key;
  struct msgbuf buf;
  int msgid;

  // Retrieve key established in oss.c
  if ((key = ftok("msgq.txt", 'B')) == -1)
  {
    perror("user.c - ftok");
    exit(1);
  }

  printf("user.c created key! %i", key);

  // Access message queue using retrived key
  if ((msgid = msgget(key, PERMS)) == -1)
  {
    perror("user.c - msgget");
    exit(1);
  }

  printf("user.c attached to msgq!");

  // Receive a message from the queue
  if(msgrcv(msgid, &buf, sizeof(buf.mtext), 0, 0) == -1)
  {
    perror("user.c - msgrcv");
    exit(1);
  }

  printf("recvd: '%s' \n", buf.mtext);

  // Remove message queue
  //if (msgctl(msgid, IPC_RMID, NULL) == -1)
  //{
  //  perror("user.c - msgctl");
  //  exit(1);
  //}


  return 0;
}
