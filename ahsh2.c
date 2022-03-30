#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct Job
{
    int jobNum;
    pid_t jobId;
    char *status;
    char *commandLine;
} Job;

// Global Variables:

int jobNumber = 1;
int numCommands = 0;
char *currentLine;
int numHistoryCommands = 10;
int maxCharacters = 100;
int isBackgroundTask = 0;
int maxJobs = 500;
int jobIndex = 0;
int numJobs = 0;
struct Job *jobs;
char **history;

// Prototype Declarations:
int JobsCheck(char *line);
void jobChecker();
void AddToHistory(char *line);
void BackgroundProcessCheck(char *line);
int CdCheck(char *line);
void ChangeDirectory(char *line);
int CommandIndex(char *line, int numChars, char delim, int equal);
int ExecuteBasicCommand(char *command, int addHistory);
void ExecuteFullCommand(char *processedLine, char *history);
void ExecuteHistoryCommand(char *line);
char *GetCommand(char *line, int numChars);
char *GetStatus(pid_t id);
int HistoryCheck(char *line);
char **InputProcessor(char *line);
char *LineOutput();
void PipeProcess(char **pipeTokens, char *history);
void PrintTableEntries();
int SpecialCommandsCheck(char *line);
char **Tokeniser(char *line, char *delim);

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
    jobs = malloc(maxJobs * sizeof(int));
    // This part does not repeat, hence outside of while loop.
    while (1)
    {
        printf("ahsh>  ");

        char *inputLine = LineOutput();
        char *processedLine = malloc(maxCharacters * sizeof(char));
        memcpy(processedLine, inputLine, maxCharacters * sizeof(char));
        BackgroundProcessCheck(processedLine);
        // every iteration of the while loop should represent the following:

        // An input is taken in and processed into executable commands.
        // if there are more than one value in this string array, that means that the first command is
        // piped into the second, then piped into the third etc.

        // undergo a method that handles the piping for each command
        // to do multiple pipes this will be a recursive process.

        // ExecuteBasicCommand(processedLine, 1);
        ExecuteFullCommand(processedLine, inputLine);
        jobChecker();
        
    }
    return 0;
}

void AddToHistory(char *line)
{
    int index = CommandIndex(line, 0, ' ', 0);
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

void BackgroundProcessCheck(char *line)
{
    char **tokens = malloc(maxCharacters * sizeof(char *));
    tokens = Tokeniser(line, " ");
    int i = 0;
    while (tokens[i] != NULL)
    {
        i++;
    }
    char *lastToken = malloc(maxCharacters * sizeof(char));
    memcpy(lastToken, tokens[i - 1], maxCharacters * sizeof(char));
    if (strcmp(lastToken, "&") == 0)
    {
        isBackgroundTask = 1;
        int ampLocation = 0;
        while (line[ampLocation] != '&')
        {
            ampLocation++;
        }
        line[ampLocation] = '\0';
    }
    else
    {
        isBackgroundTask = 0;
    }
    free(lastToken);
    free(tokens);
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

void ChangeDirectory(char *line)
{
    int j = 3;
    // check if the current line contains a directory for
    // cd to take the user to, otherwise if none is inputted,
    // cd should take the user to the home directory.
    int numChars = CommandIndex(line, 3, ' ', 0);
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

int CommandIndex(char *line, int numChars, char delim, int equal)
{ // if we are in this function, then the line must
    // currently have "cd " as the first three characters.
    // we then check if there are any characters following this,
    // otherwise if everything after is ' '.
    while (line[numChars] != '\0')
    {
        if (!equal && (line[numChars] != delim))
        {
            return numChars;
        }
        else if (equal && (line[numChars] == delim))
        {
            return numChars;
        }

        numChars++;
    }
    return 0;
}

void ExecuteFullCommand(char *processedLine, char *history)
{
    int alreadyExecuted = SpecialCommandsCheck(processedLine);
    if (!alreadyExecuted)
    {

        char **pipeTokens = InputProcessor(processedLine);
        PipeProcess(pipeTokens, history);
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
            // addToHistory(currentCommand);
            ExecuteFullCommand(currentCommand, currentCommand);
        }
    }
    free(tokens);
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

char *GetStatus(pid_t id)
{

    char idString[100];
    sprintf(idString, "%d", id);
    char StatusString[20];
    char pidPath[100] = "/proc/";
    strcat(pidPath, idString);
    strcat(pidPath, "/status");

    FILE *fp = fopen(pidPath, "r");
    char *currentLine = malloc(maxCharacters * sizeof(char));
    if (fp == NULL)
    {
        perror("cannot open file");
    }

    char *statusLine = malloc(maxCharacters * sizeof(char));

    for (int i = 0; i < 3; i++)
    {
        statusLine = fgets(currentLine, maxCharacters * sizeof(char), fp);
    }
    int index = CommandIndex(statusLine, 1, '\t', 1);
    int j = 0;
    char *status = malloc(maxCharacters * sizeof(char));

    while (statusLine[index + 1] != ' ')
    {
        status[j] = statusLine[index + 1];
        j++;
        index++;
    }

    return status;
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

char **InputProcessor(char *line)
{
    char **pipeTokens = Tokeniser(line, "|");
    int i = 0;
    while (pipeTokens[i] != NULL)
    {
        int index = CommandIndex(pipeTokens[i], 0, ' ', 0);
        pipeTokens[i] = GetCommand(pipeTokens[i], index);
        i++;
    }
    return pipeTokens;
}

char *LineOutput()
{
    size_t size = 0;
    getline(&currentLine, &size, stdin);
    currentLine[strlen(currentLine) - 1] = '\0';
    char *processedLine = malloc(maxCharacters * sizeof(char));
    memcpy(processedLine, currentLine, maxCharacters * sizeof(char));
    return processedLine;
}

void PipeProcess(char **pipeTokens, char *history)
{

    int *fd = malloc(2 * sizeof(int));
    pid_t id;
    int fdId = 0;
    int i = 0;
    while (pipeTokens[i] != NULL)
    {
        pipe(fd);
        id = fork();
        if (id == -1)
        {
            perror("fork error");
            exit(EXIT_FAILURE);
        }
        else if (id == 0)
        {
            dup2(fdId, STDIN_FILENO);
            if (pipeTokens[i + 1] != NULL)
            {
                dup2(fd[1], STDOUT_FILENO);
            }

            close(fd[0]);

            char **tokens = Tokeniser(pipeTokens[i], " ");

            if (execvp(tokens[0], tokens) == -1)
            {
                perror("executing problem");
            }
        }
        else
        {
            if (!isBackgroundTask)
            {
                pid_t pid = waitpid(id, NULL, 0);
            }
            close(fd[1]);
            fdId = fd[0];
            i++;
        }
    }
    // done once
    if (isBackgroundTask)
    {
        char *status = GetStatus(id);

        struct Job currentJob;
        currentJob.jobId = id;
        currentJob.jobNum = jobNumber;
        currentJob.status = status;
        currentJob.commandLine = currentLine;

        printf("[%d]  %d\n", jobNumber, id);

        jobs[jobIndex] = currentJob;
        numJobs++;
        jobNumber++;
        jobIndex++;
    }
    numCommands++;
    AddToHistory(history);
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

int JobsCheck(char *line)
{
    if (strcmp(line, "jobs") == 0)
    {
        int i = 0;
        while (jobs[i].jobId > 0)
        {
            printf("[%d]  <%s>  %s\n", jobs[i].jobId, jobs[i].status, jobs[i].commandLine);
            i++;
        }
        return 1;
    }
    return 0;
}

int SpecialCommandsCheck(char *line)
{
    int JobsExecuted = JobsCheck(line);
    int CdExecuted = CdCheck(line);
    int HistoryExecuted = HistoryCheck(line);
    if (JobsExecuted || CdExecuted || HistoryExecuted)
    {
        return 1;
    }
    return 0;
}

char **Tokeniser(char *line, char *delim)
{
    char *lineCopy = malloc(maxCharacters * sizeof(char));
    memcpy(lineCopy, line, maxCharacters * sizeof(char));
    char **tokens = malloc(maxCharacters * sizeof(char *));
    int i = 0;
    tokens[i] = strtok(lineCopy, delim);
    while (tokens[i] != NULL)
    {
        i++;
        tokens[i] = strtok(NULL, delim);
    }
    return tokens;
}

void jobChecker()
{
    int i = 0;
    while (jobs[i].jobId > 0)
    {
        Job currentJob = jobs[i];
        if (waitpid(currentJob.jobId, NULL, WNOHANG) == currentJob.jobId)
        {
            jobNumber--;
            int j = i;
            printf("<Done>  %s\n", currentJob.commandLine);
            while (jobs[j + 1].jobId > 0)
            {
                jobs[j] = jobs[j + 1];
                j++;
            }
        }
        i++;
    }
} 