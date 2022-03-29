#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

// Global Variables:
int numCommands = 0;
int numHistoryCommands = 10;
int maxCharacters = 100;
char **history;

// Prototype Declarations:
void execute(char *command);
void PipeProcess(char *line);
int CommandIndex(char *line, int numChars);
char *GetCommand(char *line, int numChars);
void AddToHistory(char *line);
void ChangeDirectory(char *line);
int CdCheck(char *line);
void PrintTableEntries();
void PrintTableEntries();
int ExecuteBasicCommand(char *command, int addHistory);
int SpecialCommandsCheck(char *line);
void ExecuteFullCommand(char *line);
void ExecuteHistoryCommand(char *line);
int HistoryCheck(char *line);
char **InputProcessor(char *line);
char *LineOutput();
// we will use read()
// we will use write()

// we want the output of the first command ls
// we will use write(s)
// to become the input of the second command wc
int main(int argc, char *argv[])
{

    // Introduce user to Ahsh Shell
    printf("Welcome to ahsh Shell!\n");
    printf("--------------------------\n\n");
    history = malloc(numHistoryCommands * sizeof(char *));
    // This part does not repeat, hence outside of while loop.
    while (1)
    {
        printf("ahsh>  ");

        char *line = LineOutput();
        
        // every iteration of the while loop should represent the following:

        // An input is taken in and processed into executable commands.
        // if there are more than one value in this string array, that means that the first command is
        // piped into the second, then piped into the third etc.

        // undergo a method that handles the piping for each command
        // to do multiple pipes this will be a recursive process.
        ExecuteFullCommand(line);
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

char *GetCommand(char *line, int numChars)
{
    char *path = malloc(maxCharacters * sizeof(char));
    int i = numChars;
    while (line[i] != '\0')
    {
        // take the path from the line, which is the
        // characters following cd.
        path[i - numChars] = line[i];
        i++;
    }
    return path;
}

void AddToHistory(char *line)
{
    int index = CommandIndex(line, 0);
    char *pathString = GetCommand(line, index);
    int i = 0;
    while (history[i] != NULL)
    {
        i++;
    }
    if (i < numHistoryCommands)
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
        char *path = GetCommand(line, 3);

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
    if (numCommands > numHistoryCommands)
    {
        bottomOfList = numCommands - (numHistoryCommands - 1);
    }

    for (int i = 0; i < numHistoryCommands; i++)
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

int ExecuteBasicCommand(char *command, int addHistory)
{
    char *line = malloc(maxCharacters * sizeof(char));
    strcpy(line, command);
    char delim[] = " ";
    char **tokens = malloc(maxCharacters * sizeof(char *));
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
        if (addHistory)
        {
            AddToHistory(line);
        }
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
        PipeProcess(line);
    }
}

void ExecuteHistoryCommand(char *line)
{
    char **tokens = malloc(maxCharacters * sizeof(char *));
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
        if (numCommands > numHistoryCommands)
        {
            bottomOfList = numCommands - (numHistoryCommands - 1);
        }
        if ((historyNumber >= bottomOfList) && (historyNumber <= numCommands))
        {
            int historyIndex = historyNumber - bottomOfList;
            char *currentCommand = malloc(maxCharacters * sizeof(char));
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

char **Tokeniser(char *line, char *delim)
{
    char *lineCopy = malloc(maxCharacters*sizeof(char));
    memcpy(lineCopy, line, maxCharacters*sizeof(char));
    char **tokens = malloc(maxCharacters * sizeof(char *));
    int i = 0;
    tokens[i] = strtok(lineCopy, delim);
    while (tokens[i] != NULL)
    {
        i++;
        tokens[i] = strtok(NULL, delim);
    }
    return tokens;
    free(lineCopy);
}

char **InputProcessor(char *line)
{
    char **pipeTokens = Tokeniser(line, "|");
    int i = 0;
    while (pipeTokens[i] != NULL)
    {
        int index = CommandIndex(pipeTokens[i], 0);
        pipeTokens[i] = GetCommand(pipeTokens[i], index);
        i++;
    }
    return pipeTokens;
}

void PipeProcess(char *line)
{
    char **pipeTokens = InputProcessor(line);
    int *fd = malloc(2*sizeof(int));
    pid_t id;
    int fdId = 0;
    int i = 0;
    while(pipeTokens[i] != NULL)
    {
        pipe(fd);
        id = fork();
        if(id == -1)
        {
            perror("fork error");
            exit(EXIT_FAILURE);
        }
        else if (id == 0)
        {
            dup2(fdId, STDIN_FILENO);
            if (pipeTokens[i+1] != NULL)
            {
                dup2(fd[1], STDOUT_FILENO);
            }

            close(fd[0]);
            ExecuteBasicCommand(pipeTokens[i], 0);
            exit(1);
        }
        else {
            wait(NULL);
            close(fd[1]);
            fdId = fd[0];
            i++;
        }
    }
    numCommands++;
    AddToHistory(line);
}

char *LineOutput()
{
    char *line = malloc(maxCharacters * sizeof(char));
    size_t size = 0;
    size_t characters = getline(&line, &size, stdin);
    line[strlen(line) - 1] = '\0';
    return line;
}