#define _XOPEN_SOURCE
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int detachandremove(int shmid, void *shmaddr);
void displayhelpinfo();

#define MAX_PROC 100
#define PERMS 0644

struct msgbuf {
  long mtype;
  char mtext[100];
};

int main (int argc, char **argv)
{
  int childcnt = 0;   // Counter for running children
  int childpid;       // Used to determine if parent or child
  int *clocknano;     // Shared memory clock nanoseconds
  int clocknanoid;    // ID for shared memory clock nanoseconds
  key_t clocknanokey; // Key to use in creation of nanoseconds shared memory
  int *clocksec;      // Shared memory clock seconds
  int clocksecid;     // ID for shared memory clock seconds
  key_t clockseckey;  // Key to use in creation of seconds shared memory
  int len;            // Holds length of message sent into queue
  char logfile[20];   // Logfile to write to
  int maxchildren;    // Number of child processes to run at one time
  struct msgbuf buf;  // Struct used for message queue
  int msgid;          // ID for allocated message queue
  key_t msgkey;       // Key to use in creation of message queue
  int opt;            // Used for command line options
  int proccnt = 0;    // Counter for number of children ran
  int runtime;        // Max runtime of application
  int *shmpid;        // Shared memory pid
  int shmpidid;       // ID for shared memory pid
  key_t shmpidkey;    // Key to use in creation of shared memory pid segment
  int status;         // Int indicating termination status of a child process

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

  /* * * SHARED MEMORY SETUP * * */

  /* KEYS */
  // Create key to allocate shared memory clock seconds segment
  if ((clockseckey = ftok("msgq.txt", 'A')) == -1)
  {
    perror("ftok");
    exit(1);
  }

  // Create key to allocate shared memory clock nanoseconds segment
  if ((clocknanokey = ftok("msgq.txt", 'C')) == -1)
  {
    perror("ftok");
    exit(1);
  }

  // Create key to allocate shared memory pid segment
  if ((shmpidkey = ftok("msgq.txt", 'D')) == -1)
  {
    perror("ftok");
    exit(1);
  }

  /* IDS */
  if ((clocksecid = shmget(clockseckey, sizeof(int *), IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  if ((clocknanoid = shmget(clocknanokey, sizeof(int *), IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  if ((shmpidid = shmget(shmpidkey, sizeof(int *), IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  /* ATTACH */
  if ((clocksec = (int *) shmat(clocksecid, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(clocksecid, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }
  printf("Successfully attached to clocksec shared memory!\n");

  if ((clocknano = (int *) shmat(clocknanoid, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(clocknanoid, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }
  printf("Successfully attached to clocknano shared memory!\n");

  if ((shmpid = (int *) shmat(shmpidid, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(shmpidid, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }
  printf("Successfully attached to shmpid shared memory!\n");
  
   /* * * MESSAGE QUEUE SETUP * * */

  // Create key to allocate message queue
  if ((msgkey = ftok("msgq.txt", 'B')) == -1)
  {
    perror("ftok");
    exit(1);
  }

  // Allocate message queue and store returned ID
  if ((msgid = msgget(msgkey, PERMS | IPC_CREAT)) == -1)
  {
    perror("msgget: ");
    exit(1);
  }


  // TODO: MESSAGE QUEUE
  //        - Implement a Critical Section and have parent/child alternate modifying one of the values

  // TESTING with a hard coded string to send to child process
  buf.mtype = 1;

  strcpy(buf.mtext, "testing");
  len = strlen(buf.mtext);

  if (msgsnd(msgid, &buf, len+1, 0) == -1)
    perror("msgsnd:");


  /* * * FORK TO CHILD * * */

  if ((childpid = fork()) == -1)
  {
    perror("fork failed\n");
    exit(1);
  }

  // Child Code
  if (childpid == 0)
  {
    char strclocksecid[100+1] = {'\0'}; // Create string from shared memory clock seconds id
    sprintf(strclocksecid, "%d", clocksecid); 

    char strclocknanoid[100+1] = {'\0'}; // Create string from shared memory clock nanoseconds id
    sprintf(strclocknanoid, "%d", clocknanoid); 

    char strshmpidid[100+1] = {'\0'}; // Create string from shared memory pid id
    sprintf(strshmpidid, "%d", shmpidid); 

    char *args[] = {"./user", strclocksecid, strclocknanoid, strshmpidid, '\0'};

    execv(args[0], args);
    perror("Child failed to execv\n");
  }

  // Parent Code
  if (childpid > 0)
  {
    wait(&status);
    printf("Parent waited for child! Exit status: %i\n", status);
    printf("shmpid after wait(): %i", *shmpid);
  }

  /* * * CLEAN UP * * */

  // Remove message queue
  if (msgctl(msgid, IPC_RMID, NULL) == -1)
  {
    perror("msgctl: ");
    exit(1);
  }

  // Detach from and remove all shared memory segments
  detachandremove(clocksecid, clocksec);
  detachandremove(clocknanoid, clocknano);
  detachandremove(shmpidid, shmpid);

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

int detachandremove(int shmid, void *shmaddr)
{
  int error = 0;

  if (shmdt(shmaddr) == -1)
    error = errno;
  if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error)
    error = errno;
  if (!error)
  {
    printf("Successfully detached and removed the shared memory segment - id: %d\n", shmid);
    return 0;
  }
  errno = error;
  perror("Error: ");
  return -1;
}
