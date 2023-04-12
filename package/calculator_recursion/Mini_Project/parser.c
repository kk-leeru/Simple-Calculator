#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

int sbcount = 0, track = 0, div_ret = 0, overload, idx;
Symbol table[TBLSIZE];

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str) {
    int i = 0;
    for (i = 0; i < sbcount; i++){
        if (strcmp(str, table[i].name) == 0){
            printf("MOV r%d [%d]\n", track%8, 4*i);
            track++;
            return table[i].val;
        }
    }
    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    error(UNDEFINED);
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    return -1;
}

int setval(char *str, int val) {
    int i = 0;
    //update value if the id already exists
    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            //printf("track: %d\n", track);

            if (track == 0){
                printf("MOV [%d] r%d\n", 4*i, 0);
            }
            else {
                printf("MOV [%d] r%d\n", 4*i, (track-1) % 8);
            }
//            if (track>0){
//                track--;
//            }
            return val;
        }
    }
    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    //else create the id in table symbol, and set val/ memory
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;

    if (track == 0){
        printf("MOV [%d] r%d\n", 4*sbcount, 0);
    }
    else {
        printf("MOV [%d] r%d\n", 4*sbcount, (track-1)%8);
    }

    //    if (track>0){
    //        track--;
    //    }
    sbcount++;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}
BTNode* assign_expr(void);
// factor := INT | ADDSUB INT |
//		   	 ID  | ADDSUB ID  |
//		   	 ID ASSIGN expr |
//		   	 LPAREN expr RPAREN |
//		   	 ADDSUB LPAREN expr RPAREN
//BTNode *factor(void) {
//    BTNode *retp = NULL, *left = NULL;
//
//    if (match(INT)) {
//        retp = makeNode(INT, getLexeme());
//        advance();
//    } else if (match(ID)) {
//        left = makeNode(ID, getLexeme());
//        advance();
//        if (!match(ASSIGN)) {
//            retp = left;
//        } else {
//            retp = makeNode(ASSIGN, getLexeme());
//            advance();
//            retp->left = left;
//            retp->right = expr();
//        }
//    } else if (match(ADDSUB)) {
//        retp = makeNode(ADDSUB, getLexeme());
//        retp->left = makeNode(INT, "0");
//        advance();
//        if (match(INT)) {
//            retp->right = makeNode(INT, getLexeme());
//            advance();
//        } else if (match(ID)) {
//            retp->right = makeNode(ID, getLexeme());
//            advance();
//        } else if (match(LPAREN)) {
//            advance();
//            retp->right = expr();
//            if (match(RPAREN))
//                advance();
//            else
//                error(MISPAREN);
//        } else {
//            error(NOTNUMID);
//        }
//    } else if (match(LPAREN)) {
//        advance();
//        retp = expr();
//        if (match(RPAREN))
//            advance();
//        else
//            error(MISPAREN);
//    } else {
//        error(NOTNUMID);
//    }
//    return retp;
//}

//// term := factor term_tail
//BTNode *term(void) {
//    BTNode *node = factor();
//    return term_tail(node);
//}

//// term_tail := MULDIV factor term_tail | NiL
//BTNode *term_tail(BTNode *left) {
//    BTNode *node = NULL;
//
//    if (match(MULDIV)) {
//        node = makeNode(MULDIV, getLexeme());
//        advance();
//        node->left = left;
//        node->right = factor();
//        return term_tail(node);
//    } else {
//        return left;
//    }
//}

//// expr := term expr_tail
//BTNode *expr(void) {
//    BTNode *node = term();
//    return expr_tail(node);
//}

//// expr_tail := ADDSUB term expr_tail | NiL
//BTNode *expr_tail(BTNode *left) {
//    BTNode *node = NULL;
//
//    if (match(ADDSUB)) {
//        node = makeNode(ADDSUB, getLexeme());
//        advance();
//        node->left = left;
//        node->right = term();
//        return expr_tail(node);
//    } else {
//        return left;
//    }
//}

//*new* factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *factor(void) {
    BTNode *retp = NULL;
    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) {
        retp = makeNode(ID, getLexeme());
        advance();
    } else if (match(INCDEC)) {
        retp = makeNode(INCDEC, getLexeme());
        advance();
        if (!match(ID)){
            error(SYNTAXERR);
        }
        retp->right = makeNode(ID, getLexeme());
        advance();
    } else if (match(LPAREN)) {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
}
//*new* unary_expr := ADDSUB unary_expr | factor
BTNode* unary_expr(void)
{
    BTNode* node = NULL;
    if (match(ADDSUB)){
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = makeNode(INT, "0");
        node->right = unary_expr();
//        if (node->right->data == ASSIGN){
//            error(SYNTAXERR);
//        }
//        if (node->right->data == ASSIGN){
//            printf("error detected\n");
//        }
    }
    else {
        node = factor();
    }
    return node;
}
//*new* muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode* muldiv_expr_tail(BTNode* left)
{
    BTNode* node = NULL;
    if (match(MULDIV)){
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        if (node->right == NULL || left == NULL){
            error(SYNTAXERR);
        }
//        if (node->lexeme[0] == '/' && node->right->lexeme[0] == 0){
//            err(DIVZERO);
//        }
        return muldiv_expr_tail(node);
    }
    else {
        return left;
    }
}
//*new* muldiv_expr := unary_expr muldiv_expr_tail
BTNode* muldiv_expr(void)
{
    BTNode* node = NULL;
    node = unary_expr();
    return muldiv_expr_tail(node);
}
//*new* addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode* addsub_expr_tail(BTNode* left)
{
    BTNode* node = NULL;
    if (match(ADDSUB)){
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = muldiv_expr();
        if (node->right == NULL || left == NULL){
            //printf("first detected error\n");
            error(SYNTAXERR);
        }
        return addsub_expr_tail(node);
    }
    else {
        return left;
    }
}
//*new* addsub_expr := muldiv_expr addsub_expr_tail
BTNode* addsub_expr(void)
{
    BTNode* node = NULL;
    node = muldiv_expr();
    return addsub_expr_tail(node);
}
//*new* and_expr_tail    := AND addsub_expr and_expr_tail | NiL
BTNode* and_expr_tail(BTNode* left)
{
    BTNode* node = NULL;
    if (match(AND)){
        //printf("found and\n");
        node = makeNode(AND,getLexeme());
        advance();
        node->left = left;
        node->right = addsub_expr();
        if (node->right == NULL || left == NULL){
            error(SYNTAXERR);
        }
        return and_expr_tail(node);
    }
    else {
        return left;
    }
}
//*new* and_expr := addsub_expr and_expr_tail | NiL
BTNode* and_expr(void)
{
    BTNode* node = NULL;
    node = addsub_expr();
    return and_expr_tail(node);
    //return node;
}
//*new* xor_expr_tail := XOR and_expr xor_expr_tail | NiL
BTNode* xor_expr_tail(BTNode* left)
{
    BTNode* node = NULL;
    if (match(XOR)){
        node = makeNode(XOR, getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        if (node->right == NULL || left == NULL){
            error(SYNTAXERR);
        }
        return xor_expr_tail(node);
    }
    else {
        return left;
    }
}
//*new* xor_expr := and_expr xor_expr_tail
BTNode* xor_expr(void)
{
    BTNode* node = NULL;
    node = and_expr();
    return xor_expr_tail(node);
}
//*new* or_expr_tail := OR xor_expr or_expr_tail | NiL
BTNode* or_expr_tail(BTNode* left)
{
    BTNode* node = NULL;
    if (match(OR)){
        node = makeNode(OR, getLexeme());
        advance();
        node->left = left;
        node->right = xor_expr();
        if (node->right == NULL || left == NULL){
            error(SYNTAXERR);
        }
        return or_expr_tail(node);
    }
    else {
        return left;
    }
}
//*new* or_expr := xor_expr or_expr_tail
BTNode* or_expr(void)
{
    BTNode* node = NULL;
    node = xor_expr();
    return or_expr_tail(node);
}
//*new* assign_expr := ID ASSIGN assign_expr | or_expr
BTNode* assign_expr(void)
{
    BTNode *node = NULL, *temp = NULL;
    temp = or_expr();
    if (temp->data == ID) {
        if (match(ASSIGN)) {
            node = makeNode(ASSIGN, getLexeme());
            advance();
            node->left = temp;
            node->right = assign_expr();
            return node;
        }
    }
    return temp;
}

// statement := ENDFILE | END | expr END
//*new* statement :=  END | assign_expr END
void statement(void) {
    BTNode *retp = NULL;
    track = 0;
    div_ret = 0;
    idx = 0;
    if (match(ENDFILE)) {
        printf("MOV r0 [0]\n");
        printf("MOV r1 [4]\n");
        printf("MOV r2 [8]\n");
        printf("EXIT 0\n");
        exit(0);
    }
    if (match(END)) {
        //printf(">> ");
        advance();
//        printf("MOV r0 [0] %d\n", table[0].val);
//        printf("MOV r1 [4] %d\n", table[1].val);
//        printf("MOV r2 [8] %d\n", table[2].val);
//        printf("EXIT 0\n");
        //exit(0);
    } else {
        //retp = expr();
        retp = assign_expr();
        if (match(END)) {
            evaluateTree(retp);
            //printf("Prefix traversal: ");
            //printPrefix(retp);
            //printf("\n");
            freeTree(retp);
            //printf(">> ");
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum) {
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    printf("EXIT 1\n");
    exit(0);
}
