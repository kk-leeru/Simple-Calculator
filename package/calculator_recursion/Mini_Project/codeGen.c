#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
int track, idx = 0, sbcount;
int right = 0, div_right = 0, div_ret, incdec_right = 0, overload = 0;
int sub[73];
int buffer = sbcount-1;
int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;
    if (root != NULL) {
        switch (root->data) {
            case ID:
                if (track>=8){
                    printf("MOV [%d] r%d\n", (buffer+1)*4, idx%8);
                    idx++;
                    buffer++;
                }

                root->val = track;
                retval = getval(root->lexeme);
//                if (retval == -1){
//                    error(UNDEFINED);
//                }
                if (div_right){
                    div_ret = 1;
                }
                //root->val = track++;
                break;
            case INT:
                if (track>=8){
                    printf("MOV [%d] r%d\n", (buffer+1)*4, idx%8);
                    idx++;
                    buffer++;
                }
                if (incdec_right){
                    error(SYNTAXERR);
                }
                retval = atoi(root->lexeme);
                root->val = track;
                //root->val = track++;

                printf("MOV r%d %d\n", track++%8, retval);
                //track++;
                break;
            case ASSIGN:
                //right = 1;
                root->val = track;
                lv = rv = evaluateTree(root->right);
                //right = 0;
                retval = setval(root->left->lexeme, lv);
                //root->val = root->left->val;
                break;
            case INCDEC:
                incdec_right = 1;
                root->val = track;
                rv = evaluateTree(root->right);
                incdec_right = 0;
                //root->val = root->right->val;
                if (strcmp(root->lexeme, "++") == 0){
                    if (track>=8){
                        printf("MOV [%d] r%d\n", (buffer+1)*4, idx%8);
                    }
                    printf("MOV r%d 1\n", track%8);
                    printf("ADD r%d r%d\n", (root->val)%8, track%8);
                    if (track>= 8){
                        printf("MOV r%d [%d]\n", track%8, (buffer+1)*4);
                        buffer--;
                    }
                    //printf("MOV r%d %d\n", track++, rv+1);
                    retval = setval(root->right->lexeme, rv+1);
                }
                else if (strcmp(root->lexeme, "--") == 0){
                    if (track>=8){
                        printf("MOV [%d] r%d\n", (buffer+1)*4, idx%8);
                    }
                    printf("MOV r%d 1\n", track%8);
                    printf("SUB r%d r%d\n", (track-1)%8, track%8);
                    if (track >= 8){
                        printf("MOV r%d [%d]\n", track%8, (buffer+1)*4);
                        buffer--;
                    }
                    retval = setval(root->right->lexeme, rv-1);
                }
                break;
            case ADDSUB:
                root->val = track;
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                //root->val = root->left->val;
                if (track>0){
                    track--;
                }
//                if (div_ret)
//                    div_ret = 0;
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    printf("ADD r%d r%d\n", root->left->val%8, root->right->val%8);
                    if (track>=8){
                        printf("MOV r%d [%d]\n", root->right->val%8, (buffer- 1)*4);
                        buffer--;
                    }
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB r%d r%d\n", root->left->val%8, root->right->val%8);
                    if (track>=8){
                        printf("MOV r%d [%d]\n", root->right->val%8, (buffer - 1)*4);
                        buffer--;
                    }
                }
                break;
            case OR:
                root->val = track;
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (track>0)
                    track--;
                retval = lv | rv;
                printf("OR r%d r%d\n", root->left->val%8, root->right->val%8);
                if (track>=8){
                    printf("MOV r%d [%d]\n", root->right->val%8, (buffer- 1)*4);
                    buffer--;
                }
                break;
            case XOR:
                root->val = track;
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (track>0)
                    track--;
                retval = lv ^ rv;
                //root->val = root->left->val;
                printf("XOR r%d r%d\n", root->left->val%8, root->right->val%8);
                if (track>=8){
                    printf("MOV r%d [%d]\n", root->right->val%8, (buffer - 1)*4);
                    buffer--;
                }
                break;
            case AND:
                //printf("case and\n");
                root->val = track;
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (track>0)
                    track--;
                retval = lv & rv;
                //root->val = root->left->val;
                printf("AND r%d r%d\n", root->left->val%8, root->right->val%8);
                if (track>=8){
                    printf("MOV r%d [%d]\n", root->right->val%8, (buffer- 1)*4);
                    buffer--;
                }
                break;
            case MULDIV:
                root->val = track;
                div_ret = 0;
                lv = evaluateTree(root->left);
                div_right = 1;
                rv = evaluateTree(root->right);
                div_right = 0;
                if (track>0)
                    track--;
                if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                    printf("MUL r%d r%d\n", root->left->val%8, root->right->val%8);
                    if (track>=8){
                        printf("MOV r%d [%d]\n", root->right->val%8, (buffer- 1)*4);
                        buffer--;
                    }
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv != 0){
                        retval = lv / rv;
                        printf("DIV r%d r%d\n", root->left->val%8, root->right->val%8);
                        if (track>=8){
                            printf("MOV r%d [%d]\n", root->right->val%8, (buffer- 1)*4);
                            buffer--;
                        }
                    }
                    else if (rv == 0){
                        if (div_ret){
                            printf("DIV r%d r%d\n", root->left->val%8, root->right->val%8);
                            if (track>=8){
                                printf("MOV r%d [%d]\n", root->right->val%8, (buffer - 1)*4);
                                buffer--;
                            }
                        }
                        else {
                            error(DIVZERO);
                        }

//                        if (root->right->data == INT){
//                            error(DIVZERO);
//                        }
//                        if (div_ret){
//                            printf("DIV r%d r%d\n", root->left->val, root->right->val);
//                            //div_ret = 0;
//                        }
//                        else {
//                            error(DIVZERO);
//                        }
//                        if (root->right->data == ID){
//                            printf("DIV r%d r%d\n", root->left->val, root->right->val);
//                        }
//                        else if (root->right->data == INT){
//                            error(DIVZERO);
//                        }
//                        if (root->right->data != ID){
//                            error(DIVZERO);
//                        }
//                        printf("DIV r%d r%d\n", root->left->val, root->right->val);
                    }
                }
                break;
            default:
                retval = 0;
        }
    }
    return retval;
    //return error? if node null
}

void printPrefix(BTNode *root) {
//    if (root != NULL) {
//        printf("%s ", root->lexeme);
//        printPrefix(root->left);
//        printPrefix(root->right);
//    }
    if (root != NULL){
        printPrefix(root->left);
        printf("%s ", root->lexeme);
        printPrefix(root->right);
    }
}
