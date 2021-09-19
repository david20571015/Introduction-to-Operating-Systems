/*
Student No.: 0710734
Student Name: 邱俊耀
Email: david20571015.eed07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMD_LENGTH 256
#define TOKEN_NUM 10

char *ReadCommand(FILE *input_file) {
  char *command = (char *)malloc(sizeof(char) * CMD_LENGTH);
  fgets(command, CMD_LENGTH, input_file);
  command[strlen(command) - 1] = '\0'; /* replace '\n' with '\0' */
  return command;
}

char **Tokenize(char *command) {
  char **tokens = (char **)malloc(sizeof(char *) * TOKEN_NUM);
  size_t idx = 0;

  if (strlen(command)) {
    char *token = strtok(command, " ");
    do {
      if (strcmp(token, "&") == 0) {
        tokens[idx++] = NULL;
      }
      tokens[idx++] = token;
      token = strtok(NULL, " ");
    } while (token);
  }

  while (idx < TOKEN_NUM) {
    tokens[idx++] = NULL;
  }

  return tokens;
}

int IsForegroundExecute(char **tokens) {
  int flag = 1;
  for (size_t i = 0; i < TOKEN_NUM; ++i) {
    if (tokens[i] != NULL && strcmp(tokens[i], "&") == 0) {
      flag = 0;
      break;
    }
  }

  return flag;
}

int main() {
  signal(SIGCHLD, SIG_IGN);

  while (1) {
    printf("> ");

    char *command = ReadCommand(stdin);
    char **tokens = Tokenize(command);

    pid_t pid = fork();

    switch (pid) {
      case -1: {
        fprintf(stderr, "Fork failed");
        break;
      }
      case 0: { /* child process */
        execvp(tokens[0], tokens);
        break;
      }
      default: { /* parent process */
        if (IsForegroundExecute(tokens)) {
          waitpid(pid, NULL, 0);
        }
        break;
      }
    }

    free(tokens);
    free(command);
  }

  return 0;
}
