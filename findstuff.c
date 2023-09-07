#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/sysmacros.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>

#include "findstuff.h"

/* global vars */
int fd[2];
int redirected = 0;

void formatTime(long elapsedTime, char *FormattedTime) {
    int ms = elapsedTime % 1000;
    int secs = (elapsedTime / 1000) % 60;
    int mins = (elapsedTime / (1000 * 60)) % 60;
    int hours = (elapsedTime / (1000 * 60 * 60)) % 24;

    sprintf(FormattedTime, "%02d:%02d:%02d:%02d", hours, mins, secs, ms);
}

void redirect(int i) {
    /* standard input will be seen as fd[0], so it will read from there instead of keyboard */
    dup2(fd[0], STDIN_FILENO);
    // fprintf(stderr, "redirected\n");
    redirected = 1;
}

int main() {
    DIR* dir;
    int pid;
    struct dirent *entry;
    int save = dup(STDIN_FILENO);
    
    char cwd[BUFFERSIZE];
    char input[BUFFERSIZE];

    printf("Parent PID is: %d\n", getpid());

    /* create head of linked list */
    child *head = NULL;
    child *tail = NULL;
    int childCount = 0;

    /* create pipe */
    pipe(fd);

    int inLength;
    char *flagF;
    char *flagS;


    /* infinitely loops */
    for (;;) {
        int childPid = 0;
        int status;
        char pidStr[6];
        int pidNum;

        /* clear the buffer every iteration */
        memset(input, 0, BUFFERSIZE);

        /* print the current working folder to scanf line */
        fprintf(stderr, "\033[1;34m");
        fprintf(stderr, "findstuff$ ");
        fprintf(stderr, "\033[0m");

        /* Reading from Pipe */
        signal(SIGUSR1, redirect);
        read(STDIN_FILENO, input, BUFFERSIZE); 

        /* print from pipe only if child signaled! */
        if (redirected == 1) {
            /* retrieve PID of child that sent signal, then delete them from list */
            strncpy(pidStr, input, 5);
            strcpy(pidStr + 5, "\0");
            pidNum = atoi(pidStr);
            deletePid(&head, &tail, pidNum, childCount);
            childCount--;
            
            /* print the results from child */
            fprintf(stderr, "%s\n", input + 5);

        }  
        redirected = 0;
        
        /* redirect back to keyboard*/
        dup2(save, STDIN_FILENO);

        
        /* if user enters 'list', lists all running child processes and what they try to do */
        if (strcmp(input, "list\n") == 0) {
            fprintf(stderr, "inside list -- childCount = %d\n", childCount);

            if (childCount != 0) {
                int i;
                child *current = head;

                for (i = 0; i < childCount; i++) {
                    fprintf(stderr, "Child %d\nPID: %d\nTask: %s\n", current->serialNum, current->pid, current->task);
                    if (current->next != NULL) {
                        current = current->next;
                    }                       
                }
            } else {
                fprintf(stderr, "0 active children\n\n");
            }
            

        /* if user enters 'q' or 'quit', terminate all processes */
        } else if ((strcmp(input, "q\n") == 0) || (strcmp(input, "quit\n") == 0)) {
            int i;
            child *current = head;

            for (i = 0; i < childCount; i++) {
                // fprintf(stderr, "deleted %d\n", i);
                child *next = current->next;
                deleteChild(&head, &tail, current->serialNum, childCount - i);
                current = next;
            }
            childCount = 0;
            exit(EXIT_SUCCESS);

        /* if user enters 'kill <num>', kill that child process */
        } else if (strncmp(input, "kill", 4) == 0) {
            int childNum = input[5] - '0';
            fprintf(stderr, "killing child %d\n", childNum);
            if (childCount > 0) {
                fprintf(stderr, "childCount before delete: %d\n", childCount);
                int success = deleteChild(&head, &tail, childNum, childCount);
                if (success == 0) {
                    childCount--;
                    fprintf(stderr, "Child %d was successfully deleted \n\n", childNum);
                } else if (success == -1) {
                    fprintf(stderr, "Child %d was not found \n\n", childNum);
                }
                
            } else {
                fprintf(stderr, "No children to be deleted! \n\n");
            }
            

        /* if starts with find, fork a child because it will be a find input */
        } else if (strncmp(input, "find\n", 4) == 0) {
            if (childCount < 10) {
                pid = fork();
            }

            /************************** parent ******************************/
            if (pid > 0) {      
                printf("Child PID is: %d\n", pid);

                /* if less than 10 active children, malloc new child and add to linked list */
                if (childCount < 10) {
                    child *newChild = createChild(pid, childCount + 1, input);
                    childCount++;
                    insertChild(&head, &tail, newChild);
                } else {
                    fprintf(stderr, "Cannot exceed 10 active children.\n");
                }

            }


            /************************** child ******************************/
            else if (pid == 0) {
                struct timeval startTime, endTime;
                long elapsedTime;
                char formattedTime[20];
                gettimeofday(&startTime, NULL);

                close(fd[0]);
                inLength = strlen(input);
                flagF = strstr(input, "-f");
                flagS = strstr(input, "-s");

                fprintf(stderr, "In child. input: %s\n", input);

                // fprintf(stderr, "sleepig now\n");
                // sleep(20);

                /* if user enters 'find <"text"> -f:txt -s', searches text in files ending in txt in all directories + subdirectories */
                if ((flagF != NULL) && (flagS != NULL) && (strncmp(input, "find \"", 6) == 0)) {
                    // fprintf(stderr, "made it in 1\n");
                    find1(input, inLength, flagF, flagS);

                /* if user enters 'find <"text"> -f:txt', searches text in files ending in txt in cwd */
                } else if ((flagS == NULL) && (flagF != NULL) && (strncmp(input, "find \"", 6) == 0)) {
                    // fprintf(stderr, "made it in 2\n");
                    find2(input, inLength, flagF);

                /* if user enters 'find <"text"> -s', searches text in files in all directories + subdirectories */
                } else if ((flagS != NULL) && (flagF == NULL) && (strncmp(input, "find \"", 6) == 0)) {
                    // fprintf(stderr, "made it in 3\n");
                    find3(input, inLength, flagS);    

                /* if user enters 'find <"text">', searches text in files in cwd */
                } else if ((flagS == NULL) && (flagF == NULL) && (strncmp(input, "find \"", 6) == 0)) {
                    // fprintf(stderr, "made it in 4\n")
                    find4(input, inLength);

                /* if user enters 'find <filename> -s', find file in all directories + subdirectories */
                } else if ((strncmp(input, "find", 4) == 0) && (flagS != NULL)) {
                    // fprintf(stderr, "made it in 5\n");
                    find5(input, strlen(input), flagS);

                /* if user enters 'find <filename>', find file in cwd */
                } else if (strncmp(input, "find", 4) == 0) {
                    // fprintf(stderr, "made it in 6\n");
                    find6(input, strlen(input));
                }

                /***************************** CLOSES PIPE WRITING SIDE **********************************/
                close(fd[1]);

                gettimeofday(&endTime, NULL);
                elapsedTime = (endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_usec - startTime.tv_usec) / 1000;
                formatTime(elapsedTime, formattedTime);
                fprintf(stderr, "Search took: %s\n", formattedTime);

                return 0;
            }
        }
    }
    /***************************** CLOSES PIPE READING SIDE **********************************/
    close(fd[0]);
    close(fd[1]);    
}    
