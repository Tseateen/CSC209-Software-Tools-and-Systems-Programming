#include <stdio.h>  
#include <stdlib.h>  
#include <assert.h>  
#include "seqbot_helpers.h"  
  
/* Return the melting temperature of sequence, or -1 if the sequence is invalid. 
 * The melting temperature formula is given in the handout. 
 * An invalid sequence is a sequence of length 0, or a sequence that contains 
 * characters other than 'A', 'C', 'G', 'T'. 
 */  
int calculate_melting_temperature(char *sequence, int sequence_length)  
{     
    if (sequence_length == 0) {  
        return -1;  
    }  
    int na = 0, nc = 0, ng = 0, nt = 0;  
    int cmt_bol = 0;  
    for (int cmt_i = 0; cmt_i < sequence_length; cmt_i++) {  
        if (sequence[cmt_i] == 'A') {  
            na++;  
        } else if (sequence[cmt_i] == 'C') {  
            nc++;  
        } else if (sequence[cmt_i] == 'G') {  
            ng++;  
        } else if (sequence[cmt_i] == 'T') {  
            nt++;  
        } else {  
            cmt_bol = 1;  
            break;  
        }  
    }  
    if (cmt_bol == 1) {  
        return -1;  
    }  
    return (na + nt) * 2 + (nc + ng) * 4;  
}  
  
  
/* Prints the instructions to make a molecule from sequence. 
 * If an invalid character is found in sequence print 
 * "INVALID SEQUENCE" and return immediately 
 */  
void print_instructions(char *sequence, int sequence_length)  
{  
    int temp = calculate_melting_temperature(sequence, sequence_length);  
    if (temp == -1) {  
        printf("INVALID SEQUENCE\n");  
        return;  
    }  
    printf("START\n");  
    int num_seq = 1;  
    for (int ins = 1; ins < sequence_length; ins++){  
        if (sequence[ins]==sequence[ins - 1])  
        {  
            num_seq++;  
        } else {  
            printf("WRITE %c %d\n", sequence[ins - 1], num_seq);  
            num_seq = 1;  
        }  
    }  
    printf("WRITE %c %d\n", sequence[sequence_length - 1], num_seq);  
    printf("SET_TEMPERATURE %d\n", temp);  
    printf("END\n");  
}  
  
/* 
* Below is the helper function of generate_all_molecules 
*/  
  
int my_pow(int x, int y) {  
    int num = 1;  
    for (int pow_num = 0; pow_num < y; pow_num++) {  
        num *= x;  
    }  
    return num;  
}  
  
/* Print to standard output all of the sequences of length k. 
 * The format of the output is "<length> <sequence> 0" to  
 * correspond to the input format required by generate_molecules_from_file() 
 *  
 * Reminder: you are not allowed to use string functions in these files. 
 */  
void generate_all_molecules(int k)  
{     
    if (k < 1){  
        return;  
    }  
    int num_types = my_pow(4, k);  
    int now_tm, mod, times;  
    for (int total_times = 0; total_times < num_types; total_times++) {  
        now_tm = total_times;  
        printf("%d ",k);  
        for (int index = 0; index < k; index++) {  
            times = my_pow(4, k -index -1);  
            mod = now_tm / times;  
            now_tm = now_tm % times;  
            if (mod == 0){  
                printf("A");  
            } else if (mod == 1) {  
                printf("C");  
            } else if (mod == 2) {  
                printf("G");  
            } else {  
                printf("T");  
            }  
        }  
        printf(" 0\n");  
    }  
}  
  
  
/* 
* Below is the helper function of generate_molecules_from_file 
*/    
void reverse(char *arr_q, int len) {  
    int sub_i;  
    for (int rei = 0; rei < len/2; rei++){  
        sub_i = arr_q[rei];  
        arr_q[rei]= arr_q[len - 1 -rei];  
        arr_q[len - 1 -rei] = sub_i;  
    }  
}  
  
int comple(char *arr_q, int len) {  
    for (int coi = 0; coi < len; coi++) {  
        if (arr_q[coi] == 'A') {  
            arr_q[coi] = 'T';  
        } else if (arr_q[coi] == 'C') {  
            arr_q[coi] = 'G';  
        } else if (arr_q[coi] == 'G') {  
            arr_q[coi] = 'C';  
        } else if (arr_q[coi] == 'T') {  
            arr_q[coi] = 'A';  
        } else {  
            return 1;  
        }  
    }
    return 0;  
}  
  
int other_char_check(char * arr_q, int len) {  
    for (int occi = 0; occi < len; occi++) {  
        if (arr_q[occi] != 'A' && arr_q[occi] != 'C'&& arr_q[occi] != 'G'&& arr_q[occi] != 'T'){  
            return 1;  
        }  
    }
    return 0;  
}  
  
/* Print the instructions for each of the sequences found in filename according 
 * to the mode provided. 
 * filename contains one sequence per line, and the format of each line is 
 * "<length> <sequence> <mode>" where  
 *     - <length> is the number of characters in the sequence  
 *     - <sequence> is the array of DNA characters 
 *     - <mode> is either 0, 1, 2, or 3 indicating how the <sequence> should  
 *              be modified before printing the instrutions. The modes have the  
 *              following meanings: 
 *         - 0  - print instructions for sequence (unmodified) 
 *         - 1  - print instructions for the the complement of sequence 
 *         - 2  - print instructions for the reverse of sequence 
 *         - 3  - print instructions for sequence where it is complemented  
 *                and reversed. 
 *  
 * Error checking: If any of the following error conditions occur, the function 
 * immediately prints "INVALID SEQUENCE" to standard output and exits the  
 * program with a exit code of 1. 
 *  - length does not match the number of characters in sequence 
 *  - length is not a positive number 
 *  - sequence contains at least one invalid character 
 *  - mode is not a number between 0 and 3 inclusive 
 *  
 * You do not need to verify that length or mode are actually integer numbers, 
 * only that they are in the correct range. It is recommended that you use a  
 * fscanf to read the numbers and fgetc to read the sequence characters. 
 */  
void generate_molecules_from_file(char* filename)  
{  
    // TODO: complete this function  
    FILE *fp = fopen(filename, "r");  
    if (fp == NULL) {  
        printf("INVALID SEQUENCE\n");  
        exit(1); 
    }  
    int q_length;  
    int q_mod;  
    int a;
    while (fscanf(fp, "%d", &q_length) == 1) {  
        if (q_length < 1) {  
            printf("INVALID SEQUENCE\n");  
            exit(1);
            fclose(fp); 
        }  
        char q_arr[q_length + 1];  
        fscanf(fp, "%s %d", q_arr, &q_mod);  
        if (q_mod < 0 || q_mod > 3) {  
            printf("INVALID SEQUENCE\n");  
            exit(1);
            fclose(fp);  
        }  
        if (sizeof(q_arr) == 0) {  
            printf("INVALID SEQUENCE\n");  
            exit(1);
            fclose(fp);  
        }  
        if ((sizeof(q_arr) / sizeof(q_arr[0]) - 1) != q_length || q_arr[q_length]) {  
            printf("INVALID SEQUENCE\n");  
            exit(1);
            fclose(fp);  
        }  
        if (q_mod == 1) {  
            a = comple(q_arr, q_length);
            if (a == 1) {
                printf("INVALID SEQUENCE\n");  
                exit(1);
                fclose(fp); 
            }  
        } else if (q_mod == 2) {  
            a = other_char_check(q_arr, q_length); 
            if (a == 1) {
                printf("INVALID SEQUENCE\n");  
                exit(1);
                fclose(fp); 
            } 
            reverse(q_arr, q_length);  
        } else if (q_mod == 3) {  
            a = comple(q_arr, q_length); 
            if (a == 1) {
                printf("INVALID SEQUENCE\n");  
                exit(1);
                fclose(fp); 
            } 
            reverse(q_arr, q_length);  
        } else {  
            a = other_char_check(q_arr, q_length);
            if (a == 1) {
                printf("INVALID SEQUENCE\n");  
                exit(1);
                fclose(fp); 
            }   
        }  
        print_instructions(q_arr, q_length);  
    }  
    fclose(fp);
}  