#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordle.h"
#include "constraints.h"

/* Read the wordle grid and solution from fp. 
 * Return a pointer to a wordle struct.
 * See sample files for the format. Assume the input file has the correct
 * format.  In other words, the word on each is the correct length, the 
 * words are lower-case letters, and the line ending is either '\n' (Linux,
 * Mac, WSL) or '\r\n' (Windows)
 */
struct wordle *create_wordle(FILE *fp) {
    struct wordle *w = malloc(sizeof(struct wordle));
    char line[MAXLINE]; 
    w->num_rows = 0;

    while(fgets(line, MAXLINE, fp ) != NULL) {
        
        // remove the newline character(s) 
        char *ptr;
        if(((ptr = strchr(line, '\r')) != NULL) ||
           ((ptr = strchr(line, '\n')) != NULL)) {
            *ptr = '\0';
        }
        
        strncpy(w->grid[w->num_rows], line, SIZE);
        w->grid[w->num_rows][SIZE - 1] = '\0';
        w->num_rows++;
    }
    return w;
}


/* Create a solver_node and return it.
 * If con is not NULL, copy con into dynamically allocated space in the struct
 * If con is NULL set the new solver_node con field to NULL.
 * Tip: struct assignment makes copying con a one-line statements
 */
struct solver_node *create_solver_node(struct constraints *con, char *word) {
    struct solver_node *sn = malloc(sizeof(struct solver_node));
    if (con) {
        sn -> con = init_constraints();
        memcpy(sn -> con, con, sizeof(struct constraints));
    } 
    strncpy(sn->word, word, SIZE - 1);
    sn -> word[SIZE - 1] = '\0';
    sn -> next_sibling = NULL;
    sn -> child_list = NULL;
    return sn;
}

/* Return 1 if "word" matches the constraints in "con" for the wordle "w".
 * Return 0 if it does not match
 */
int match_constraints(char *word, struct constraints *con, 
struct wordle *w, int row) {
    
    for (int i = 0; i < strlen(word); i++) {
        if (strlen(con -> must_be[i]) > 0) {
            
            if (strchr(con -> must_be[i], *(word + i)) == NULL) {
                
                return 0;
            }
            
        } else if (con -> cannot_be[*(word + i) - 'a'] == '1') {
            
            return 0;
        }
        if (w -> grid[row][i] != 'g') {
            if (word[i] == w->grid[0][i]) {
                return 0;
            }
        }
    }

    for (int i = 0; i < strlen(word); i++) {
        for (int j = i + 1; j < strlen(word); j++) {
            if (word[i] == word[j]) {
                if (w -> grid[row][i] == 'y' && w -> grid[row][j] == 'y' ) {
                    return 0;
                }
            }
        }
    }

    return 1;
}
/* remove "letter" from "word"
 * "word" remains the same if "letter" is not in "word"
 */
void remove_char(char *word, char letter) {
    char *ptr = strchr(word, letter);
    if(ptr != NULL) {
        *ptr = word[strlen(word) - 1];
        word[strlen(word) - 1] = '\0';
    }
}

/* Build a tree starting at "row" in the wordle "w". 
 * Use the "parent" constraints to set up the constraints for this node
 * of the tree
 * For each word in "dict", 
 *    - if a word matches the constraints, then 
 *        - create a copy of the constraints for the child node and update
 *          the constraints with the new information.
 *        - add the word to the child_list of the current solver node
 *        - call solve_subtree on newly created subtree
 */

void solve_subtree(int row, struct wordle *w,  struct node *dict, 
                   struct solver_node *parent) {
    if(verbose) {
        printf("Running solve_subtree: %d, %s\n", row, parent->word);
    }

    // debugging suggestion, but you can choose how to use the verbose option
    /*if(verbose) {
        print_constraints(c);
    } */
    
    if (row > (w-> num_rows) - 1 || row < 1) {
        return;
    } 
    char next[WORDLEN + 1] = "ggggg\0";
    if (row > 1) {
        strncpy(next, w -> grid[row - 1], WORDLEN);
        next[WORDLEN] = '\0';
    }
    for (int i = 0; i < WORDLEN; i++) {
        set_yellow(i, w -> grid[row], next, parent -> word, parent -> con);
    }
    add_to_cannot_be(parent -> word, parent -> con);

    if (dict -> word) {
        struct node *curr = dict;
        while (curr -> next && (parent -> child_list == NULL)) {
            if (match_constraints( curr -> word , parent -> con, w, row) == 1) {
                
                parent -> child_list = create_solver_node(parent -> con, curr -> word);
            }
            curr = (curr -> next);
        }
        if (!(parent -> child_list)) {
            if (match_constraints( curr -> word , parent -> con, w, row) == 1) {
                
                parent -> child_list = create_solver_node(parent -> con, curr -> word);
            }
        } else {
            struct solver_node *loc = parent -> child_list;
            while (curr -> next) {
                if (match_constraints( curr -> word, parent -> con, w, row) == 1) {
                    loc -> next_sibling = create_solver_node(parent -> con, curr -> word);
                    loc = loc -> next_sibling;
                }
                curr = curr -> next;
            }
            if (match_constraints( curr -> word, parent -> con, w, row) == 1) {
                loc -> next_sibling = create_solver_node(parent -> con, curr -> word);
            }
        }
    }
    
    
    if (parent -> next_sibling) {
        solve_subtree(row, w,  dict, parent -> next_sibling);
    }
    
    if (parent -> child_list) {
        solve_subtree(row + 1, w,  dict, parent -> child_list);
    }
    
}

/* Print to standard output all paths that are num_rows in length.
 * - node is the current node for processing
 * - path is used to hold the words on the path while traversing the tree.
 * - level is the current length of the path so far.
 * - num_rows is the full length of the paths to print
 */

void print_paths(struct solver_node *node, char **path, 
                 int level, int num_rows) {
    
    if (level == num_rows) {
        for (int i = 0; i < num_rows - 1; i++) {
            printf("%s ", path[i]);
        }
        printf("%s\n", node -> word);
        if (node -> next_sibling){
            print_paths(node -> next_sibling, path, level, num_rows);
        }
        return;
    }
    if (level > num_rows) {
        return;
    }

    if (node -> next_sibling) {
        print_paths(node-> next_sibling, path, level, num_rows);

    }
    

    *(path + level - 1) = node -> word;
    
    if (node -> child_list){
        print_paths(node -> child_list, path, level + 1, num_rows);
    }
}
/*
void print_tree(struct solver_node *tree, int level) {
    printf("%d, %s", level, tree -> word);
    if (tree -> child_list) {
        print_tree(tree -> child_list, level + 1);
    }
    if (tree -> next_sibling) {
        print_tree(tree -> next_sibling, level);
    }
}*/

/* Free all dynamically allocated memory pointed to from w.
 */ 
void free_wordle(struct wordle *w){
    free(w);
}

/* Free all dynamically allocated pointed to from node
 */
void free_tree(struct solver_node *node){
    if (!(node -> con)) {
        free(node);
        return;
    }
    free_constraints(node -> con);
    if (node -> next_sibling) {
        free_tree(node -> next_sibling);
    }
    
    if (node -> child_list) {
        free_tree(node -> child_list);
    }
    free(node);
    return;
}
