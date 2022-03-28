#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

// Global Variables:
int numCommands = 0;
char **history;

// Prototype Declarations:
int CommandIndex(char *line, int numChars);
void GetCommand(char *line, char *path, int numChars);
void AddToHistory(char *line);
void ChangeDirectory(char *line);
int CdCheck(char *line);
void PrintTableEntries();
void PrintTableEntries();
int ExecuteBasicCommand(char *command);
int SpecialCommandsCheck(char *line);
void ExecuteFullCommand(char *line);
void ExecuteHistoryCommand(char *line);
int HistoryCheck(char *line);
char *InputProcessor();

int main(int argc, char *argv[])
{

    // Introduce user to Ahsh Shell
    printf("Welcome to ahsh Shell!\n");
    printf("--------------------------\n\n");
    history = malloc(10 * sizeof(char *));
    // This part does not repeat, hence outside of while loop.
    while (1)
    {
        printf("ahsh>  ");
        // every iteration of the while loop should represent the following:
        
        // An input is taken in and processed
        char *processedLine = InputProcessor();

        // this processed input is executed
        ExecuteFullCommand(processedLine);
    }
    return 0;
}

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

void GetCommand(char *line, char *path, int numChars)
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

void AddToHistory(char *line)
{
    char *pathString = malloc(100 * sizeof(char));
    int index = CommandIndex(line, 0);
    GetCommand(line, pathString, index);
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
}

void ChangeDirectory(char *line)
{
    int j = 3;
    // check if the current line contains a directory for
    // cd to take the user to, otherwise if none is inputted,
    // cd should take the user to the home directory.
    int numChars = CommandIndex(line, 3);
    if (numChars > 0)
    {
        char *path = malloc(100 * sizeof(char));
        GetCommand(line, path, 3);

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
            AddToHistory(line);
            numCommands++;
        }
    }
    else
    {
        chdir("/home");
        AddToHistory(line);
        numCommands++;
    }
}

int CdCheck(char *line)
{
    // first check if cd is the only thing in the line
    if (strcmp(line, "cd") == 0)
    {
        // if so, change the directory to home.
        chdir("/home");
        AddToHistory(line);
        numCommands++;
        return 1;
    }
    // if the line contains "cd ", there is potential
    // that the line contains a directory (or perhaps just
    //  more spaces so we need to check both cases).
    else if (strncmp(line, "cd ", 3) == 0)
    {
        ChangeDirectory(line);
        return 1;
    }
    return 0;
}

void PrintTableEntries()
{
    int bottomOfList = 1;
    if (numCommands > 10)
    {
        bottomOfList = numCommands - 9;
    }

    for (int i = 0; i < 10; i++)
    {
        if (history[i] == NULL)
        {
            printf("%d:\n", bottomOfList);
        }
        else
        {
            printf("%d: %s\n", bottomOfList, history[i]);
        }
        bottomOfList++;
    }
}

int ExecuteBasicCommand(char *command)
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
            perror(line);
        }
    }
    else
    {
        pid_t pid = waitpid(id, NULL, 0);
        free(tokens);
        numCommands++;
        AddToHistory(line);
    }
}

int SpecialCommandsCheck(char *line)
{
    int CdExecuted = CdCheck(line);
    int HistoryExecuted = HistoryCheck(line);
    if (CdExecuted || HistoryExecuted)
    {
        return 1;
    }
    return 0;
}

void ExecuteFullCommand(char *line)
{
    int alreadyExecuted = SpecialCommandsCheck(line);
    if (!alreadyExecuted)
    {
        ExecuteBasicCommand(line);
    }
}

void ExecuteHistoryCommand(char *line)
{
    char **tokens = malloc(100 * sizeof(char *));
    tokens[0] = strtok(line, " ");
    tokens[1] = strtok(NULL, " ");
    tokens[2] = strtok(NULL, " ");
    int historyNumber = atoi(tokens[1]);
    if (tokens[2] != NULL)
    {
        perror("Two many arguments");
    }
    else
    {
        int bottomOfList = 1;
        if (numCommands > 10)
        {
            bottomOfList = numCommands - 9;
        }
        if ((historyNumber >= bottomOfList) && (historyNumber <= numCommands))
        {
            int historyIndex = historyNumber - bottomOfList;
            char *currentCommand = malloc(100 * sizeof(char));
            currentCommand = history[historyIndex];
            ExecuteFullCommand(currentCommand);
        }
    }
    free(tokens);
}

int HistoryCheck(char *line)
{
    if ((strcmp(line, "history") == 0) || (strcmp(line, "h") == 0))
    {
        AddToHistory(line);
        numCommands++;
        PrintTableEntries();
        return 1;
    }
    else if ((strncmp(line, "history ", 8) == 0) || (strncmp(line, "h ", 2) == 0))
    {
        ExecuteHistoryCommand(line);
        return 1;
    }
    return 0;
}

char *InputProcessor()
{
    char *line = malloc(100 * sizeof(char));
    size_t size = 0;
    size_t characters = getline(&line, &size, stdin);
    char *shortLine = malloc(100 * sizeof(char));
    int index = CommandIndex(line, 0);
    GetCommand(line, shortLine, index);
    shortLine[strlen(shortLine) - 1] = '\0';
    return shortLine;
}
