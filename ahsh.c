#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int numCommands = 0;

int CommandIndex(char *line, int numChars)
{ // if we are in this function, then the line must
    // currently have "cd " as the first three characters.
    // we then check if there are any characters following this,
    // otherwise if everything after is ' '.
    while (line[numChars] != '\0')
    {
        if (line[numChars] != ' ')
        {
            return numChars;
        }
        numChars++;
    }
    return 0;
}

void getCommand(char *line, char *path, int numChars)
{
    int i = numChars;
    while (line[i] != '\0')
    {
        // take the path from the line, which is the
        // characters following cd.
        path[i - numChars] = line[i];
        i++;
    }
}

void changeDirectory(char *line, char *path)
{
    int j = 3;
    // check if the current line contains a directory for
    // cd to take the user to, otherwise if none is inputted,
    // cd should take the user to the home directory.
    int numChars = CommandIndex(line, 3);
    if (numChars > 0)
    {
        getCommand(line, path, 3);

        // change the directory, no directory change will happen
        // if it's invalid, but it will return -1, hence
        // we print that there is no file or directory.
        int isValid = chdir(path);
        if (isValid == -1)
        {
            printf("cd: %s: No such file or directory\n", path);
        }
        else
        {
            numCommands++;
        }
    }
    else
    {
        chdir("/home");
        numCommands++;
    }
}

int CdCheck(char *line, char *path)
{
    // first check if cd is the only thing in the line
    if (strcmp(line, "cd") == 0)
    {
        // if so, change the directory to home.
        chdir("/home");
        numCommands++;
        return 1;
    }
    // if the line contains "cd ", there is potential
    // that the line contains a directory (or perhaps just
    //  more spaces so we need to check both cases).
    else if (strncmp(line, "cd ", 3) == 0)
    {
        changeDirectory(line, path);
        return 1;
    }
    return 0;
}

void PrintTableEntries(char **history)
{
    int i = 0;
    for (int i = 0; i < 10; i++)
    {
        if (history[i] == NULL)
        {
            printf("%d:\n", (i + 1));
        }
        else
        {
            printf("%d: %s\n", (i + 1), history[i]);
        }
    }
}

void addToHistory(char *line, int numChars, char **history)
{
    char *pathString = malloc(100 * sizeof(char));
    int index = CommandIndex(line, numChars);
    getCommand(line, pathString, index);
    int i = 0;
    while (history[i] != NULL)
    {
        i++;
    }
    if (i < 10)
    {
        history[i] = pathString;
    }
    else
    {
        for (int j = 0; j < 9; j++)
        {
            history[j] = history[j + 1];
        }
        history[9] = pathString;
    }
    free(pathString);
}

int ExecuteCommand(char *command, char **history)
{
    char *line = malloc(100 * sizeof(char));
    strcpy(line, command);
    char delim[] = " ";
    char **tokens = malloc(100 * sizeof(char *));
    int i = 0;
    tokens[i] = strtok(command, delim);
    while (tokens[i] != NULL)
    {
        i++;
        tokens[i] = strtok(NULL, delim);
    }
    pid_t id = fork();
    if (id == 0)
    {
        if (execvp(tokens[0], tokens) == -1)
        {
            perror("invalid command");
        }
    }
    else
    {
        pid_t pid = waitpid(0, NULL, id);
        free(tokens);
        numCommands++;
        addToHistory(line, 0, history);
    }
}

void MakeHistoryTable(int noCommand, char **history, char *line, char *path)
{

    if (noCommand)
    {
        PrintTableEntries(history);
    }
    else
    {
        int index = CommandIndex(line, 8);
        getCommand(line, path, index);
        int i = 0;
        while (history[i] != NULL)
        {
            i++;
        }
        if (i < 10)
        {
            history[i] = path;
            PrintTableEntries(history);
        }
        else
        {
            for (int j = 0; j < 9; j++)
            {
                history[j] = history[j + 1];
            }
            history[9] = path;
            PrintTableEntries(history);
            ExecuteCommand(path, history);
        }
    }
}

int HistoryCheck(char *line, char *path, char **history)
{
    if ((strcmp(line, "history") == 0) || (strcmp(line, "h") == 0))
    {
        MakeHistoryTable(1, history, line, path);
        return 1;
    }
    else if ((strncmp(line, "history ", 8) == 0) || (strncmp(line, "h ", 2) == 0))
    {
        MakeHistoryTable(0, history, line, path);
        return 1;
    }
    return 0;
}

int SpecialCommandsCheck(char *line, char *path, char **history)
{
    int CdExecuted = CdCheck(line, path);
    int HistoryExecuted = HistoryCheck(line, path, history);
    if (CdExecuted || HistoryExecuted)
    {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{

    // Introduce user to Ahsh Shell
    printf("Welcome to ahsh Shell!\n");
    printf("--------------------------\n\n");
    // This part does not repeat, hence outside of while loop.
    char **history = malloc(10 * sizeof(char *));
    while (1)
    {
        // every iteration of the while loop should represent the following
        // An input is taken in
        // this input is processed into tokens
        // these tokens are taken and executed.
        printf("ahsh>  ");
        // set up a char array and size value, so the user's input can be taken and put into this array.
        char *line = malloc(100 * sizeof(char));
        size_t size = 0;
        size_t characters = getline(&line, &size, stdin);

        // note that when the user types their input, the enter key is used to signal that they have finished typing.
        // this leaves getline with an extra \n, so we manually remove this.
        line[strlen(line) - 1] = '\0';

        // allocate some memory to store the path section of the line.
        char *path = malloc(100 * sizeof(char));

        // check if the word you want is a special command:
        int alreadyExecuted = SpecialCommandsCheck(line, path, history);
        if (!alreadyExecuted)
        {
            ExecuteCommand(line, history);
        }
        printf("%d\n", numCommands);
    }
    return 0;
}
