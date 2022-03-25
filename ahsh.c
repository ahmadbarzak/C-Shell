#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

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
        char delim[] = " ";
        char *tokens[100];
        int i = 0;
        tokens[i] = strtok(line, delim);
        while(tokens[i] != NULL)
        {
            i++;
            tokens[i] = strtok(NULL, delim);
        }
        if (!strcmp(tokens[0], "exit"))
        {
            return 0;
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
    }
    return 0;
}
