#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "constraints.h"

/* Create and initialize a constraints struct. 
 * Sets the fields to 0 or empty string.
 * Return a pointer to the newly created constraints struct.
 */
struct constraints *init_constraints() {
    struct constraints *c = malloc(sizeof(struct constraints));
    char s[ALPHABET_SIZE] = {'\0'};
    strncpy(c -> cannot_be, s, ALPHABET_SIZE);
    for (int i = 0; i < WORDLEN; i++) {
        strncpy(c -> must_be[i], s, SIZE);
    }
    return c;
}

/* Update the "must_be" field at "index" to be a string 
 * containing "letter"
 * The tile at this index is green, therefore the letter at "index"
 * must be "letter"
 */
void set_green(char letter, int index, struct constraints *con) {
    assert(islower(letter));
    assert(index >= 0 && index < WORDLEN);
    
    char s[SIZE] = {'\0'};
    s[0] = letter;
    strcpy(con->must_be[index], s);
    con->must_be[index][0] = letter;
}

/* Update "con" by adding the possible letters to the string at the must_be 
 * field for "index".
 * - index is the index of the yellow tile in the current row to be updated
 * - cur_tiles is the tiles of this row
 * - next_tiles is the tiles of the row that is one closer to the solution row
 * - word is the word in the next row (assume word is all lower case letters)
 * Assume cur_tiles and next_tiles contain valid characters ('-', 'y', 'g')
 * 
 * Add to the must_be list for this index the letters that are green in the
 * next_tiles, but not green in the cur_tiles or green or yellow in the 
 * next_tiles at index.
 * Also add letters in yellow tiles in next_tiles.
 */
void set_yellow(int index, char *cur_tiles, char *next_tiles, 
                char *word, struct constraints *con) {

    assert(index >=0 && index < SIZE);
    assert(strlen(cur_tiles) == WORDLEN);
    assert(strlen(next_tiles) == WORDLEN);
    assert(strlen(word) == WORDLEN);

    char s[SIZE] = {'\0'};
    if (*(cur_tiles + index) == 'g') {
        set_green(*(word + index), index, con);
        return;
    } else if (*(cur_tiles + index) == '-') {
        strcpy(con -> must_be[index], s);
        return;
    }
    int num = 0;
    for (int i = 0; i < WORDLEN; i++) {
        if (*(cur_tiles + i) == 'g') {
            continue;
        } else if (i == index) {
            continue;
        } if (*(next_tiles + i) == 'g' || *(next_tiles + i) == 'y') {
            s[num] = word[i];
            num++;
        }
    }
    strncpy(con -> must_be[index], s, SIZE - 1);
    con -> must_be[index][SIZE -1] = '\0';
    return;
}

/* Add the letters from cur_word to the cannot_be field.
 * See the comments in constraints.h for how cannot_be is structured.
 */
void add_to_cannot_be(char *cur_word, struct constraints *con) {
    assert(strlen(cur_word) <= WORDLEN);

    for (int i = 0; i < strlen(cur_word); i++) {
        con -> cannot_be[*(cur_word + i) - 'a'] = '1';
    }
}

void print_constraints(struct constraints *c) {
    printf("cannot_be: ");

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (c->cannot_be[i] == '1') {
            printf("%c ", 'a' + i);
        }
    }
    
    printf("\nmust_be\n");

    for (int ik = 0; ik < WORDLEN; ik++) {
        printf("[%d] ", ik);
        for (int j = 0; j < strlen(c->must_be[ik]); j++) {
            printf("%c ", c->must_be[ik][j]);
        }
        printf("\n");
    }

}

/* Free all dynamically allocated memory pointed to by c
 */
void free_constraints(struct constraints *c) {
    free(c);
}