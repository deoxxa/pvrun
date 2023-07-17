#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>

#define PV_BUF_SIZE 1024 * 8
#define PV_ARGV_SIZE 50

int add_pv_option(char *buf, int *buf_pos, char **argv, int *argv_pos,
                  const char *format, ...) {
  va_list args;

  va_start(args, format);
  int buf_wrote =
      vsnprintf(&buf[*buf_pos], PV_BUF_SIZE - *buf_pos, format, args);
  if (buf_wrote < 0) {
    return buf_wrote;
  }
  va_end(args);

  if ((*buf_pos + buf_wrote + 1) >= PV_BUF_SIZE) {
    return -ENOBUFS;
  }
  if ((*argv_pos + 1) >= PV_ARGV_SIZE) {
    return -ENOBUFS;
  }

  argv[*argv_pos] = &buf[*buf_pos];

  *buf_pos += buf_wrote + 1;
  *argv_pos += 1;

  return buf_wrote;
}

#define ADD_PV_OPTION_OR_EXIT(buf, buf_pos, argv, argv_pos, ...)               \
  do {                                                                         \
    if (add_pv_option(buf, buf_pos, argv, argv_pos, __VA_ARGS__) < 0) {        \
      fprintf(stderr, "error: could not add option to pv command buffer\n");   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0);

#define VERSION_TEMPLATE                                                       \
  "pvrun version 1.0.0\n"                                                      \
  "copyright 2023 conrad pankoff <deoxxa@fknsrs.biz> "                         \
  "(https://www.fknsrs.biz/)\n"

void print_version(FILE *stream) { fprintf(stream, VERSION_TEMPLATE); }

#define USAGE_TEMPLATE                                                         \
  "usage: %1$s [-h/-V] [OPTIONS... --] PROGRAM [ARGUMENTS]\n"                  \
  "  -h             print this help and exit (must be first argument)\n"       \
  "  -V             print version and exit (must be first argument)\n"         \
  "  OPTIONS        additional arguments to supply to pv (optional). must\n"   \
  "                 begin with an argument starting with '-', must be\n"       \
  "                 terminated with --, can not contain options -d or -R,\n"   \
  "                 maximum count of %3$d, maximum combined length of %2$d\n"  \
  "                 bytes including spaces. if a positional 'file' argument\n" \
  "                 is supplied, it will not work as expected. see 'man pv'\n" \
  "                 for more information.\n"                                   \
  "  PROGRAM        program to run (required)\n"                               \
  "  ARGUMENTS      arguments for target program (optional)\n"                 \
  "\n"                                                                         \
  "examples:\n"                                                                \
  "  %1$s -h\n"                                                                \
  "  %1$s -V\n"                                                                \
  "  %1$s cp src_file dst_file\n"                                              \
  "  %1$s -- cp -a src_dir dst_dir\n"                                          \
  "  %1$s -N my-copy -- cp -a src_dir dst_dir\n"

void print_usage_and_exit(const char *name, FILE *stream, int exit_code) {
  fprintf(stream, USAGE_TEMPLATE, name, PV_BUF_SIZE, PV_ARGV_SIZE);
  exit(exit_code);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_version(stderr);
    fprintf(stderr, "\n");
    print_usage_and_exit(argv[0], stderr, 1);
  }

  if (strcmp(argv[1], "-h") == 0) {
    print_version(stdout);
    fprintf(stdout, "\n");
    print_usage_and_exit(argv[0], stdout, 0);
  }

  if (strcmp(argv[1], "-V") == 0) {
    print_version(stdout);
    exit(0);
  }

  int command_pos = -1;

  char pv_buf[PV_BUF_SIZE + 1] = {0};
  int pv_buf_pos = 0;
  char *pv_argv[PV_ARGV_SIZE + 1] = {NULL};
  int pv_argv_pos = 0;

  ADD_PV_OPTION_OR_EXIT(pv_buf, &pv_buf_pos, pv_argv, &pv_argv_pos, "pv")

  if (strchr(argv[1], '-') == argv[1]) {
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--") == 0) {
        command_pos = i + 1;
        break;
      }

      if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-R") == 0) {
        fprintf(stderr, "error: disallowed pv option '%s' supplied\n\n",
                argv[i]);
        print_usage_and_exit(argv[0], stderr, 1);
      }

      ADD_PV_OPTION_OR_EXIT(pv_buf, &pv_buf_pos, pv_argv, &pv_argv_pos, argv[i])
    }
  } else {
    command_pos = 1;
  }

  if (command_pos == -1 || command_pos >= argc) {
    fprintf(stderr, "error: program not provided or mixed with pv arguments - "
                    "probably '--' missing or in the wrong place\n\n");
    print_usage_and_exit(argv[0], stderr, 1);
  }

  pid_t ppid_before_fork = getpid();

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) {
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) == -1) {
      perror("prctl");
      exit(1);
    }
    if (getppid() != ppid_before_fork) {
      fprintf(stderr, "parent already exited\n");
      exit(1);
    }

    execvp(argv[command_pos], &argv[command_pos]);
    perror("execvp (child)");
    exit(1);
  }

  ADD_PV_OPTION_OR_EXIT(pv_buf, &pv_buf_pos, pv_argv, &pv_argv_pos, "-d")
  ADD_PV_OPTION_OR_EXIT(pv_buf, &pv_buf_pos, pv_argv, &pv_argv_pos, "%d", pid)

  execvp(pv_argv[0], &pv_argv[0]);
  perror("execvp (parent)");
  exit(1);
}
