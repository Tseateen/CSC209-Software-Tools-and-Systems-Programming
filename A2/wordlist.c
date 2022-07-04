#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordlist.h"


/* Read the words from a filename and return a linked list of the words.
 *   - The newline character at the end of the line must be removed from
 *     the word stored in the node.
 *   - You an assume you are working with Linux line endings ("\n").  You are
 *     welcome to also handle Window line endings ("\r\n"), but you are not
 *     required to, and we will test your code on files with Linux line endings.
 *   - The time complexity of adding one word to this list must be O(1)
 *     which means the linked list will have the words in reverse order
 *     compared to the order of the words in the file.
 *   - Do proper error checking of fopen, fclose, fgets
 */
struct node *read_list(char *filename) {
    FILE *f = fopen(filename, "r");
    struct node *nodes = malloc(sizeof(struct node));
    nodes -> next = NULL;
    if (f == NULL) {  
        return nodes; 
    } 
    char s[SIZE];
    if (fscanf(f, "%s", s) == 1) {
        strncpy(nodes -> word, s, 5);
        *((nodes -> word) + 5) = '\0';
    }
    struct node *curr_node = nodes;
    while (fscanf(f, "%s", s) == 1) {
        struct node *next_node = malloc(sizeof(struct node));
        next_node -> next = curr_node;
        strncpy(next_node -> word, s, 5);
        *((next_node -> word) + 5) = '\0';
        curr_node = next_node;
    }
    return curr_node;
}

/* Print the words in the linked-list list one per line
 */
void print_dictionary(struct node *list) {
    struct node *curr = list;
    if (!(curr -> next)) {
        printf("%s\n",curr -> word);
        return;
    }
    while (curr-> next) {
        printf("%s\n", curr -> word);
        curr = (curr -> next);
    }
    printf("%s\n",curr -> word);
}
/* Free all of the dynamically allocated memory in the dictionary list 
 */
void free_dictionary(struct node *list) {
    if (!(list -> next)) {
        free(list);
        return;
    }
    while (list-> next) {
        struct node *next = list->next;
        free(list);
        list = next;
    }
    free(list);
}
