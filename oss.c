#define _XOPEN_SOURCE
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

void displayhelpinfo();

int main (int argc, char **argv)
{
  int opt;           // Used for command line options
  int maxchildren;   // Number of child processes to run at one time
  char logfile[20];  // Logfile to write to
  int runtime;       // Max runtime of application

  int cflag = 0, lflag = 0, tflag = 0; // Flags for command line options
  
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

  printf("maxchildren %i\n", maxchildren);
  printf("logfile: %s\n", logfile);
  printf("runtime: %i\n", runtime);

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
