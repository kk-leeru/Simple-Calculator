#ifndef __LEX__
#define __LEX__

#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, END, ENDFILE,
    INT, ID,
    ADDSUB, MULDIV,
    ASSIGN,
    LPAREN, RPAREN,
    OR, XOR, AND, INCDEC,
} TokenSet;

// Test if a token matches the current token
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

#endif // __LEX__
#ifndef __PARSER__
#define __PARSER__

#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 0

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left;
    struct _Node *right;
} BTNode;

// The symbol table
extern Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Get the value of a variable
extern int getval(char *str);

// Set the value of a variable
extern int setval(char *str, int val);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

extern BTNode *factor(void);
extern BTNode *term(void);
extern BTNode *term_tail(BTNode *left);
extern BTNode *expr(void);
extern BTNode *expr_tail(BTNode *left);
extern void statement(void);

// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif // __PARSER__
#ifndef __CODEGEN__
#define __CODEGEN__


// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } else if (isalpha(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while ((isalpha(c) || isdigit(c) || c == '_') && i < MAXLEN){
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    } else if (c == '+' || c == '-') {
        lexeme[0] = c;
        char temp = fgetc(stdin);
        if (c == temp){
            lexeme[1] = temp;
            lexeme[2] = '\0';
            return INCDEC;
        }
        ungetc(temp, stdin);
        lexeme[1] = '\0';
        return ADDSUB;
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (c == '|'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    } else if (c == '^'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    } else if (c == '&'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == EOF) {
        return ENDFILE;
    } else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int track, idx = 0;
int right = 0, div_right = 0, div_ret, incdec_right = 0, overload = 0, sbcount;
int sub[73];
int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;
    if (root != NULL) {
        switch (root->data) {
            case ID:
                if (track>=8){
                    printf("MOV [%d] r%d\n", sbcount*4, idx%8);
                    idx++;
                    sbcount++;
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
                    printf("MOV [%d] r%d\n", sbcount*4, idx%8);
                    idx++;
                    sbcount++;
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
                        printf("MOV [%d] r%d\n", (sbcount-1)*4, idx%8);
                    }
                    printf("MOV r%d 1\n", track%8);
                    printf("ADD r%d r%d\n", (root->val)%8, track%8);
                    if (track>= 8){
                        printf("MOV r%d [%d]\n", track%8, (sbcount)*4);
                        sbcount--;
                    }
                    //printf("MOV r%d %d\n", track++, rv+1);
                    retval = setval(root->right->lexeme, rv+1);
                }
                else if (strcmp(root->lexeme, "--") == 0){
                    if (track>=8){
                        printf("MOV [%d] r%d\n", (sbcount-1)*4, idx%8);
                    }
                    printf("MOV r%d 1\n", track%8);
                    printf("SUB r%d r%d\n", (track-1)%8, track%8);
                    if (track >= 8){
                        printf("MOV r%d [%d]\n", track%8, (sbcount)*4);
                        sbcount--;
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
                        printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                        sbcount--;
                    }
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB r%d r%d\n", root->left->val%8, root->right->val%8);
                    if (track>=8){
                        printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                        sbcount--;
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
                    printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                    sbcount--;
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
                    printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                    sbcount--;
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
                    printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                    sbcount--;
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
                        printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                        sbcount--;
                    }
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv != 0){
                        retval = lv / rv;
                        printf("DIV r%d r%d\n", root->left->val%8, root->right->val%8);
                        if (track>=8){
                            printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                            sbcount--;
                        }
                    }
                    else if (rv == 0){
                        if (div_ret){
                            printf("DIV r%d r%d\n", root->left->val%8, root->right->val%8);
                            if (track>=8){
                                printf("MOV r%d [%d]\n", root->right->val%8, (sbcount)*4);
                                sbcount--;
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// This package is a calculator
// It works like a Python interpretor
// Example:
// >> y = 2
// >> z = 2
// >> x = 3 * y + 4 / (2 * z)
// It will print the answer of every line
// You should turn it into an expression compiler
// And print the assembly code according to the input

// This is the grammar used in this package
// You can modify it according to the spec and the slide
// statement  :=  ENDFILE | END | expr END
// expr    	  :=  term expr_tail
// expr_tail  :=  ADDSUB term expr_tail | NiL
// term 	  :=  factor term_tail
// term_tail  :=  MULDIV factor term_tail| NiL
// factor	  :=  INT | ADDSUB INT |
//		   	      ID  | ADDSUB ID  | +
//		   	      ID ASSIGN expr |
//		   	      LPAREN expr RPAREN |
//		   	      ADDSUB LPAREN expr RPAREN

int main() {
    //freopen("input.txt", "w", stdout);

    initTable();
    //printf(">> ");
    while (1) {
        statement();
    }
//    printf("MOV r0 [0]\n");
//    printf("MOV r1 [4]\n");
//    printf("MOV r2 [8]\n");
//    printf("EXIT 0\n");
    return 0;
}
