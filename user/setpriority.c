#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(2, "usage: setpriority arguments\n");
    exit(1);
  }

  int priority = atoi(argv[1]);
  int pid = atoi(argv[2]);

  int prev = set_priority(priority, pid);

  if (prev < 0)
  {
    printf(2, "Error setting priority\n");
  }
  else
  {
    printf(1, "Priority of PID %d updated from %d to %d\n", pid, prev, priority);
  }

  exit(0);
}