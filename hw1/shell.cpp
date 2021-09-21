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

char* ReadCommand(FILE* source = stdin) {
  char* command = (char*)malloc(sizeof(char) * CMD_LENGTH);
  fgets(command, CMD_LENGTH, source);

  command[strlen(command) - 1] = '\0'; /* replace '\n' with '\0' */
  size_t i = strlen(command) - 1;
  while (i >= 0 && command[i] == ' ') command[i--] = '\0';

  return command;
}

void InputRedirect(char* args[], FILE* input_file) {
  pid_t pid = fork();

  switch (pid) {
    case -1: {
      fprintf(stderr, "Fork failed");
      break;
    }
    case 0: { /* child process */
      dup2(fileno(input_file), STDIN_FILENO);
      execvp(args[0], args);
      break;
    }
    default: { /* parent process */
      waitpid(pid, NULL, 0);
      _exit(0);
      break;
    }
  }
}

void OutputRedirect(char* args[], FILE* output_file) {
  pid_t pid = fork();

  switch (pid) {
    case -1: {
      fprintf(stderr, "Fork failed");
      break;
    }
    case 0: { /* child process */
      dup2(fileno(output_file), STDOUT_FILENO);
      execvp(args[0], args);
      break;
    }
    default: { /* parent process */
      waitpid(pid, NULL, 0);
      _exit(0);
      break;
    }
  }
}

void Pipeline(char* first_cmd[], char* second_cmd[]) {
  int p[2];

  if (pipe(p) == -1) {
    fprintf(stderr, "Pipe failed");
    return;
  }

  pid_t pid = fork();

  switch (pid) {
    case -1: {
      fprintf(stderr, "Fork failed");
      break;
    }
    case 0: { /* child process */
      close(p[0]);
      dup2(p[1], STDOUT_FILENO);
      close(p[1]);
      execvp(first_cmd[0], first_cmd);
      break;
    }
    default: { /* parent process */
      waitpid(pid, NULL, 0);
      close(p[1]);
      dup2(p[0], STDIN_FILENO);
      close(p[0]);
      execvp(second_cmd[0], second_cmd);
      break;
    }
  }
}

struct Command {
 public:
  char **first_cmd, **second_cmd;
  FILE *input_file, *output_file;
  int is_foreground_execute;
  enum CommandType { SINGLE_COMMAND, INPUT_REDIRECT, OUTPUT_REDIRECT, PIPELINE } type;

  Command(char* cmd) {
    first_cmd = second_cmd = NULL;
    input_file = output_file = NULL;
    is_foreground_execute = 1;
    type = SINGLE_COMMAND;

    tokens = Tokenize(cmd);

    first_cmd = tokens;

    for (size_t i = 0; i < TOKEN_NUM; ++i) {
      char* token = tokens[i];
      if (token == NULL) continue;
      if (strcmp(token, "<") == 0) {
        input_file = fopen(tokens[i + 1], "r");
        type = INPUT_REDIRECT;
      } else if (strcmp(token, ">") == 0) {
        output_file = fopen(tokens[i + 1], "w");
        type = OUTPUT_REDIRECT;
      } else if (strcmp(token, "|") == 0) {
        second_cmd = tokens + i + 1;
        type = PIPELINE;
      } else if (strcmp(token, "&") == 0) {
        is_foreground_execute = 0;
        break;
      }
    }
  }

  ~Command() {
    if (input_file != NULL) fclose(input_file);
    if (output_file != NULL) fclose(output_file);
    free(tokens);
  }

 private:
  char** tokens;

  char** Tokenize(char* command) {
    char** tokens = (char**)malloc(sizeof(char*) * TOKEN_NUM);
    size_t idx = 0;

    char* token = strtok(command, " ");
    do {
      if (!strcmp(token, "|") || !strcmp(token, "<") || !strcmp(token, ">") || !strcmp(token, "&")) {
        tokens[idx++] = NULL;
      }
      tokens[idx++] = token;
      token = strtok(NULL, " ");
    } while (token);

    while (idx < TOKEN_NUM) {
      tokens[idx++] = NULL;
    }

    return tokens;
  }
};

int main() {
  signal(SIGCHLD, SIG_IGN);

  while (1) {
    printf("> ");

    char* cmd = ReadCommand(stdin);
    if (strlen(cmd) == 0) continue;
    Command command(cmd);

    pid_t pid = fork();

    switch (pid) {
      case -1: {
        fprintf(stderr, "Fork failed");
        break;
      }
      case 0: { /* child process */
        switch (command.type) {
          case Command::CommandType::INPUT_REDIRECT: {
            InputRedirect(command.first_cmd, command.input_file);
            break;
          }
          case Command::CommandType::OUTPUT_REDIRECT: {
            OutputRedirect(command.first_cmd, command.output_file);
            break;
          }
          case Command::CommandType::PIPELINE: {
            Pipeline(command.first_cmd, command.second_cmd);
            break;
          }
          default: {
            execvp(command.first_cmd[0], command.first_cmd);
            break;
          }
        }
        break;
      }
      default: { /* parent process */
        if (command.is_foreground_execute) waitpid(pid, NULL, 0);
        break;
      }
    }

    free(cmd);
  }

  return 0;
}
