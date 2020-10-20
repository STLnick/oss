#define _XOPEN_SOURCE
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

void displayhelpinfo();

#define MAX_PROC 100
#define PERMS 0644

struct msgbuf {
  long mtype;
  char mtext[100];
};

int main (int argc, char **argv)
{
  int childcnt = 0;  // Counter for running children
  int childpid;      // Used to determine if parent or child
  key_t key;         // Key to use in creation of message queue
  int len;           // Holds length of message sent into queue
  char logfile[20];  // Logfile to write to
  int maxchildren;   // Number of child processes to run at one time
  struct msgbuf buf; // Struct used for message queue
  int msgid;         // ID for allocated message queue
  int opt;           // Used for command line options
  int proccnt = 0;   // Counter for number of children ran
  int runtime;       // Max runtime of application

  int cflag = 0, lflag = 0, tflag = 0; // Flags for command line options

  // TODO: Implement shared clock - ensure children can see/modify the values
  //       -- 'periodicasterick.c' pg. 318 in textbook (example)
  //       -- One int for 'seconds' one int for 'nanoseconds'

  // Create dummy txt file to create a key with ftok
  system("touch msgq.txt");
  
  while ((opt = getopt(argc, argv, "hc:l:t:")) != -1)
  {
    switch (opt)
    {
      case 'h': // Help - describe how to run program and default values
        displayhelpinfo();
        exit(EXIT_SUCCESS);
        break;
      case 'c': // Specify number of child processes to be ran
        cflag = 1;
        maxchildren = atoi(optarg);
        break;
      case 'l': // Specify name of logfile to write to
        lflag = 1;
        strcpy(logfile, optarg);
        break;
      case 't': // Specify number of seconds the program will run before terminating
        tflag = 1;
        runtime = atoi(optarg);
        break;
      default:
        printf("Please use -h for help to see valid options.\n");
        exit(EXIT_FAILURE);
    }
  }

  // Set default values if none specified in command line
  maxchildren = cflag ? maxchildren : 5;
  runtime = tflag ? runtime : 20;
  if (!lflag) strcpy(logfile, "output.log");

  // Enforce hard limits on options
  maxchildren = maxchildren > 10 ? 10 : maxchildren;
  maxchildren = maxchildren > 0 ? maxchildren : 5;
  runtime = runtime > 120 ? 120 : runtime;
  runtime = runtime > 0 ? runtime : 20;


  // Create key to allocate message queue
  if ((key = ftok("msgq.txt", 'B')) == -1)
  {
    perror("ftok");
    exit(1);
  }

  printf("oss.c created key: %i", key);

  // Allocate message queue and store returned ID
  if ((msgid = msgget(key, PERMS | IPC_CREAT)) == -1)
  {
    perror("msgget: ");
    exit(1);
  }



  // TODO: MESSAGE QUEUE
  //        - Simple test that parent/child can communicate with queue
  //        - Implement a Critical Section and have parent/child alternate modifying one of the values

  // TESTING with a hard coded string to send to child process
  buf.mtype = 1;

  strcpy(buf.mtext, "testing");
  len = strlen(buf.mtext);

  if (msgsnd(msgid, &buf, len+1, 0) == -1)
    perror("msgsnd:");

  printf("Message sent!\n");


  /* * * FORK TO CHILD * * */

  if ((childpid = fork()) == -1)
  {
    perror("fork failed\n");
    exit(1);
  }

  // Child Code
  if (childpid == 0)
  {
    execv("./user");
    perror("Child failed to execv\n");
  }

  // Parent Code
  if (childpid > 0)
  {
    wait(childpid);
    printf("Parent waited for %i to finish!\n", childpid);
  }

  /* * * CLEAN UP * * */

  // Remove message queue
  if (msgctl(msgid, IPC_RMID, NULL) == -1)
  {
    perror("msgctl: ");
    exit(1);
  }

  // Remove dummy txt file used to create key with ftok
  system("rm msgq.txt");

  return 0;
}

void displayhelpinfo()
{
  printf("\nOperating System Simulator\n");
  printf("-------------------------\n");
  printf("Example command to run ./oss:\n\n");
  printf("./oss -c 10 -l logfile -t 60\n\n");
  printf("-------------------------\n");
  printf("Program options information:\n");
  printf("-c = The total number of child processes to be ran.\n");
  printf("-l = The name of the logfile to write to.\n");
  printf("-t = The time in seconds that the program will execute. This will end\n");
  printf("     execution even if the processes haven't finished the tasks.\n\n");
  printf("-------------------------\n");
  printf("Default value information:\n");
  printf("All options specified above are optional and will use the\n");
  printf("following default values if not specified.\n\n");
  printf("-c = 10\n");
  printf("-l = output.log\n");
  printf("-t = 100\n\n");
  printf("NOTE: Any Negative values passed will be set to these default values\n");
  printf("      Any excessive values will be set to a ceiling value.\n");
  printf("-------------------------\n\n");
}
