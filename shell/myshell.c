#include "csapp.h"
#define MAXARGS 128
#define MAXJOBS 128

typedef struct {
  pid_t pid;
  char cmd[MAXLINE];
} JOB;

/* function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int add_job(pid_t pid, char *cmd);
pid_t remove_job(pid_t pid);

static JOB jobs[MAXJOBS];
static pid_t f_process;

void reap_children(int sig) {

  pid_t pid;

  while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    remove_job(pid);

  return;
}

void call_frontend_process(int sig) {

  if (f_process) {
    printf("---%d---%d\n", f_process, sig);
    // Kill(-f_process, sig);
  }
  return;
}

int main() {
  char cmdline[MAXLINE]; /* Command line */

  Signal(SIGCHLD, reap_children);

  Signal(SIGINT, call_frontend_process);
  Signal(SIGTSTP, call_frontend_process);

  while (1) {
    /* Read */
    printf("> ");
    Fgets(cmdline, MAXLINE, stdin);
    if (feof(stdin))
      exit(0);

    /* Evaluate */
    eval(cmdline);
  }
}

/* eval - Evaluate a command line */
void eval(char *cmdline) {
  char *argv[MAXARGS]; /* Argument list execve() */
  char buf[MAXLINE];   /* Holds modified command line */
  int bg;              /* Should the job run in bg or fg? */
  pid_t pid;           /* Process id */

  strcpy(buf, cmdline);
  bg = parseline(buf, argv);
  if (argv[0] == NULL)
    return; /* Ignore empty lines */

  if (!builtin_command(argv)) {
    if ((pid = Fork()) == 0) { /* Child runs user job */

      /* 进程组id与进程id一致 */
      pid = getpid();
      Setpgid(pid, pid);

      if (execve(argv[0], argv, environ) < 0) {
        printf("%s: Command not found.\n", argv[0]);
        exit(0);
      }
    }

    /* Parent waits for foreground job to terminate */
    if (!bg) {
      int status;
      f_process = pid;
      if (waitpid(pid, &status, 0) < 0)
        unix_error("waitfg: waitpid error");
      f_process = 0;
    } else {
      int jid = add_job(pid, cmdline);
      printf("[%d] %d %s", jid, pid, cmdline);
    }
  }
  return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) {
  if (!strcmp(argv[0], "jobs")) { /* list jobs */
    for (int i = 0; i < MAXJOBS; i++) {
      if (jobs[i].pid)
        printf("[%d] %d %s\n", i, jobs[i].pid, jobs[i].cmd);
    }
    return 1;
  }

  if (!strcmp(argv[0], "bg") || !strcmp(argv[0], "fg")) {
    pid_t pid;

    if (!argv[1]) {
      return 1;
    }

    if (argv[1][0] == '%') {
      int jobid = atoi(&argv[1][1]);
      if (jobid > MAXJOBS - 1) {
        printf("job [%d] not found\n", jobid);
        return 1;
      }

      JOB j = jobs[jobid];
      pid = j.pid;
      if (!pid) {
        printf("job [%d] not found\n", jobid);
        return 1;
      }
    } else {

      pid = atoi(argv[1]);
    }

    Kill(pid, SIGCONT);
    // 前台运行，则当前进程等待子进程运行完毕
    if (!strcmp(argv[0], "fg")) {
      remove_job(pid);
      int status;
      if (waitpid(pid, &status, 0) < 0)
        unix_error("waitfg: waitpid error");
    }

    return 1;
  }

  if (!strcmp(argv[0], "quit")) /* quit command */
    exit(0);

  if (!strcmp(argv[0], "&")) /* Ignore singleton & */
    return 1;

  return 0; /* Not a builtin command */
}

/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) {
  char *delim; /* Points to first space delimiter */
  int argc;    /* Number of args */
  int bg;      /* Background job? */

  buf[strlen(buf) - 1] = ' ';   /* Replace trailing '\n' with space */
  while (*buf && (*buf == ' ')) /* Ignore leading spaces */
    buf++;

  /* Build the argv list */
  argc = 0;
  while ((delim = strchr(buf, ' '))) {
    argv[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')) /* Ignore spaces */
      buf++;
  }
  argv[argc] = NULL;

  if (argc == 0) /* Ignore blank line */
    return 1;

  /* Should the job run in the background? */
  if ((bg = (*argv[argc - 1] == '&')) != 0)
    argv[--argc] = NULL;

  return bg;
}

int add_job(pid_t pid, char *cmd) {

  for (int i = 0; i < MAXJOBS; i++) {
    if (!jobs[i].pid) {
      jobs[i].pid = pid;
      strcpy(jobs[i].cmd, cmd);
      return i;
    }
  }
  return 0;
}

pid_t remove_job(pid_t pid) {

  for (int i = 0; i < MAXJOBS; i++) {
    if (jobs[i].pid && jobs[i].pid == (int)pid) {
      jobs[i].pid = 0;
      return pid;
    }
  }
  return 0;
}
