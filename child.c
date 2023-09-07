#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "findstuff.h"

// int main() {
//     child *head = NULL;
//     child *tail = NULL;


//     child *newChild1 = createChild(1, 1, "find hello1");
//     insertChild(&head, &tail, newChild1);
//     child *newChild2 = createChild(2, 2, "find hello2");
//     insertChild(&head, &tail, newChild2);
//     child *newChild3 = createChild(3, 3, "find hello3");
//     insertChild(&head, &tail, newChild3);


//     child *current = head;
//     for (int i = 0; i < 3; i++) {
//         fprintf(stderr, "Child %d\nPID: %d\nTask: %s\n", current->serialNum, current->pid, current->task);
//         if (current->next != NULL)
//             current = current->next;
//         fprintf(stderr, "\n");
//     }

//     deleteChild(&head, &tail, 1, 3);
//     deleteChild(&head, &tail, 2, 2);
//     deleteChild(&head, &tail, 3, 1);

//     fprintf(stderr, "*********************\n");

//     current = head;
//     for (int i = 0; i < 2; i++) {
//         fprintf(stderr, "Child %d\nPID: %d\nTask: %s\n", current->serialNum, current->pid, current->task);
//         if (current->next != NULL)
//             current = current->next;
//         fprintf(stderr, "\n");
//     }


    
// }


child* createChild(int pid, int serialNum, char *task) {
    child *newChild = malloc(sizeof(child));
    if (newChild == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    newChild->pid = pid;
    newChild->serialNum = serialNum;    
    strcpy(newChild->task, task);

    // fprintf(stderr, "Task: %s\n", *newChild->task);
    return newChild;
}

void insertChild(child **head, child **tail, child *newChild) {
    int i;

    /* if list empty */
    if (*head == NULL) {
        // fprintf(stderr, "empty list\n");
        *head = newChild;
        *tail = newChild;
        newChild->next = NULL;

    /* else if list is not empty */
    } else {
        (*tail)->next = newChild;
        *tail = newChild;
        newChild->next = NULL;
    }
    // fprintf(stderr, "addy: %p\n", head);
    
    return;
}

/* returns 0 if successful, -1 if not */
int deleteChild(child **head, child **tail, int childNum, int childCount) {
    int i;

    /* if deleting head child in list */
    if ((*head)->serialNum == childNum) {
        kill((*head)->pid, SIGKILL);
        child *temp = *head;
        *head = (*head)->next;
        free(temp);
        return 0;
    }

    child *current = *head;
    for (i = 0; i < childCount - 1; i++) {
        if (current->next->serialNum == childNum) {
            kill(current->next->pid, SIGKILL);
            child *temp = current->next;
            current->next = current->next->next;
            free(temp);
            /* if deleting last child in list */
            if (current->next == NULL) {
                *tail = current;
            }
            return 0;
        }
        current = current->next;
    }
    return -1;
}

void deletePid(child **head, child **tail, int pid, int childCount) {
    int i;

    kill(pid, SIGKILL);

    /* if deleting head child in list */
    if ((*head)->pid == pid) {
        child* temp = *head;
        *head = (*head)->next;
        free(temp);
        return;
    }

    child *current = *head;
    for (i = 0; i < childCount - 1; i++) {
        if (current->next->pid == pid) {
            child *temp = current->next;
            current->next = current->next->next;
            free(temp);
            /* if deleting last child in list */
            if (current->next == NULL)
                *tail = current;
            return;
        }
    }
}