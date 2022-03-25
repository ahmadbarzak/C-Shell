#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>



int containsDirectory(char* line, char* path)
{
    int k = 4;
    int hasCommand = 0;
    while(line[k] != '\0')
    {
        if(line[k] != ' ')
        {
            return 1;
        }
        k++;
    }
    return 0;
}

void changeDirectory(char* line, char* path)
{
    int j = 3;
    int hasCommand = containsDirectory(line, path);
    if (hasCommand == 1)
    {
        while(line[j] != '\0')
        {
            path[j-3] = line[j];
            j++;
        }
        chdir(path);
    }
    else
    {
        chdir("/home");
    }
}



int main(int argc, char *argv[]){
    
    printf("Welcome to ahsh Shell!\n");
    printf("--------------------------\n\n");
    while(1)
    {
        printf("ahsh>  ");
        char *line = NULL;
        size_t size;
        size_t characters = getline(&line, &size, stdin);
        line[strlen(line) - 1] = '\0';
        char *path = malloc(100*sizeof(char));
        if(strcmp(line, "cd") == 0)
        {
            chdir("/home");
        }
        else if (strncmp(line, "cd ", 3) == 0)
        {
            changeDirectory(line, path);
        }
        else
        {
            char delim[] = " ";
            char **tokens = malloc(100*sizeof(char*));
            int i = 0;
            tokens[i] = strtok(line, delim);
            while(tokens[i] != NULL)
            {
                i++;
                tokens[i] = strtok(NULL, delim);
            }
            pid_t id = fork();
            if (id == 0)
            {
                execvp(tokens[0], tokens);
            }
            else
            {
                pid_t pid = waitpid(id, NULL, 0);
            }
            free(tokens);
        }
        free(line);
        free(path);
    }
    return 0;
}

