#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/sysmacros.h>

#include "findstuff.h"


// int main() {
//     char input[256];
//     strcpy(input, "find child.c");
//     find5(input, strlen(input), );
// }

// void writeToPipe(char *paths) {

//     enter_critical_section(process_id);

//     // Critical section: Only one process can be here at a time
//     printf("Process %d is in the critical section\n", process_id);
//     usleep(1000000);  // Simulating some work in the critical section

//     write(fd[1], paths, strlen(paths));

//     exit_critical_section(process_id);

//     // Remaining section: Other processes can access concurrently
//     printf("Process %d is in the remaining section\n", process_id);
//     usleep(1000000);  // Simulating some work in the remaining section
    
// }

void find1(char input[256], int inLength, char *flagF, char *flagS) {
    char paths[1000];
    memset(paths, 0, 1000);
    int found = 0;

    /* malloc for the file ending and store in char* */
    char *fileEnd = malloc((inLength - 3) - (flagF - input + 3));
    strncpy(fileEnd, flagF + 3, (inLength - 3) - (flagF - input + 4));
    // fprintf(stderr, "file ending: %s\n", fileEnd);

    /* malloc for the text and store in char* */
    char *text = malloc((flagF - input - 2) - (6));
    strncpy(text, input + 6, (flagF - input - 2) - (6));
    // fprintf(stderr, "text: %s\n", text);

    found = recursive1(paths, text, fileEnd, ".", found, 1);

    /* if nothing was found, write that to pipe */
    if (found == 0) {
        char pid[6];
        sprintf(pid, "%d", getpid());
        strcat(paths, pid);
        strcat(paths, "\"");
        strcat(paths, text);
        strcat(paths, "\"");
        strcat(paths, " was not found in any files ending in ");
        strcat(paths, fileEnd);
        strcat(paths, " in this directory or subdirectories\n");
    }

    /* send signal when done writing something */
    strcat(paths, "");
    write(fd[1], paths, strlen(paths));
    // writeToPipe(paths);
    // fprintf(stderr, "Written to pipe = %s\n", paths);
    kill(getppid(), SIGUSR1);

    /* clean up */
    free(fileEnd);
    free(text);
}

int recursive1(char *paths, char *text, char *fileEnd, char *direct, int found, int option) {
    DIR* dir;
    struct dirent* entry;
    char *buffer;
    char path[BUFFERSIZE];
    strcat(direct, "");
    int numCompare;
    char subEntry[10];

    /* open the directory */
    dir = opendir(direct);
    if (dir == NULL) {
        perror("opendir failed");
        exit(EXIT_FAILURE);
    }
    
    /* read the directory entries */
    while ((entry = readdir(dir)) != NULL) {
        /* only check entries excluding cwd, parent, and entries longer than fileEnd */
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strlen(entry->d_name) >= strlen(fileEnd)) {
            
            /* create path with current entry */
            memset(path, 0, BUFFERSIZE);
            strcat(path, direct);
            strcat(path, "/");
            strcat(path, entry->d_name);

            strcpy(subEntry, entry->d_name + (strlen(entry->d_name) - strlen(fileEnd)));

            /* if end of entry name matches fileEnd, search for text in that file */
            if (strcmp(subEntry, fileEnd) == 0) {
                
                /* if entry is file */
                if (opendir(path) == NULL) {
                    // fprintf(stderr, "path passed in to fileBuffer: %s\n", path);
                    buffer = fileToBuffer(path);

                    /* if word exists in the buffer, add path to paths */
                    if (strstr(buffer, text) != NULL) {
                        if (found == 0) {
                            char pid[6];
                            sprintf(pid, "%d", getpid());
                            strcat(paths, pid);
                            strcat(paths, "\"");
                            strcat(paths, text);
                            strcat(paths, "\"");
                            strcat(paths, " found in: \n");
                            found = 1;
                        }
                        char cwd[BUFFERSIZE];
                        getcwd(cwd, BUFFERSIZE);
                        strcat(paths, cwd);
                        strcat(paths, "/");
                        if (direct == ".") {
                            strcat(paths, entry->d_name);
                        } else {
                            strcat(paths, path);
                        }
                        strcat(paths, "\n");
                    }
                    free(buffer);
                }

            /* if called from 1 and if entry is a directory, call again */
            } else if (option == 1 && opendir(path) != NULL) {
                // fprintf(stderr, "PATH DIRECTORY! \n");
                found = recursive1(paths, text, fileEnd, path, found, 1);
            }
        }
    }
    closedir(dir);

    return found;
}

/* if user enters 'find <"text"> -f:txt', searches text in files ending in txt in cwd */
void find2(char input[256], int inLength, char *flagF) {
    char paths[1000];
    memset(paths, 0, 1000);
    int found = 0;

    /* malloc for the file ending and store in char* */
    char *fileEnd = malloc((inLength) - (flagF - input + 3));
    strncpy(fileEnd, flagF + 3, (inLength) - (flagF - input + 4));
    // fprintf(stderr, "fileEnd: %s\n", fileEnd);

    /* malloc for the text and store in char* */
    char *text = malloc((flagF - input - 2) - (6));
    strncpy(text, input + 6, (flagF - input - 2) - (6));
    // fprintf(stderr, "text: %s\n", text);

    found = recursive1(paths, text, fileEnd, ".", found, 2);

    /* if nothing was found, write that to pipe */
    if (found == 0) {
        char pid[6];
        sprintf(pid, "%d", getpid());
        strcat(paths, pid);
        strcat(paths, "\"");
        strcat(paths, text);
        strcat(paths, "\"");
        strcat(paths, " was not found in any files ending in ");
        strcat(paths, fileEnd);
        strcat(paths, " in this directory\n");
    }

    /* send signal when done writing something */
    strcat(paths, "");
    write(fd[1], paths, strlen(paths));
    // writeToPipe(paths);
    // fprintf(stderr, "Written to pipe = %s\n", paths);
    kill(getppid(), SIGUSR1);

    /* clean up */
    free(fileEnd);
    free(text);
}

/* if user enters 'find <"text"> -s', searches text in files in all directories + subdirectories */
void find3(char input[256], int inLength, char *flagS) {
    char paths[1000];
    memset(paths, 0, 1000);
    int found = 0;
    
    /* malloc for the text and store in char* */
    char *text = malloc((flagS - input - 2) - (6));
    strncpy(text, input + 6, (flagS - input - 2) - (6));
    // fprintf(stderr, "text: %s\n", text);

    found = recursive3(paths, text, ".", 0, 3);

    /* if nothing was found, write that to pipe */
    if (found == 0) {
        char pid[6];
        sprintf(pid, "%d", getpid());
        strcat(paths, pid);
        strcat(paths, "\"");
        strcat(paths, text);
        strcat(paths, "\"");
        strcat(paths, " was not found in any files in this directory or subdirectories\n");
    }

    /* send signal when done writing something */
    strcat(paths, "");
    write(fd[1], paths, strlen(paths));
    // writeToPipe(paths);
    // fprintf(stderr, "Written to pipe = %s\n", paths);
    kill(getppid(), SIGUSR1);

    /* Clean up */
    free(text);           
}

int recursive3(char *paths, char *text, char *direct, int found, int option) {
    DIR* dir;
    struct dirent* entry;
    char *buffer;
    char path[BUFFERSIZE];
    strcat(direct, "");

    /* open the directory */
    dir = opendir(direct);
    if (dir == NULL) {
        perror("opendir failed");
        exit(EXIT_FAILURE);
    }
    
    /* read the directory entries */
    while ((entry = readdir(dir)) != NULL) {
        /* only check entries excluding cwd and parent */
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            memset(path, 0, BUFFERSIZE);
            strcat(path, direct);
            strcat(path, "/");
            strcat(path, entry->d_name);
            // fprintf(stderr, "path passed in to fileBuffer: %s\n", path);

            /* if called from 3 and if entry is a directory, call again */
            if (option == 3 && opendir(path) != NULL) {
                // fprintf(stderr, "PATH DIRECTORY! \n");
                found = recursive3(paths, text, path, found, 3);
            
            /* else, entry is file */
            } else if (opendir(path) == NULL) {
                buffer = fileToBuffer(path);

                /* if word exists in the buffer, add path to paths */
                if (strstr(buffer, text) != NULL) {
                    if (found == 0) {
                        char pid[6];
                        sprintf(pid, "%d", getpid());
                        strcat(paths, pid);
                        strcat(paths, "\"");
                        strcat(paths, text);
                        strcat(paths, "\"");
                        strcat(paths, " found in: \n");
                        found = 1;
                    }
                    char cwd[BUFFERSIZE];
                    getcwd(cwd, BUFFERSIZE);
                    strcat(paths, cwd);
                    strcat(paths, "/");
                    if (direct == ".") {
                        strcat(paths, entry->d_name);
                    } else {
                        strcat(paths, path);
                    }
                    strcat(paths, "\n");
                    
                }
                free(buffer);
            }
        }
    }
    closedir(dir);

    return found;
}



/* if user enters 'find <"text">', searches text in files in cwd */
void find4(char input[256], int inLength) {
    char paths[1000];
    memset(paths, 0, 1000);
    int found = 0;
    
    /* malloc for the text and store in char* */
    char *text = malloc(inLength - 5);
    strncpy(text, input + 6, inLength - (8));
    // fprintf(stderr, "text: %s\n", text);

    found = recursive3(paths, text, ".", found, 4);

    /* if nothing was found, write that to pipe */
    if (found == 0) {
        char pid[6];
        sprintf(pid, "%d", getpid());
        strcat(paths, pid);
        strcat(paths, "\"");
        strcat(paths, text);
        strcat(paths, "\"");
        strcat(paths, " was not found in any files in this directory\n");
    }

    /* send signal when done writing something */
    strcat(paths, "");
    write(fd[1], paths, strlen(paths));
    // writeToPipe(paths);
    // fprintf(stderr, "Written to pipe = %s\n", paths);
    kill(getppid(), SIGUSR1);

    /* Clean up */
    free(text);
}

/* reads entire contents of file into a string buffer 
returns buffer if success, NULL if not*/
char* fileToBuffer(char* filename) {
    struct stat sb;
    FILE *file;
    long fileSize;
    char *buffer;

    file = fopen(filename, "rb");

    if (file == NULL) {
        perror("couldn't open: ");
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    /* if file was able to be opened, read into malloced buffer */
    } else {
        lstat(filename, &sb);
        // fprintf(stderr, "filename: %s, fileSize: %ld\n", filename, sb.st_size);

        // fseek(file, 0, SEEK_END);
        // fileSize = ftell(file);
        // rewind(file);
        // fprintf(stderr, "filename: %s, fileSize %ld\n", filename, fileSize);

        fileSize = sb.st_size;

        /* malloc */
        buffer = (char *)malloc(fileSize + 1);
        if (buffer == NULL) {
            fprintf(stderr, "Memory allocation failed: Error code %d\n", errno);
            perror("Malloc for buffer failed");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        /* read all of file into buffer */
        if (fread(buffer, 1, fileSize, file) != fileSize) {
            perror("failed to read file.");
            fclose(file);
            free(buffer);
            return NULL;
        }
        /* add null char at end */
        buffer[fileSize] = '\0';
        fclose(file);
        return buffer;
    }
}


/* if user enters 'find <filename> -s', find file(s) in all directories + subdirectories */
void find5(char input[256], int inLength, char *flagS) {
    int wrote = 0;
    char paths[1000];
    memset(paths, 0, 1000);
    
    /* malloc for the filename and store in char* */
    char *filename = malloc((flagS - input - 1) - (5));
    strncpy(filename, input + 5, (flagS - input - 1) - (5));
    // fprintf(stderr, "filename: %s\n", filename);

    wrote = recursive5(paths, filename, ".", 0, 5);

    free(filename);

    /* if nothing written in path, write nothing found */
    if (wrote == 0) {
        char pid[6];
        sprintf(pid, "%d", getpid());
        strcat(paths, pid);
        strcat(paths, "No such file or directory in this directory or subdirectories\n");
    }

    /* send signal when done writing something */
    strcat(paths, "");
    write(fd[1], paths, strlen(paths));
    // writeToPipe(paths);
    // fprintf(stderr, "Written to pipe = %s\n", paths);
    kill(getppid(), SIGUSR1);
}


int recursive5(char *paths, char *filename, char *direct, int wrote, int option) {
    DIR* dir;
    struct dirent* entry;
    char path[BUFFERSIZE];

    /* open the directory */
    dir = opendir(direct);
    if (dir == NULL) {
        perror("opendir failed");
        exit(EXIT_FAILURE);
    }
    /* read the directory entries */
    while ((entry = readdir(dir)) != NULL) {
        /* only check paths excluding cwd and parent */
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            memset(path, 0, BUFFERSIZE);
            strcat(path, direct);
            strcat(path, "/");
            strcat(path, entry->d_name);
            
            /* if path is a directory, call findFile there too if called from find5 */
            if (opendir(path) != NULL && option == 5) {
                // fprintf(stderr, "PATH DIRECTORY! : %s\n", path);
                wrote = recursive5(paths, filename, path, wrote, 5);
            } 

            /* look through entries and check if they equal filename */
            if (strcmp(entry->d_name, filename) == 0) {
                if (wrote == 0) {
                    char pid[6];
                    sprintf(pid, "%d", getpid());
                    strcat(paths, pid);
                    strcat(paths, "file found in: \n");
                    wrote = 1;
                }
                char cwd[BUFFERSIZE];
                getcwd(cwd, BUFFERSIZE);
                strcat(paths, cwd);
                strcat(paths, "/");
                if (direct == ".") {
                    strcat(paths, entry->d_name);
                } else {
                    strcat(paths, path);
                }
                strcat(paths, "\n");
            }
        }
    }

    /* Close the directory */
    closedir(dir);
    
    return wrote;
}


/* if user enters 'find <filename>', find file in cwd */
void find6(char input[256], int inLength) {
    char path[100];
    memset(path, 0, 100);
    int wrote;
    
    /* malloc for the filename and store in char* */
    char *filename = malloc(inLength - 5);
    strncpy(filename, input + 5, inLength - (6));
    // fprintf(stderr, "filename: %s\n", filename);

    // findFile(filename, 6, path, 0);
    wrote = recursive5(path, filename, ".", 0, 6);
    free(filename);

    /* if nothing written in path, write nothing found */
    if (wrote == 0) {
        char pid[6];
        sprintf(pid, "%d", getpid());
        strcat(path, pid);
        strcat(path, "No such file or directory\n");
    }

    /* send signal when done writing something */
    write(fd[1], path, strlen(path));
    // writeToPipe(path);
    kill(getppid(), SIGUSR1);
}

