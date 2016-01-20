#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

enum ScopeState {
    START, INSIDE_MAIN
};
enum State {
    KEYWORD, ID, ASSIGN, IF_CONDITION, WHILE_CONDITION, RETURN, NIY
};
enum Keyword {
    INT, FLOAT, CHAR, BOOL, IF, ELSE, MAIN, VOID, NUL, WHILE
};
enum VariableKeyword {
    INTK, FLOATK, CHARK, BOOLK, NULLK
};

struct Token {
    char *text;
    int lineNumber;
    struct Token *nextPointer;
};

struct StackNode {
    char data[100];
    struct StackNode *nextPointer;
};

struct ConditionStackNode {
    int index;
    int isWhile;
    int elseIndex;
    struct ConditionStackNode *nextPointer;
};

struct Symbol {
    int index;
    char *name;
    char *value;
    enum VariableKeyword type;
    struct Symbol *nextPointer;
};

int getPrecedence(char*);
typedef enum State State;
typedef struct Token Token;
typedef struct Symbol Symbol;
typedef enum Keyword Keyword;
typedef struct StackNode StackNode;
typedef struct Token *TokenPointer;
typedef enum ScopeState ScopeState;
typedef struct Symbol *SymbolPointer;
typedef struct StackNode *StackNodePointer;
typedef enum VariableKeyword VariableKeyword;
typedef struct ConditionStackNode ConditionStackNode;
typedef ConditionStackNode *ConditionStackNodePointer;

int isNumber(char*);
int isOperator(char*);
int isExpression(char*);
Keyword getKeyword(char*);
int isValidKeyword(char*);
int isVariableKeyword(char*);
int isValidIdentifier(char*);
int isCharacterAssignment(char*);
int isConditionalOperator(char*);
void emptyStack(StackNodePointer*);
int isStackEmpty(StackNodePointer);
SymbolPointer createSymbolPointer();
char *popFromStack(StackNodePointer*);
void skipToToken(TokenPointer*, char*);
void messageError(TokenPointer, char*);
VariableKeyword getVariableKeyword(char*);
void pushToStack(StackNodePointer*, char*);
int syntaxAnalyze(TokenPointer, SymbolPointer*);
void unexpectedTokenException(TokenPointer, char*);
SymbolPointer getSymbolFromTable(SymbolPointer, char*);
void insertSymbolToTable(SymbolPointer*, SymbolPointer);
void checkConditionalExpression(TokenPointer*, SymbolPointer);
void checkDefiniteExpression(TokenPointer*, SymbolPointer*, SymbolPointer);

// Methods for Intermediate Language Generation
char *buildPreTabsString(int);
int hasElse(TokenPointer tokenPointer);
void generateIRCode(TokenPointer, SymbolPointer);
int isConditionStackEmpty(ConditionStackNodePointer);
char *generateExpressionIRCode(TokenPointer*, SymbolPointer, int);
ConditionStackNode popFromConditionStack(ConditionStackNodePointer*);
void pushToConditionStack(ConditionStackNodePointer *, int, int, int);
char *generateConditionExpressionIRCode(TokenPointer*, SymbolPointer, int);

int preprocessor();
void save(TokenPointer head);
int loadSecond(TokenPointer head);
void printList(TokenPointer head);
int load(TokenPointer head, char a[]);
void loadToken(TokenPointer head, int lineNumber);
void tokenizer(TokenPointer *headToken , int lineNumber);

int isInteger(char*);
int isBoolean(char*);
int calculateIntStack(StackNodePointer*, TokenPointer*);
int calculateIntParStack(StackNodePointer*, TokenPointer*);
double calculateFloatStack(StackNodePointer*, TokenPointer*);
double calculateFloatParStack(StackNodePointer*, TokenPointer*);
VariableKeyword getExpressionType(TokenPointer*, SymbolPointer);
void checkComplexExpression(TokenPointer*, SymbolPointer*, SymbolPointer);

int isIRVariable(char*);
char *generateStackIR(StackNodePointer*, SymbolPointer, int*, int);
char *generateParStackIR(StackNodePointer*, SymbolPointer, int*, int);
char *generateComplexExpressionIRCode(TokenPointer*, SymbolPointer, int, SymbolPointer);

int hasError = 0;
int returnType = -1; // 0 = Void and 1 = Expression

int main() {

    TokenPointer headToken = NULL;
    SymbolPointer headSymbol = NULL;

    tokenizer(&headToken, preprocessor());
    syntaxAnalyze(headToken->nextPointer, &headSymbol);
    if (!hasError){
          generateIRCode(headToken->nextPointer, headSymbol);
    }
    /*
     * TODO: COMPLEX CONDITIONS
     * TODO: AFTER RETURN
     */
    return 0;

}

int syntaxAnalyze(TokenPointer currentToken, SymbolPointer *headSymbol) {

    char *token;
    State currentState = NIY;
    ScopeState scopeState = START;

    TokenPointer temp;
    SymbolPointer symbol;
    int mainHasReturned = 0;
    ConditionStackNodePointer conditionTopPointer = NULL;
    // Todo: Check When Next is Null

    while (currentToken != NULL) {

        token = currentToken->text;
        if (mainHasReturned && scopeState==INSIDE_MAIN && strcmp(token, "}")!=0) {
            unexpectedTokenException(currentToken, "Can't run any statement in main() after returning! Expected }");
            hasError = 1;
            skipToToken(&currentToken, ";");
            currentToken = currentToken->nextPointer;
            token = currentToken->text;
        }

        switch (currentState) {

            case NIY: {
                if (isValidKeyword(token)) {
                    if (isVariableKeyword(token)) {
                        if (currentToken->nextPointer != NULL) {
                            if (getVariableKeyword(token)==INTK && strcmp(currentToken->nextPointer->text, "main")==0) {
                                if (scopeState == START) {
                                    temp = currentToken->nextPointer->nextPointer;
                                    if (temp != NULL) {
                                        if (strcmp(temp->text, "(") == 0) {
                                            temp = temp->nextPointer;
                                            if (temp != NULL) {
                                                if (strcmp(temp->text, ")") == 0) {
                                                    temp = temp->nextPointer;
                                                    if (temp != NULL) {
                                                        if (strcmp(temp->text, "{") == 0) {
                                                            returnType = 1;
                                                            scopeState = INSIDE_MAIN;
                                                            currentToken = temp;
                                                        } else {
                                                            unexpectedTokenException(currentToken, "Expected {");
                                                            skipToToken(&currentToken, "{");
                                                            hasError = 1 ;
                                                        }
                                                    } else {
                                                        messageError(currentToken->nextPointer->nextPointer->nextPointer, "Expected [ but nothing found!");
                                                        hasError = 1;
                                                        exit(1);
                                                    }
                                                } else {
                                                    unexpectedTokenException(currentToken, "Expected )");
                                                    skipToToken(&currentToken, ")"); // TODO
                                                    hasError = 1 ;
                                                }
                                            } else {
                                                messageError(currentToken->nextPointer->nextPointer, "Expected ) but nothing found!");
                                                hasError = 1;
                                                exit(1);
                                            }
                                        } else {
                                            unexpectedTokenException(currentToken, "Expected (");
                                            skipToToken(&currentToken, "(");
                                            hasError = 1 ;
                                        }
                                    } else {
                                        messageError(currentToken->nextPointer, "Expected ( but nothing found!");
                                        hasError = 1;
                                        exit(1);
                                    }
                                } else {
                                    messageError(currentToken, "main() is defined already.");
                                    skipToToken(&currentToken, ";");
                                    hasError = 1 ;
                                }
                            } else {
                                symbol = createSymbolPointer();
                                symbol->type = getVariableKeyword(token);
                                currentState = KEYWORD; // Has Read Keyword, is going for Id.
                            }
                        } else {
                            messageError(currentToken, "Expected Identifer but nothing found!");
                            hasError = 1;
                            exit(1);
                        }
                    } else if (strcmp(token, "void") == 0) {
                        if (currentToken->nextPointer != NULL) {
                            if (strcmp(currentToken->nextPointer->text, "main")==0) {
                                if (scopeState == START) {
                                    temp = currentToken->nextPointer->nextPointer;
                                    if (temp != NULL) {
                                        if (strcmp(temp->text, "(") == 0) {
                                            temp = temp->nextPointer;
                                            if (temp != NULL) {
                                                if (strcmp(temp->text, ")") == 0) {
                                                    temp = temp->nextPointer;
                                                    if (temp != NULL) {
                                                        if (strcmp(temp->text, "{") == 0) {
                                                            returnType = 0;
                                                            scopeState = INSIDE_MAIN;
                                                            currentToken = temp;
                                                        } else {
                                                            unexpectedTokenException(currentToken, "Expected {");
                                                            skipToToken(&currentToken, "{");
                                                            hasError = 1 ;
                                                        }
                                                    } else {
                                                        messageError(currentToken->nextPointer->nextPointer->nextPointer, "Expected { but nothing found!");
                                                        exit(1);
                                                    }
                                                } else {
                                                    unexpectedTokenException(currentToken, "Expected )");
                                                    skipToToken(&currentToken, ")"); // Todo
                                                    hasError = 1 ;
                                                }
                                            } else {
                                                messageError(currentToken->nextPointer->nextPointer, "Expected ) but nothing found!");
                                                exit(1);
                                            }
                                        } else {
                                            unexpectedTokenException(currentToken, "Expected (");
                                            skipToToken(&currentToken, "(");
                                            hasError = 1 ;
                                        }
                                    } else {
                                        messageError(currentToken->nextPointer, "Expected ( but nothing found!");
                                        exit(1);
                                    }
                                } else {
                                    messageError(currentToken, "main() is defined already.");
                                    skipToToken(&currentToken, ";");
                                    hasError = 1 ;
                                }
                            } else {
                                unexpectedTokenException(currentToken, "Expected a valid start");
                                skipToToken(&currentToken, ";"); // Todo
                                hasError = 1;
                            }
                        } else {
                            messageError(currentToken, "Expected main but nothing found!");
                            exit(1);
                        }
                    } else if (strcmp(token, "if") == 0) {
                        if (scopeState == INSIDE_MAIN) {
                            if (currentToken->nextPointer != NULL) {
                                if (strcmp(currentToken->nextPointer->text, "(") == 0) {
                                    currentToken = currentToken->nextPointer;
                                    currentState = IF_CONDITION;
                                } else {
                                    unexpectedTokenException(currentToken->nextPointer, "Expected (");
                                    skipToToken(&currentToken, "("); /// Todo
                                    hasError = 1 ;
                                }
                            } else {
                                messageError(currentToken, "Expected ( but found nothing!");
                                exit(1);
                            }
                        } else {
                            messageError(currentToken, "If is not allowed outside main!");
                            skipToToken(&currentToken, "{"); // Todo
                            hasError = 1 ;
                        }
                    } else if (strcmp(token, "while") == 0) {
                        if (scopeState == INSIDE_MAIN) {
                            if (currentToken->nextPointer != NULL) {
                                if (strcmp(currentToken->nextPointer->text, "(") == 0) {
                                    currentToken = currentToken->nextPointer;
                                    currentState = WHILE_CONDITION;
                                } else {
                                    unexpectedTokenException(currentToken->nextPointer, "Expected (");
                                    skipToToken(&currentToken, "("); /// Todo
                                    hasError = 1 ;
                                }
                            } else {
                                messageError(currentToken, "Expected ( but found nothing!");
                                exit(1);
                            }
                        } else {
                            messageError(currentToken, "If is not allowed outside main!");
                            skipToToken(&currentToken, "{"); // Todo
                            hasError = 1 ;
                        }
                    } else if (strcmp(token, "else") == 0) {
                        messageError(currentToken, "No if to use else after that!");
                        skipToToken(&currentToken, "{"); // Todo
                        hasError = 1 ;
                    }
                } else if (strcmp(token, "return") == 0) {
                    currentState = RETURN; // Todo: Nothing Allowed After return
                } else if (strcmp(token, "}") == 0) {
                    if (!isConditionStackEmpty(conditionTopPointer)) {
                        ConditionStackNode conditionNode = popFromConditionStack(&conditionTopPointer);
                        if (conditionNode.elseIndex == -1) {
                            if (currentToken->nextPointer == NULL) {
                                if (!isConditionStackEmpty(conditionTopPointer)) {
                                    messageError(currentToken, "Main and some other scopes weren't closed with }");
                                } else messageError(currentToken, "Main wasn't closed with }");
                            } else {
                                if (strcmp(currentToken->nextPointer->text, "else") == 0) {
                                    TokenPointer temp = currentToken->nextPointer;
                                    if (temp != NULL) {
                                        currentToken = temp;
                                        if (strcmp(currentToken->nextPointer->text, "{") == 0) {
                                            currentToken = currentToken->nextPointer;
                                            pushToConditionStack(&conditionTopPointer, 0, 0, 0);
                                        } else {
                                            unexpectedTokenException(currentToken, "Expected {");
                                            skipToToken(&currentToken, "{");
                                            hasError = 1 ;
                                        }
                                    } else {
                                        messageError(currentToken, "Expected { but found nothing!");
                                        exit(1);
                                    }
                                } // Nothing Special Was done... !
                            }
                        } // Nothing Special Was done... !
                    } else if (scopeState == INSIDE_MAIN) {
                        if (mainHasReturned || returnType==0) scopeState = START;
                        else {
                            messageError(currentToken, "Can't close the main function before returning.");
                            hasError = 1 ;
                        }
                    } else {
                        messageError(currentToken, "No scope to close!");
                        hasError = 1 ;
                    }
                } else if (isValidIdentifier(token)) {
                    if (scopeState == INSIDE_MAIN) {
                        symbol = getSymbolFromTable(*headSymbol, token);
                        if (symbol != NULL) {
                            currentState = ID;
                        } else {
                            messageError(currentToken, "the variable wasn't defined but used!");
                            skipToToken(&currentToken, ";");
                            hasError = 1 ;
                        }
                    } else {
                        messageError(currentToken, "Can't work with identifiers outside main");
                        skipToToken(&currentToken, ";");
                        hasError = 1 ;
                    }
                } else {
                    unexpectedTokenException(currentToken, "Expected a valid start");
                    skipToToken(&currentToken, ";");
                    hasError = 1 ;
                }
                break;
            }

            case KEYWORD: {
                if (isValidIdentifier(token)) {
                    SymbolPointer symbolPointer = getSymbolFromTable(*headSymbol, token);
                    if (symbolPointer == NULL) {
                        symbol->name = token;
                        currentState = ID;
                    } else {
                        printf("Error at line %d: %s was already defined!\n", currentToken->lineNumber, token);
                        skipToToken(&currentToken, ";");
                        hasError = 1;
                        currentState = NIY;
                    }
                } else {
                    unexpectedTokenException(currentToken, "Expected a valid identifier");
                    skipToToken(&currentToken, ";");
                    hasError = 1;
                    currentState = NIY;
                }
                break;
            }

            case ID: {
                if (strcmp(token, "=") == 0) {
                    currentState = ASSIGN;
                } else if (strcmp(token, ";") == 0) {
                    insertSymbolToTable(headSymbol, symbol);
                    currentState = NIY;
                } else if (strcmp(token, ",") == 0) {
                    insertSymbolToTable(headSymbol, symbol);
                    VariableKeyword type = symbol->type;
                    symbol = createSymbolPointer();
                    symbol->type = type;
                    currentState = KEYWORD;
                } else {
                    unexpectedTokenException(currentToken, "Expected ; or = or comma");
                    skipToToken(&currentToken, ";");
                    hasError = 1;
                    currentState = NIY;
                }
                break;
            }

            case ASSIGN: {
                checkComplexExpression(&currentToken, headSymbol, symbol);
                currentState = NIY;
                break;
            }

            case IF_CONDITION: {
                checkConditionalExpression(&currentToken, *headSymbol);
                pushToConditionStack(&conditionTopPointer, 0, -1, 0);
                currentState = NIY;
                break;
            }

            case WHILE_CONDITION: {
                checkConditionalExpression(&currentToken, *headSymbol);
                pushToConditionStack(&conditionTopPointer, 0, 0, 1);
                currentState = NIY;
                break;
            }

            case RETURN: {
                if (scopeState == INSIDE_MAIN) {
                    if (!mainHasReturned) {
                        if (returnType == 1) {
                            if (isExpression(token)) {
                                if (currentToken->nextPointer != NULL) {
                                    if (strcmp(currentToken->nextPointer->text, ";") == 0) {
                                        mainHasReturned = 1;
                                        currentToken = currentToken->nextPointer;
                                        currentState = NIY;
                                    } else {
                                        unexpectedTokenException(currentToken->nextPointer, "Expected ; after return's expression");
                                        skipToToken(&currentToken, ";");
                                        hasError = 1 ;
                                        currentState = NIY;
                                    }
                                } else {
                                    messageError(currentToken, "Expected ; but found nothing!");
                                    exit(1);
                                }
                            } else {
                                unexpectedTokenException(currentToken, "Expected an expression after return");
                                skipToToken(&currentToken, ";");
                                hasError = 1 ;
                                currentState = NIY;
                            }
                        } else {
                            if (strcmp(token, ";") == 0) {
                                mainHasReturned = 1;
//                                currentToken = currentToken->nextPointer; TODO WHY
                                currentState = NIY;
                            } else {
                                unexpectedTokenException(currentToken, "Expected a ; after return in void main()");
                                skipToToken(&currentToken, ";");
                                hasError = 1 ;
                                currentState = NIY;
                            }
                        }
                    } else {
                        unexpectedTokenException(currentToken, "main has returned once!");
                        skipToToken(&currentToken, ";");
                        hasError = 1;
                    }
                } else {
                    unexpectedTokenException(currentToken, "Can't return outside of main! Expected a valid start");
                    skipToToken(&currentToken, ";");
                    hasError = 1;
                }
                break;
            }

        }

        if (currentToken != NULL) currentToken = currentToken->nextPointer;

    }

    if (!isConditionStackEmpty(conditionTopPointer)) {
        printf("Error at last line. some scopes and main weren't closed!\n");
        hasError = 1;
    } else if (scopeState == INSIDE_MAIN) {
        printf("Error at last line. main() wasn't closed!\n");
        hasError = 1;
    } else if (currentState != NIY) {
        switch (currentState) {
            case KEYWORD: {
                printf("Error at last line. Expected an identifier but nothing found!\n");
                hasError = 1;
                break;
            }
            case ID: {
                printf("Error at last line. Expected = or ; or , but nothing found!\n");
                hasError = 1;
                break;
            }
            case ASSIGN: {
                printf("Error at last line. Expected an expression but nothing found!\n");
                hasError = 1;
                break;
            }
            case IF_CONDITION: {
                printf("Error at last line. Expected an expression for if condition but nothing found!\n");
                hasError = 1;
                break;
            }
            case WHILE_CONDITION: {
                printf("Error at last line. Expected an expression for while condition but nothing found!\n");
                hasError = 1;
                break;
            }
            case RETURN: {
                printf("Error at last line. Expected an expression for return but nothing found!\n");
                hasError = 1;
                break;
            }
            default: break;
        }
    }

    return hasError;

}

void skipToToken(TokenPointer *currentToken, char *token) {
    while ((*currentToken) != NULL) {
        if (strcmp((*currentToken)->text, token) == 0) break;
        (*currentToken) = (*currentToken)->nextPointer;
    }
}

int preprocessor(void) {
    Token * head;
    int lineNumber = 0;
    head = (TokenPointer) malloc(sizeof (Token));
    head->text = " ";
    head->nextPointer = NULL;
    lineNumber = loadSecond(head);
    save(head);
    return lineNumber;
}

void save(TokenPointer headToken){
    TokenPointer current;
    current = headToken->nextPointer;
    FILE *p;
    p = fopen("tmp.c","w+");
    if (p==NULL){
        printf("ERROR!\n");
        exit(1);
    }
    while(current != NULL){
        fprintf(p,"%s", current->text);
        current = current->nextPointer;
    }
    fclose(p);
}

int load(Token* head, char a[]) {
    TokenPointer newToken;
    TokenPointer current;
    current = head;
    int counter = 1;
    char *str;
    char c;
    FILE *s;
    s = fopen(a, "r");
    if (s == NULL) {
        printf(" ERROR!\n");
        exit(1);
    }
    c = fgetc(s);
    fseek(s , 0 , SEEK_SET);
    if( c == EOF)
        return -1 ;
    else
        while (!feof(s)) {
            str = (char *) malloc(sizeof(char)*100);
            fgets(str, 100, s);
            newToken = (TokenPointer) malloc(sizeof(Token));
            current->nextPointer = newToken;
            newToken->text = str;
            newToken->nextPointer = NULL;
            current = current->nextPointer;
            counter++;
        }
    counter--;
    newToken = (TokenPointer) malloc(sizeof(Token));
    newToken->nextPointer = NULL;
    current->nextPointer = newToken;
    newToken->text = "\n";
    fclose(s);
    return counter;
}

int loadSecond(Token * head) {
    TokenPointer current = head;
    TokenPointer newToken;
    char *str;
    char a[100], b[100], c, abbas[100];
    int i = 0, j = 0, lineNumber = 0;
    FILE *p;
    p = fopen("main.c", "r");
    if (p == NULL) {
        printf("ERROR!\n");
        exit(1);
    }
    c = fgetc(p);
    if (c == '#') {
        fscanf(p, "%s", abbas);
        if (strcmp(abbas, "include") == 0) {
            fscanf( p,"%s", abbas);
            if (abbas[i] == '\"') {
                while (abbas[i] != '\"' && abbas[i] != '\0') {
                    i++;
                }
                if (abbas[i] != '\"') {
                    printf("ERROR 1 :D\n");
                    exit(1);
                } else {
                    fseek(p, 0, SEEK_SET);
                    fgets(a, 100, p);
                    i = 0;
                    while (a[i] != '\"')
                        i++;
                    i++;
                    while (a[i] != '\"') {
                        b[j] = a[i];
                        i++;
                        j++;
                    }
                    b[j] = '\0';
                    lineNumber = load(head, b);
                    if (lineNumber == -1){
                        lineNumber = 0;
                    }
                    while (!feof(p)) {
                        current = head;
                        while (current->nextPointer != NULL)
                            current = current->nextPointer;
                        str = (char*) malloc(sizeof (char)*50);
                        newToken = (Token *) malloc(sizeof (Token));
                        newToken->nextPointer = NULL;
                        current->nextPointer = newToken;
                        fgets(str, 100, p);
                        newToken->text = str;
                        current = current->nextPointer;
                    }

                }
            }
        } else {
            printf("ERROR! :D\n");
            return 0;
        }
    } else {
        fseek( p , 0, SEEK_SET);
        while( !feof(p)){
            current = head;
            while (current->nextPointer != NULL)
                current = current->nextPointer;
            str = (char*) malloc(sizeof (char)*50);
            newToken = (TokenPointer) malloc(sizeof (Token));
            newToken->nextPointer = NULL;
            current->nextPointer = newToken;
            fgets(str, 100, p);
            newToken->text = str;
            current = current->nextPointer;
        }
        fclose(p);
        return 1;
    }

    fclose(p);
    return lineNumber ;
}

void tokenizer(TokenPointer *headToken , int lineNumber) {
    *headToken = (TokenPointer) malloc(sizeof(Token));
    loadToken(*headToken , lineNumber );
    return;
}

void loadToken(Token * head , int lineNumber) {
    Token * current, * newToken;
    char * str, * delim, *token, *newLine;
    int counter = 2 , lineNum;
    current = head;
    delim = " ";
    FILE * p;
    p = fopen("tmp.c", "r");
    if (p == NULL) {
        printf("ERROR!\n");
        exit(1);
    }
    while (!feof(p)) {
        str = (char*) malloc(sizeof (char)* 80);
        fgets(str, 80, p);
        newLine = strchr(str, '\n');
        if (newLine != NULL) {
            *newLine = '\0';
        }
        token = strtok(str, delim);
        if ( token == NULL)
            counter --;
        while (token != NULL) {
            newToken = (Token *) malloc(sizeof (Token));
            current->nextPointer = newToken;
            newToken->nextPointer = NULL;
            newToken->text = token;
            lineNum = counter - lineNumber;
            if (lineNum <= 0) lineNum = 1;
            newToken->lineNumber = lineNum;
            current = current->nextPointer;
            token = strtok(NULL, delim);
        }
        counter++;
    }

    return;
}

void printList(TokenPointer head) {
    TokenPointer current;
    current = head->nextPointer;
    while (current != NULL) {
        printf("%s\t%d\n", current->text, current->lineNumber);
        current = current->nextPointer;
    }
    return;
}


void generateIRCode(TokenPointer currentToken, SymbolPointer headSymbol) {

    char *token;
    int conditionIndex=1, preTabs=0;

    SymbolPointer symbol;
    ConditionStackNodePointer conditionTopPointer = NULL;\

    while (currentToken != NULL) {

        token = currentToken->text;

        if (isValidKeyword(token)) {
            if (isVariableKeyword(token)) {
                if (getVariableKeyword(token)==INTK && strcmp(currentToken->nextPointer->text, "main")==0) {
                    while (strcmp(currentToken->text, "{") != 0) {
                        currentToken = currentToken->nextPointer;
                    }
                    printf("PROCEDURE MAIN\nBEGIN\n");
                    preTabs++;
                } else {
                    currentToken = currentToken->nextPointer; // Set tokenPointer to the id after the variable keyword
                    symbol = getSymbolFromTable(headSymbol, currentToken->text);
                    if (strcmp(currentToken->nextPointer->text, "=") == 0) {
                        currentToken = currentToken->nextPointer->nextPointer; // Set tokenPointer to after =
//                        printf("%sR%d := %s\n", buildPreTabsString(preTabs), symbol->index, generateExpressionIRCode(&currentToken, headSymbol, preTabs));
                        printf("%sR%d := %s\n", buildPreTabsString(preTabs), symbol->index, generateComplexExpressionIRCode(&currentToken, headSymbol, preTabs, symbol));
                        printf("%sT%d := R%d\n", buildPreTabsString(preTabs), symbol->index, symbol->index);
                    } else {
                        while (strcmp(currentToken->text, ";") != 0) currentToken = currentToken->nextPointer;
                        // Sets the TokenPointer on the semicolon, Handles the comma
                    }
                }
            } else if (strcmp(token, "void") == 0) {
                if (strcmp(currentToken->nextPointer->text, "main")==0) {
                    while (strcmp(currentToken->text, "{") != 0) {
                        currentToken = currentToken->nextPointer;
                    }
                    printf("PROCEDURE MAIN\nBEGIN\n");
                    preTabs++;
                }
            } else if (strcmp(token, "if") == 0) {

                currentToken = currentToken->nextPointer->nextPointer; // Set The TokenPointer to after (
                char conditionResult[50];
                strcpy(conditionResult, generateConditionExpressionIRCode(&currentToken, headSymbol, preTabs));
                currentToken = currentToken->nextPointer; // set the TokenPointer on {

                if (hasElse(currentToken->nextPointer)) {

                    int index=conditionIndex, elseIndex=index+1;
                    conditionIndex += 2;

                    printf("%sIF %s GOTO L%d ELSE L%d\n", buildPreTabsString(preTabs), conditionResult, index, elseIndex);
                    printf("%sL%d:\n", buildPreTabsString(preTabs), index);
                    preTabs++;
                    pushToConditionStack(&conditionTopPointer, index, elseIndex, 0);

                } else {

                    int index = conditionIndex;
                    conditionIndex++;

                    printf("%sIF %s GOTO L%d\n", buildPreTabsString(preTabs), conditionResult, index);
                    printf("%sL%d:\n", buildPreTabsString(preTabs), index);
                    preTabs++;
                    pushToConditionStack(&conditionTopPointer, index, -1, 0);

                }

            } else if (strcmp(token, "while") == 0) {

                int index = conditionIndex;
                conditionIndex++;

                printf("%sGOTO W%d\n", buildPreTabsString(preTabs), index);
                printf("%sW%d:\n", buildPreTabsString(preTabs), index);
                preTabs++;

                currentToken = currentToken->nextPointer->nextPointer; // Set The TokenPointer to after (
                char conditionResult[50];
                strcpy(conditionResult, generateConditionExpressionIRCode(&currentToken, headSymbol, preTabs));
                currentToken = currentToken->nextPointer; // set the TokenPointer on {

                printf("%sIF %s GOTO L%d\n", buildPreTabsString(preTabs), conditionResult, index);
                printf("%sL%d:\n", buildPreTabsString(preTabs), index);
                pushToConditionStack(&conditionTopPointer, index, -1, 1);
                preTabs++;

            }

        } else if (strcmp(token, "return") == 0) {
            currentToken = currentToken->nextPointer;
            if (returnType == 1) {
                symbol = (SymbolPointer) malloc(sizeof(Symbol));
                symbol->type = INTK;
//                printf("%sRETURN %s\n", buildPreTabsString(preTabs), generateExpressionIRCode(&currentToken, headSymbol, 0));
                printf("%sRETURN %s\n", buildPreTabsString(preTabs), generateComplexExpressionIRCode(&currentToken, headSymbol, 0, symbol));
            } else {
                printf("%sRETURN\n", buildPreTabsString(preTabs));
            }
            preTabs--;
        } else if (isValidIdentifier(token)) {
            symbol = getSymbolFromTable(headSymbol, token);
            currentToken = currentToken->nextPointer->nextPointer; // Set tokenPointer to after =
            if (strcmp(currentToken->nextPointer->text, ";") == 0 && isValidIdentifier(currentToken->text)) {
                printf("%sT%d := T%d\n", buildPreTabsString(preTabs), symbol->index, getSymbolFromTable(headSymbol, currentToken->text)->index);
            } else {
//                printf("%sR%d := %s\n", buildPreTabsString(preTabs),
//                       symbol->index, generateExpressionIRCode(&currentToken, headSymbol, preTabs));
                printf("%sR%d := %s\n", buildPreTabsString(preTabs),
                       symbol->index, generateComplexExpressionIRCode(&currentToken, headSymbol, preTabs, symbol));
                printf("%sT%d := R%d\n", buildPreTabsString(preTabs), symbol->index, symbol->index);
            }
            // Sets the TokenPointer on the semicolon.
        } else if (strcmp(token, "}") == 0) {

            if (!isConditionStackEmpty(conditionTopPointer)) {
                if (conditionTopPointer->isWhile) {
                    printf("%sGOTO W%d\n", buildPreTabsString(preTabs), conditionTopPointer->index);
                    popFromConditionStack(&conditionTopPointer);
                    preTabs -= 2;
                } else {
                    if (conditionTopPointer->elseIndex == -1) {
                        popFromConditionStack(&conditionTopPointer);
                        preTabs--;
                    } else {
                        int elseIndex = popFromConditionStack(&conditionTopPointer).elseIndex;
                        preTabs--;
                        printf("%sL%d:\n", buildPreTabsString(preTabs), elseIndex);
                        preTabs++;
                        pushToConditionStack(&conditionTopPointer, elseIndex, -1, 0);
                        currentToken = currentToken->nextPointer->nextPointer; // Set TokenPointer on {
                    }
                }
            } else {
                // Main Was Closed
                printf("CALL MAIN");
            }

        }

        currentToken = currentToken->nextPointer;

    }

}

char *generateExpressionIRCode(TokenPointer *tokenPointer, SymbolPointer headSymbol, int preTabs) {

    int index = 0;
    char *result = (char*) malloc(sizeof(char)*50);

    if (strcmp((*tokenPointer)->nextPointer->text, ";") == 0) {
        if (isValidIdentifier((*tokenPointer)->text)) {
            if (strcmp(result, "NULL") == 0) result = "0";
            else sprintf(result, "T%d", getSymbolFromTable(headSymbol, (*tokenPointer)->text)->index);
        } else {
            strcpy(result, (*tokenPointer)->text);
            (*tokenPointer) = (*tokenPointer)->nextPointer;
        }
        return result;
    } else {

        char *token1 = (char*) malloc(sizeof(char)*50);
        char *op = (char*) malloc(sizeof(char)*5);
        char *token2 = (char*) malloc(sizeof(char)*50);

        strcpy(token1, (*tokenPointer)->text);
        (*tokenPointer) = (*tokenPointer)->nextPointer;
        strcpy(op, (*tokenPointer)->text);
        (*tokenPointer) = (*tokenPointer)->nextPointer;
        strcpy(token2, (*tokenPointer)->text);
        (*tokenPointer) = (*tokenPointer)->nextPointer;

        if (!isValidIdentifier(token1)) {
            printf("%s%c%d := %s\n", buildPreTabsString(preTabs), 'T_', index++, token1);
            sprintf(token1, "T_%d", index-1);
        } else {
            if (strcmp(token1, "NULL") == 0) token1 = 0;
            else sprintf(token1, "T%d", getSymbolFromTable(headSymbol, token1)->index);
        }
        if (!isValidIdentifier(token2)) {
            printf("%sT_%d := %s\n", buildPreTabsString(preTabs), index++, token2);
            sprintf(token2, "T%d", index-1);
        } else {
            if (strcmp(token2, "NULL") == 0) token2 = 0;
            else sprintf(token2, "T%d", getSymbolFromTable(headSymbol, token2)->index);
        }

        sprintf(result, "%s %s %s", token1, op, token2);
        return result;

    }

}


char *generateConditionExpressionIRCode(TokenPointer *tokenPointer, SymbolPointer headSymbol, int preTabs) {

    int index = 0;
    char *result = (char*) malloc(sizeof(char)*50);

    char *token1 = (char*) malloc(sizeof(char)*50);
    char *op = (char*) malloc(sizeof(char)*5);
    char *token2 = (char*) malloc(sizeof(char)*50);

    strcpy(token1, (*tokenPointer)->text);
    (*tokenPointer) = (*tokenPointer)->nextPointer;
    strcpy(op, (*tokenPointer)->text);
    (*tokenPointer) = (*tokenPointer)->nextPointer;
    strcpy(token2, (*tokenPointer)->text);
    (*tokenPointer) = (*tokenPointer)->nextPointer;

    if (!isValidIdentifier(token1)) {
        printf("%sT_%d := %s\n", buildPreTabsString(preTabs), index++, token1);
        sprintf(token1, "T_%d", index-1);
    } else {
        if (strcmp(token1, "NULL") == 0) token1 = 0;
        else sprintf(token1, "%c%d", 'T', getSymbolFromTable(headSymbol, token1)->index);
    }
    if (!isValidIdentifier(token2)) {
        printf("%sT_%d := %s\n", buildPreTabsString(preTabs), index++, token2);
        sprintf(token2, "T_%d", index-1);
    } else {
        if (strcmp(token2, "NULL") == 0) token2 = 0;
        else sprintf(token2, "%c%d", 'T', getSymbolFromTable(headSymbol, token2)->index);
    }

    sprintf(result, "%s %s %s", token1, op, token2);
    return result;

}

int hasElse(TokenPointer tokenPointer) {

    char *token;
    int index = 0;
    ConditionStackNodePointer topPointer = NULL;
    TokenPointer currentToken = tokenPointer;

    do {

        token = currentToken->text;

        if (strcmp(token, "{") == 0) {
            pushToConditionStack(&topPointer, index++, -1, 0);
        } else if (strcmp(token, "}") == 0) {
            if (isConditionStackEmpty(topPointer)) {
                if (strcmp(currentToken->nextPointer->text, "else") == 0) return 1;
                else return 0;
            } else {
                popFromConditionStack(&topPointer).index;
                token = currentToken->nextPointer->text;
            }
        }

        currentToken = currentToken->nextPointer;

    } while (strcmp(token, "}") != 0);

    return 0; // Never Reaches Here! (I hope at least...)

}

char *buildPreTabsString(int preTabsCount) {
    char *result = malloc(sizeof(char) * 100);
    result[0] = '\0';
    while (preTabsCount-- > 0) {
        strcat(result, "    ");
    }
    return result;
}

void checkDefiniteExpression(TokenPointer *tokenPointer, SymbolPointer *headSymbol, SymbolPointer symbol) {

    // Todo: check comma as operator
    if (strcmp((*tokenPointer)->nextPointer->text, ";") == 0) {
        if (isExpression((*tokenPointer)->text)) {
            symbol->value = (*tokenPointer)->text;
            insertSymbolToTable(headSymbol, symbol);
            (*tokenPointer) = (*tokenPointer)->nextPointer;
        } else {
            unexpectedTokenException(*tokenPointer, "Expected an expression");
            skipToToken(tokenPointer, ";");
            hasError = 1;
        }
    } else if (isOperator((*tokenPointer)->nextPointer->text)) {
        // Todo: Calculate Expression and assign to value
        if (isExpression((*tokenPointer)->text)) {
            (*tokenPointer) = (*tokenPointer)->nextPointer->nextPointer;
            if (isExpression((*tokenPointer)->text)) {
                insertSymbolToTable(headSymbol, symbol);
                (*tokenPointer) = (*tokenPointer)->nextPointer;
            } else {
                unexpectedTokenException(*tokenPointer, "Expected an expression");
                skipToToken(tokenPointer, ";");
                hasError = 1;
            }
        } else {
            unexpectedTokenException(*tokenPointer, "Expected an expression");
            skipToToken(tokenPointer, ";");
            hasError = 1;
        }
    } else {
        unexpectedTokenException((*tokenPointer)->nextPointer, "Expected ; or operator");
        skipToToken(tokenPointer, ";");
        hasError = 1;
    }

}

char *generateComplexExpressionIRCode(TokenPointer *tokenPointer, SymbolPointer headSymbol, int preTabs, SymbolPointer symbol) {

    char *token;
    int tempIndex = 0;
    int currentState = 1;
    int currentPrecedence = -1;
    StackNodePointer topParStack = NULL;
    StackNodePointer topPrecedenceStack = NULL;
    StackNodePointer topCalculatorStack = NULL;

    switch (symbol->type) {

        case FLOATK:
        case INTK:

            while (strcmp((*tokenPointer)->text, ";")!=0) {

                token = (*tokenPointer)->text;
                switch (currentState) {

                    // Was NIY or operator
                    case 1: {
                        if (isExpression(token)) {
                            if (strcmp(token, "NULL") == 0) {
                                pushToStack(&topCalculatorStack, "0");
                            } else {
                                pushToStack(&topCalculatorStack, token);
                            }
                            currentState = 0; // Wants Operator or )
                        } else if (strcmp(token, "(") == 0) {
                            pushToStack(&topCalculatorStack, token);
                            pushToStack(&topParStack, token);
                            char *pre = (char*) malloc(sizeof(char)*10);
                            sprintf(pre, "%d", currentPrecedence);
                            pushToStack(&topPrecedenceStack, pre);
                            currentPrecedence = -1;
                            currentState = 1; // Wants Number
                        }
                        break;
                    }

                        // Was Number
                    case 0: {
                        if (isOperator(token)) {
                            if (getPrecedence(token) < currentPrecedence) {
                                // Calculate and push result to stack + operator and update precedence
                                char *pusher = generateStackIR(&topCalculatorStack, headSymbol, &tempIndex, preTabs);
                                pushToStack(&topCalculatorStack, pusher); // pushing result
                                pushToStack(&topCalculatorStack, token); // pushing operator
                                currentPrecedence = getPrecedence(token);
                                currentState = 1; // Wants a number
                            } else {
                                currentPrecedence = getPrecedence(token);
                                pushToStack(&topCalculatorStack, token);
                                currentState = 1; // Wants a number
                            }
                        } else if (strcmp(token, ")") == 0) {
                            // Calculate and pop from par stack and recover the precedence and push result to stack
                            // Should reach '('
                            char *pusher = generateParStackIR(&topCalculatorStack, headSymbol, &tempIndex, preTabs);
                            popFromStack(&topCalculatorStack); // Popping '('
                            popFromStack(&topParStack); // Popping '('
                            pushToStack(&topCalculatorStack, pusher);
                            if (!isStackEmpty(topPrecedenceStack)) {
                                currentPrecedence = atoi(popFromStack(&topPrecedenceStack));
                                currentState = 0; // Wants an operator
                            }
                        }
                        break;
                    }

                    default: break;

                }

                if ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {
                    (*tokenPointer) = (*tokenPointer)->nextPointer;
                }

            }

            return generateStackIR(&topCalculatorStack, headSymbol, &tempIndex, preTabs);
            break;

        case CHARK:
        case BOOLK:
            token = (*tokenPointer)->text;
            char *result = (char*) malloc(sizeof(char)*50);
            if (isValidIdentifier(token)) {
                if (strcmp(token, "NULL") == 0) result = "0";
                else sprintf(result, "T%d", getSymbolFromTable(headSymbol, token)->index);
            } else {
                strcpy(result, token);
                (*tokenPointer) = (*tokenPointer)->nextPointer;
            }
            return result;
            break;

    }

}

void checkComplexExpression(TokenPointer *tokenPointer, SymbolPointer *headSymbol, SymbolPointer symbol) {

    /*
     * Checks if syntax is true and assign the value to symbol and insert the symbol
     *  0 : Number
     *  1 : Operator
     */
    char *token;
    int currentState = 1;
    int buggyExpression = 0;
    int currentPrecedence = -1;
    StackNodePointer topParStack = NULL;
    StackNodePointer topPrecedenceStack = NULL;
    StackNodePointer topCalculatorStack = NULL;

    // TODO:
    // Todo: NULL
    // TODO: Handle Multiple initializations
    // TODO: Handle Unknown Value
    // Todo: Comma Initialization

    switch (symbol->type) {

        case INTK: {

            while ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {

                token = (*tokenPointer)->text;
                switch (currentState) {

                    // Was NIY or operator
                    case 1: {
                        if (isExpression(token)) {
                            if (getExpressionType(tokenPointer, *headSymbol) == INTK)  {
                                if (isValidIdentifier(token)) {
                                    token = getSymbolFromTable(*headSymbol, token)->value;
                                }
                                pushToStack(&topCalculatorStack, token);
                                currentState = 0; // Wants Operator or )
                            } else if (strcmp(token, "NULL") == 0) {
                                pushToStack(&topCalculatorStack, "0");
                                currentState = 0; // Wants Operator or )
                            } else {
                                unexpectedTokenException(*tokenPointer, "Expected an integer");
                                skipToToken(tokenPointer, ";");
                                emptyStack(&topParStack);
                                emptyStack(&topCalculatorStack);
                                emptyStack(&topPrecedenceStack);
                                hasError = 1;
                                buggyExpression = 1;
                            }
                        } else if (strcmp(token, "(") == 0) {
                            pushToStack(&topCalculatorStack, token);
                            pushToStack(&topParStack, token);
                            char *pre = (char*) malloc(sizeof(char)*10);
                            sprintf(pre, "%d", currentPrecedence);
                            pushToStack(&topPrecedenceStack, pre);
                            currentPrecedence = -1;
                            currentState = 1; // Wants Number
                        } else {
                            unexpectedTokenException(*tokenPointer, "Expected ( or an expression");
                            skipToToken(tokenPointer, ";");
                            emptyStack(&topParStack);
                            emptyStack(&topCalculatorStack);
                            emptyStack(&topPrecedenceStack);
                            hasError = 1;
                            buggyExpression = 1;
                        }
                        break;
                    }

                        // Was Number
                    case 0: {
                        if (isOperator(token)) {
                            if (getPrecedence(token) < currentPrecedence) {
                                // Calculate and push result to stack + operator and update precedence
                                int result = calculateIntStack(&topCalculatorStack, tokenPointer);
                                if ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {
                                    char *pusher = (char*) malloc(sizeof(char)*50);
                                    sprintf(pusher, "%d", result);
                                    pushToStack(&topCalculatorStack, pusher); // pushing result
                                    pushToStack(&topCalculatorStack, token); // pushing operator
                                    currentPrecedence = getPrecedence(token);
                                    currentState = 1; // Wants a number
                                } else {
                                    emptyStack(&topParStack);
                                    emptyStack(&topCalculatorStack);
                                    emptyStack(&topPrecedenceStack);
                                    buggyExpression = 1;
                                }
                            } else {
                                currentPrecedence = getPrecedence(token);
                                pushToStack(&topCalculatorStack, token);
                                currentState = 1; // Wants a number
                            }
                        } else if (strcmp(token, ")") == 0) {
                            // Calculate and pop from par stack and recover the precedence and push result to stack
                            // Should reach '('
                            int result = calculateIntParStack(&topCalculatorStack, tokenPointer);
                            if ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {
                                popFromStack(&topCalculatorStack); // Popping '('
                                popFromStack(&topParStack); // Popping '('
                                char *pusher = (char*) malloc(sizeof(char)*50);
                                sprintf(pusher, "%d", result);
                                pushToStack(&topCalculatorStack, pusher);
                                if (!isStackEmpty(topPrecedenceStack)) {
                                    currentPrecedence = atoi(popFromStack(&topPrecedenceStack));
                                    currentState = 0; // Wants an operator
                                } else {
                                    messageError(*tokenPointer, "WTF???");
                                    exit(1);
                                }
                            } else {
                                emptyStack(&topParStack);
                                emptyStack(&topCalculatorStack);
                                emptyStack(&topPrecedenceStack);
                                buggyExpression = 1;
                            }
                        } else {
                            unexpectedTokenException(*tokenPointer, "Expected ) or an operator");
                            skipToToken(tokenPointer, ";");
                            emptyStack(&topParStack);
                            emptyStack(&topCalculatorStack);
                            emptyStack(&topPrecedenceStack);
                            buggyExpression = 1;
                            hasError = 1;
                        }
                        break;
                    }

                    default: break;

                }

                if ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {
                    (*tokenPointer) = (*tokenPointer)->nextPointer;
                }

            }

            if (!buggyExpression) {
                if ((*tokenPointer) != NULL) {
                    if (isStackEmpty(topParStack)) {
                        int result = calculateIntStack(&topCalculatorStack, tokenPointer);
                        char *string = (char*) malloc(sizeof(char)*50);
                        sprintf(string, "%d", result);
                        symbol->value = string;
                        insertSymbolToTable(headSymbol, symbol);
                    } else {
                        messageError(*tokenPointer, "Invalid math expression! It's open parantheses are more than the close ones.");
                        skipToToken(tokenPointer, ";");
                        hasError = 1;
                    }
                } else {
                    printf("Invalid math expression was found at the end of the file! Needed a semicolon...\n");
                    hasError = 1;
                    exit(1);
                }
            }

            break;
        }

        case FLOATK: {

            while ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {

                token = (*tokenPointer)->text;
                switch (currentState) {

                    // Was NIY or operator
                    case 1: {
                        if (isExpression(token)) {
                            VariableKeyword expressionType = getExpressionType(tokenPointer, *headSymbol);
                            if (expressionType==FLOATK || expressionType==INTK)  {
                                if (isValidIdentifier(token)) {
                                    token = getSymbolFromTable(*headSymbol, token)->value;
                                }
                                pushToStack(&topCalculatorStack, token);
                                currentState = 0; // Wants Operator or )
                            } else if (strcmp(token, "NULL") == 0) {
                                pushToStack(&topCalculatorStack, "0");
                                currentState = 0; // Wants Operator or )
                            } else {
                                unexpectedTokenException(*tokenPointer, "Expected a float");
                                skipToToken(tokenPointer, ";");
                                emptyStack(&topParStack);
                                emptyStack(&topCalculatorStack);
                                emptyStack(&topPrecedenceStack);
                                buggyExpression = 1;
                                hasError = 1;
                            }
                        } else if (strcmp(token, "(") == 0) {
                            pushToStack(&topCalculatorStack, token);
                            pushToStack(&topParStack, token);
                            char *pre = (char*) malloc(sizeof(char)*10);
                            sprintf(pre, "%d", currentPrecedence);
                            pushToStack(&topPrecedenceStack, pre);
                            currentPrecedence = -1;
                            currentState = 1; // Wants Number
                        } else {
                            unexpectedTokenException(*tokenPointer, "Expected ( or an expression");
                            skipToToken(tokenPointer, ";");
                            hasError = 1;
                            emptyStack(&topParStack);
                            emptyStack(&topCalculatorStack);
                            emptyStack(&topPrecedenceStack);
                            buggyExpression = 1;
                        }
                        break;
                    }

                        // Was Number
                    case 0: {
                        if (isOperator(token)) {
                            if (getPrecedence(token) < currentPrecedence) {
                                // Calculate and push result to stack + operator and update precedence
                                double result = calculateFloatStack(&topCalculatorStack, tokenPointer);
                                if ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {
                                    char *pusher = (char*) malloc(sizeof(char)*50);
                                    sprintf(pusher, "%f", result);
                                    pushToStack(&topCalculatorStack, pusher); // pushing result
                                    pushToStack(&topCalculatorStack, token); // pushing operator
                                    currentPrecedence = getPrecedence(token);
                                    currentState = 1; // Wants a number
                                } else {
                                    emptyStack(&topParStack);
                                    emptyStack(&topCalculatorStack);
                                    emptyStack(&topPrecedenceStack);
                                    buggyExpression = 1;
                                }
                            } else {
                                currentPrecedence = getPrecedence(token);
                                pushToStack(&topCalculatorStack, token);
                                currentState = 1; // Wants a number
                            }
                        } else if (strcmp(token, ")") == 0) {
                            // Calculate and pop from par stack and recover the precedence and push result to stack
                            // Should reach '('
                            double result = calculateFloatParStack(&topCalculatorStack, tokenPointer);
                            if ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {
                                popFromStack(&topCalculatorStack); // Popping '('
                                popFromStack(&topParStack); // Popping '('
                                char *pusher = (char*) malloc(sizeof(char)*50);
                                sprintf(pusher, "%f", result);
                                pushToStack(&topCalculatorStack, pusher);
                                if (!isStackEmpty(topPrecedenceStack)) {
                                    currentPrecedence = atoi(popFromStack(&topPrecedenceStack));
                                    currentState = 0; // Wants an operator
                                } else {
                                    messageError(*tokenPointer, "WTF???");
                                    exit(1);
                                }
                            }
                        } else {
                            unexpectedTokenException(*tokenPointer, "Expected ) or an operator");
                            skipToToken(tokenPointer, ";");
                            hasError = 1;
                            emptyStack(&topParStack);
                            emptyStack(&topCalculatorStack);
                            emptyStack(&topPrecedenceStack);
                            buggyExpression = 1;
                        }
                        break;
                    }

                    default: break;

                }

                if ((*tokenPointer)!=NULL && strcmp((*tokenPointer)->text, ";")!=0) {
                    (*tokenPointer) = (*tokenPointer)->nextPointer;
                }

            }

            if (!buggyExpression) {
                if ((*tokenPointer) != NULL) {
                    if (isStackEmpty(topParStack)) {
                        double result = calculateFloatStack(&topCalculatorStack, tokenPointer);
                        char *string = (char*) malloc(sizeof(char)*50);
                        sprintf(string, "%f", result);
                        symbol->value = string;
                        insertSymbolToTable(headSymbol, symbol);
                    } else {
                        messageError(*tokenPointer, "Invalid math expression! It's open parantheses are more than the close ones.");
                        skipToToken(tokenPointer, ";");
                        hasError = 1;
                    }
                } else {
                    printf("Invalid math expression was found at the end of the file! Needed a semicolon...\n");
                    hasError = 1;
                    exit(1);
                }
            }

            break;

        }

        case BOOLK: {
            token = (*tokenPointer)->text;
            if (isExpression(token)) {
                if (isBoolean(token) || strcmp(token, "NULL")==0) {
                    if (strcmp(token, "NULL") == 0) token = "0";
                    if ((*tokenPointer)->nextPointer != NULL) {
                        if (strcmp((*tokenPointer)->nextPointer->text, ";") == 0) {
                            symbol->value = token;
                            insertSymbolToTable(headSymbol, symbol);
                            (*tokenPointer) = (*tokenPointer)->nextPointer; // Push it on semicolon
                        } else {
                            unexpectedTokenException(*tokenPointer, "Expected a semicolon");
                            skipToToken(tokenPointer, ";");
                            hasError = 1;
                        }
                    } else {
                        messageError(*tokenPointer, "Expected a semicolon but found nothing!");
                        hasError = 1;
                        exit(1);
                    }
                } else if (isValidIdentifier(token)) {
                    SymbolPointer symbolPointer = getSymbolFromTable(*headSymbol, token);
                    if (symbolPointer!=NULL && symbolPointer->type==BOOLK) {
                        if (symbolPointer->value != NULL) {
                            if ((*tokenPointer)->nextPointer != NULL) {
                                if (strcmp((*tokenPointer)->nextPointer->text, ";") == 0) {
                                    symbol->value = token;
                                    insertSymbolToTable(headSymbol, symbol);
                                    (*tokenPointer) = (*tokenPointer)->nextPointer; // Push it on semicolon
                                } else {
                                    unexpectedTokenException(*tokenPointer, "Expected a semicolon");
                                    skipToToken(tokenPointer, ";");
                                    hasError = 1;
                                }
                            } else {
                                messageError(*tokenPointer, "Expected a semicolon but found nothing!");
                                hasError = 1;
                                exit(1);
                            }
                        } else {
                            messageError(*tokenPointer, "The value of this variable isn't defined yet!");
                            skipToToken(tokenPointer, ";");
                            hasError = 1;
                        }
                    } else {
                        unexpectedTokenException(*tokenPointer, "Type missmatch! Expected a boolean");
                        skipToToken(tokenPointer, ";");
                        hasError = 1;
                    }
                } else {
                    unexpectedTokenException(*tokenPointer, "Type missmatch! Expected a boolean");
                    skipToToken(tokenPointer, ";");
                    hasError = 1;
                }
            } else {
                unexpectedTokenException(*tokenPointer, "Expected an expression");
                skipToToken(tokenPointer, ";");
                hasError = 1;
            }
            break;
        }

        case CHARK: {
            token = (*tokenPointer)->text;
            if (isExpression(token)) {
                if (isCharacterAssignment(token) || strcmp(token, "NULL")==0) {
                    if (strcmp(token, "NULL") == 0) token = "\'0\'";
                    if ((*tokenPointer)->nextPointer != NULL) {
                        if (strcmp((*tokenPointer)->nextPointer->text, ";") == 0) {
                            symbol->value = token;
                            insertSymbolToTable(headSymbol, symbol);
                            (*tokenPointer) = (*tokenPointer)->nextPointer; // Push it on semicolon
                        } else {
                            unexpectedTokenException(*tokenPointer, "Expected a semicolon");
                            skipToToken(tokenPointer, ";");
                            hasError = 1;
                        }
                    } else {
                        messageError(*tokenPointer, "Expected a semicolon but found nothing!");
                        hasError = 1;
                        exit(1);
                    }
                } else if (isValidIdentifier(token)) {
                    SymbolPointer symbolPointer = getSymbolFromTable(*headSymbol, token);
                    if (symbolPointer!=NULL && symbolPointer->type==CHARK) {
                        if (symbolPointer->value != NULL) {
                            if ((*tokenPointer)->nextPointer != NULL) {
                                if (strcmp((*tokenPointer)->nextPointer->text, ";") == 0) {
                                    symbol->value = token;
                                    insertSymbolToTable(headSymbol, symbol);
                                    (*tokenPointer) = (*tokenPointer)->nextPointer; // Push it on semicolon
                                } else {
                                    unexpectedTokenException(*tokenPointer, "Expected a semicolon");
                                    skipToToken(tokenPointer, ";");
                                    hasError = 1;
                                }
                            } else {
                                messageError(*tokenPointer, "Expected a semicolon but found nothing!");
                                hasError = 1;
                                exit(1);
                            }
                        } else {
                            messageError(*tokenPointer, "The value of this variable isn't defined yet!");
                            skipToToken(tokenPointer, ";");
                            hasError = 1;
                        }
                    } else {
                        unexpectedTokenException(*tokenPointer, "Type missmatch! Expected a character");
                        skipToToken(tokenPointer, ";");
                        hasError = 1;
                    }
                } else {
                    unexpectedTokenException(*tokenPointer, "Type missmatch! Expected a character");
                    skipToToken(tokenPointer, ";");
                    hasError = 1;
                }
            } else {
                unexpectedTokenException(*tokenPointer, "Expected an expression");
                skipToToken(tokenPointer, ";");
                hasError = 1;
            }
            break;
        }

    }

}

int calculateIntParStack(StackNodePointer *topCalculatorNode, TokenPointer *currentToken) {

    if (!isStackEmpty(*topCalculatorNode)) {
        if (strcmp((*topCalculatorNode)->data, "(") != 0) {
            char* result = (char*) malloc(sizeof(char)*50);
            int a = atoi(popFromStack(topCalculatorNode));
            if (!isStackEmpty(*topCalculatorNode)) {
                if (strcmp((*topCalculatorNode)->data, "(")!=0) {
                    char *operation = popFromStack(topCalculatorNode);
                    int b = atoi(popFromStack(topCalculatorNode));
                    if (strcmp(operation, "+") == 0) {
                        sprintf(result ,"%d", b + a);
                        pushToStack(topCalculatorNode, result);
                        return calculateIntParStack(topCalculatorNode, currentToken);
                    } else if (strcmp(operation, "*") == 0) {
                        sprintf(result ,"%d", b * a);
                        pushToStack(topCalculatorNode, result);
                        return calculateIntParStack(topCalculatorNode, currentToken);
                    } else if (strcmp(operation, "-") == 0) {
                        sprintf(result ,"%d", b - a);
                        pushToStack(topCalculatorNode, result);
                        return calculateIntParStack(topCalculatorNode, currentToken);
                    } else if (strcmp(operation, "/") == 0) {
                        if (a != 0) {
                            sprintf(result ,"%d", b / a);
                            pushToStack(topCalculatorNode, result);
                            return calculateIntParStack(topCalculatorNode, currentToken);
                        } else {
                            messageError(*currentToken, "Division by zero isn't allowed!");
                            skipToToken(currentToken, ";");
                            hasError = 1;
                            return 0;
                        }
                    } else {
                        messageError(*currentToken, "Invalid Operator?");
                        printf("Operator: %s\n", operation);
                        skipToToken(currentToken, ";");
                        hasError = 1;
                        return 0;
                    }
                } else return a;
            } else {
                messageError(*currentToken, "Invalid math expression! Closed \'(\' without any opening \')\'!");
                skipToToken(currentToken, ";");
                hasError = 1;
                return 0;
            }
        } else {
            messageError(*currentToken, "Invalid math expression! Found Empty\'()\'");
            skipToToken(currentToken, ";");
            hasError = 1;
            return 0;
        }
    } else {
        messageError(*currentToken, "Invalid math expression! Found \')\' without any \'(\' before it.");
        skipToToken(currentToken, ";");
        hasError = 1;
        return 0;
    }

}

int calculateIntStack(StackNodePointer *topCalculatorNode, TokenPointer *currentToken) {

    if (!isStackEmpty(*topCalculatorNode) && strcmp((*topCalculatorNode)->data, "(")!=0) {

        char* result = (char*) malloc(sizeof(char)*50);
        int a = atoi(popFromStack(topCalculatorNode));
        if (!isStackEmpty(*topCalculatorNode) && strcmp((*topCalculatorNode)->data, "(")!=0) {
            char *operation = popFromStack(topCalculatorNode);
            int b = atoi(popFromStack(topCalculatorNode));
            if (strcmp(operation, "+") == 0) {
                sprintf(result ,"%d", b + a);
                pushToStack(topCalculatorNode, result);
                return calculateIntStack(topCalculatorNode, currentToken);
            } else if (strcmp(operation, "*") == 0) {
                sprintf(result ,"%d", b * a);
                pushToStack(topCalculatorNode, result);
                return calculateIntStack(topCalculatorNode, currentToken);
            } else if (strcmp(operation, "-") == 0) {
                sprintf(result ,"%d", b - a);
                pushToStack(topCalculatorNode, result);
                return calculateIntStack(topCalculatorNode, currentToken);
            } else if (strcmp(operation, "/") == 0) {
                if (a != 0) {
                    sprintf(result ,"%d", b / a);
                    pushToStack(topCalculatorNode, result);
                    return calculateIntStack(topCalculatorNode, currentToken);
                } else {
                    messageError(*currentToken, "Division by zero isn't allowed!");
                    skipToToken(currentToken, ";");
                    hasError = 1;
                    return 0;
                }
            } else {
                messageError(*currentToken, "Invalid Operator?");
                printf("Operator: %s\n", operation);
                skipToToken(currentToken, ";");
                hasError = 1;
                return 0;
            }
        } else return a;

    } else {
        messageError(*currentToken, "Invalid expression!");
        skipToToken(currentToken, ";");
        hasError = 1;
        return 0;
    }

}

double calculateFloatParStack(StackNodePointer *topCalculatorNode, TokenPointer *currentToken) {

    if (!isStackEmpty(*topCalculatorNode)) {
        if (strcmp((*topCalculatorNode)->data, "(") != 0) {
            char* result = (char*) malloc(sizeof(char)*50);
            double a = atof(popFromStack(topCalculatorNode));
            if (!isStackEmpty(*topCalculatorNode)) {
                if (strcmp((*topCalculatorNode)->data, "(")!=0) {
                    char *operation = popFromStack(topCalculatorNode);
                    double b = atof(popFromStack(topCalculatorNode));
                    if (strcmp(operation, "+") == 0) {
                        sprintf(result ,"%f", b + a);
                        pushToStack(topCalculatorNode, result);
                        return calculateFloatParStack(topCalculatorNode, currentToken);
                    } else if (strcmp(operation, "*") == 0) {
                        sprintf(result ,"%f", b * a);
                        pushToStack(topCalculatorNode, result);
                        return calculateFloatParStack(topCalculatorNode, currentToken);
                    } else if (strcmp(operation, "-") == 0) {
                        sprintf(result ,"%f", b - a);
                        pushToStack(topCalculatorNode, result);
                        return calculateFloatParStack(topCalculatorNode, currentToken);
                    } else if (strcmp(operation, "/") == 0) {
                        if (a != 0) {
                            sprintf(result ,"%f", b / a);
                            pushToStack(topCalculatorNode, result);
                            return calculateFloatParStack(topCalculatorNode, currentToken);
                        } else {
                            messageError(*currentToken, "Division by zero isn't allowed!");
                            skipToToken(currentToken, ";");
                            hasError = 1;
                            return 0;
                        }
                    } else {
                        messageError(*currentToken, "Invalid Operator?");
                        printf("Operator: %s\n", operation); // TODO DEL
                        skipToToken(currentToken, ";");
                        hasError = 1;
                        return 0;
                    }
                } else return a;
            } else {
                messageError(*currentToken, "Invalid math expression! Closed \'(\' without any opening \')\'!");
                skipToToken(currentToken, ";");
                hasError = 1;
                return 0;
            }
        } else {
            messageError(*currentToken, "Invalid math expression! Found Empty\'()\'");
            skipToToken(currentToken, ";");
            hasError = 1;
            return 0;
        }
    } else {
        messageError(*currentToken, "Invalid math expression! Found \')\' without any \'(\' before it.");
        skipToToken(currentToken, ";");
        hasError = 1;
        return 0;
    }

}

double calculateFloatStack(StackNodePointer *topCalculatorNode, TokenPointer *currentToken) {

    if (!isStackEmpty(*topCalculatorNode) && strcmp((*topCalculatorNode)->data, "(")!=0) {

        char* result = (char*) malloc(sizeof(char)*50);
        double a = atof(popFromStack(topCalculatorNode));
        if (!isStackEmpty(*topCalculatorNode) && strcmp((*topCalculatorNode)->data, "(")!=0) {
            char *operation = popFromStack(topCalculatorNode);
            double b = atof(popFromStack(topCalculatorNode));
            if (strcmp(operation, "+") == 0) {
                sprintf(result ,"%f", b + a);
                pushToStack(topCalculatorNode, result);
                return calculateFloatStack(topCalculatorNode, currentToken);
            } else if (strcmp(operation, "*") == 0) {
                sprintf(result ,"%f", b * a);
                pushToStack(topCalculatorNode, result);
                return calculateFloatStack(topCalculatorNode, currentToken);
            } else if (strcmp(operation, "-") == 0) {
                sprintf(result ,"%f", b - a);
                pushToStack(topCalculatorNode, result);
                return calculateFloatStack(topCalculatorNode, currentToken);
            } else if (strcmp(operation, "/") == 0) {
                if (a != 0) {
                    sprintf(result ,"%f", b / a);
                    pushToStack(topCalculatorNode, result);
                    return calculateFloatStack(topCalculatorNode, currentToken);
                } else {
                    messageError(*currentToken, "Division by zero isn't allowed!");
                    skipToToken(currentToken, ";");
                    hasError = 1;
                    return 0;
                }
            } else {
                messageError(*currentToken, "Invalid Operator?");
                printf("Operator: %s\n", operation); // TODO DEL
                skipToToken(currentToken, ";");
                hasError = 1;
                return 0;
            }
        } else return a;

    } else {
        messageError(*currentToken, "Invalid expression!");
        skipToToken(currentToken, ";");
        hasError = 1;
        return 0;
    }

}

int isIRVariable(char *token) {
    if (token[0] == 'T') {
        if (token[1] == '_') {
            token += 2;
            return isInteger(token);
        } else {
            token++;
            return isInteger(token);
        }
    } else return 0;
}

char *generateParStackIR(StackNodePointer *topCalculatorNode, SymbolPointer headSymbol, int *tempIndex, int preTabs) {

    char *result = (char*) malloc(sizeof(char)*50);
    char *token1 = popFromStack(topCalculatorNode);
    if (!isIRVariable(token1)) {
        if (isValidIdentifier(token1)) {
            sprintf(token1, "T%d", getSymbolFromTable(headSymbol, token1)->index);
        } else {
            printf("%sT_%d = %s\n", buildPreTabsString(preTabs), (*tempIndex)++, token1);
            sprintf(token1, "T_%d", (*tempIndex)-1);
        }
    }

    if (strcmp((*topCalculatorNode)->data, "(")!=0) {

        char *operation = popFromStack(topCalculatorNode);
        char *token2 = popFromStack(topCalculatorNode);
        if (!isIRVariable(token2)) {
            if (isValidIdentifier(token2)) {
                sprintf(token2, "T%d", getSymbolFromTable(headSymbol, token2)->index);
            } else {
                printf("%sT_%d = %s\n", buildPreTabsString(preTabs), (*tempIndex)++, token2);
                sprintf(token2, "T_%d", (*tempIndex)-1);
            }
        }

        printf("%sT_%d = %s %s %s\n", buildPreTabsString(preTabs), (*tempIndex)++, token1, operation, token2);
        sprintf(result, "T_%d", (*tempIndex)-1);
        pushToStack(topCalculatorNode, result);
        return generateParStackIR(topCalculatorNode, headSymbol, tempIndex, preTabs);

    } else return token1;

}



char *generateStackIR(StackNodePointer *topCalculatorNode, SymbolPointer headSymbol, int *tempIndex, int preTabs) {

    char *result = (char*) malloc(sizeof(char)*50);
    char *token1 = popFromStack(topCalculatorNode);
    if (!isIRVariable(token1)) {
        if (isValidIdentifier(token1)) {
            sprintf(token1, "T%d", getSymbolFromTable(headSymbol, token1)->index);
        } else {
            printf("%sT_%d = %s\n", buildPreTabsString(preTabs), (*tempIndex)++, token1);
            sprintf(token1, "T_%d", (*tempIndex)-1);
        }
    }

    if (!isStackEmpty(*topCalculatorNode) && strcmp((*topCalculatorNode)->data, "(")!=0) {

        char *operation = popFromStack(topCalculatorNode);
        char *token2 = popFromStack(topCalculatorNode);
        if (!isIRVariable(token2)) {
            if (isValidIdentifier(token2)) {
                sprintf(token2, "T%d", getSymbolFromTable(headSymbol, token2)->index);
            } else {
                printf("%sT_%d = %s\n", buildPreTabsString(preTabs), (*tempIndex)++, token2);
                sprintf(token2, "T_%d", (*tempIndex)-1);
            }
        }

        printf("%sT_%d = %s %s %s\n", buildPreTabsString(preTabs), (*tempIndex)++, token1, operation, token2);
        sprintf(result, "T_%d", (*tempIndex)-1);
        pushToStack(topCalculatorNode, result);
        return generateStackIR(topCalculatorNode, headSymbol, tempIndex, preTabs);

    } else return token1;

}

VariableKeyword getExpressionType(TokenPointer *tokenPointer, SymbolPointer headSymbol) {
    char *token = (*tokenPointer)->text;
    if (isValidIdentifier(token)) {
        SymbolPointer symbol = getSymbolFromTable(headSymbol, token);
        if (symbol == NULL) {
            messageError(*tokenPointer, "Using undeclared variable");
            skipToToken(tokenPointer, ";");
            hasError = 1;
            return NULLK;
        } else if (symbol->value == NULL) {
            unexpectedTokenException(*tokenPointer, "The value of the variable isn't defined yet, expected an expression");
            skipToToken(tokenPointer, ";");
            hasError = 1;
            return NULLK;
        } else return symbol->type;
    } else if (isInteger(token)) {
        return INTK;
    } else if (isNumber(token)) {
        return FLOATK;
    } else if (isCharacterAssignment(token)) {
        return CHARK;
    } else if (isBoolean(token)) {
        return BOOLK;
    } else return NULLK;
}

void checkConditionalExpression(TokenPointer *tokenPointer, SymbolPointer headSymbol) {

    if (isExpression((*tokenPointer)->text)) {
        (*tokenPointer) = (*tokenPointer)->nextPointer;
        if (isConditionalOperator((*tokenPointer)->text)) {
            (*tokenPointer) = (*tokenPointer)->nextPointer;
            if (isExpression((*tokenPointer)->text)) {
                (*tokenPointer) = (*tokenPointer)->nextPointer;
                if (strcmp((*tokenPointer)->text, ")") == 0) {
                    (*tokenPointer) = (*tokenPointer)->nextPointer;
                    if (strcmp((*tokenPointer)->text, "{") == 0) {
                        return;
                    } else {
                        unexpectedTokenException(*tokenPointer, "Expected {");
                        skipToToken(tokenPointer, "{"); // Todo
                        hasError = 1;
                    }
                } else {
                    unexpectedTokenException(*tokenPointer, "Expected (");
                    skipToToken(tokenPointer, "{"); // Todo
                    hasError = 1;
                }
            } else {
                unexpectedTokenException(*tokenPointer, "Expected an expression");
                skipToToken(tokenPointer, "{");
                hasError = 1;
            }
        } else if (strcmp((*tokenPointer)->text, ")")) {
            (*tokenPointer) = (*tokenPointer)->nextPointer;
            if (strcmp((*tokenPointer)->text, "{") == 0) {
                return;
            } else {
                unexpectedTokenException(*tokenPointer, "Expected {");
                skipToToken(tokenPointer, "{"); // Todo
                hasError = 1;
            }
        } else {
            unexpectedTokenException(*tokenPointer, "Expected a conditional operator or )");
            skipToToken(tokenPointer, "{");
            hasError = 1;
        }
    } else {
        unexpectedTokenException(*tokenPointer, "Expected an expression");
        skipToToken(tokenPointer, "{");
        hasError = 1;
    }

}

int isExpression(char *token) {
    if (strcmp(token, "NULL") == 0) {
        return 1;
    } else if (strcmp(token, "true") == 0) {
        return 1;
    } else if (strcmp(token, "false") == 0) {
        return 1;
    } else if (isCharacterAssignment(token)) {
        return 1;
    } else if (isNumber(token)) {
        return 1;
    } else if (isValidIdentifier(token)) {
        return 1;
    } else return 0;
}

int isCharacterAssignment(char *token) {
    if (*token == '\'' && *(token+2) == '\'') return 1;
    else return 0;
}

int isNumber(char *data) {
    int dots = 0;
    while (*data != NULL) {
        if (*data == '.') dots++;
        else if (!isdigit(*data)) return 0;
        if (dots > 1) return 0;
        data++;
    }
    return 1;
}

int isInteger(char *data) {
    while (*data != NULL) {
        if (!isdigit(*data)) return 0;
        data++;
    }
    return 1;
}

int isBoolean(char *token) {
    return
            strcmp(token, "true")==0 ||
            strcmp(token, "1")==0 ||
            strcmp(token, "0")==0 ||
            strcmp(token, "false")==0;
}

int isConditionalOperator(char *data) {
    return
            strcmp(data, "&&") == 0 ||
            strcmp(data, "||") == 0 ||
            strcmp(data, "==") == 0 ||
            strcmp(data, "!=") == 0 ||
            strcmp(data, ">") == 0 ||
            strcmp(data, "<") == 0 ||
            strcmp(data, ">=") == 0 ||
            strcmp(data, "<=") == 0 ||
            strcmp(data, "+") == 0 ||
            strcmp(data, "-") == 0 ||
            strcmp(data, "*") == 0 ||
            strcmp(data, "/") == 0;
}

int isOperator(char *data) {
    return
            strcmp(data, "+") == 0 ||
            strcmp(data, "-") == 0 ||
            strcmp(data, "*") == 0 ||
            strcmp(data, "/") == 0;
}

int getPrecedence(char *operator) {
    if (strcmp(operator, "+") == 0) return 1;
    else if (strcmp(operator, "-") == 0) return 1;
    else if (strcmp(operator, "/") == 0) return 2;
    else if (strcmp(operator, "*") == 0) return 2;
}

void messageError(TokenPointer tokenPointer, char *message) {
    printf("Error at line %d, %s\n", tokenPointer->lineNumber, message);
}

void unexpectedTokenException(TokenPointer tokenPointer, char *description) {
    printf("Error at line %d. %s but found %s\n", tokenPointer->lineNumber, description, tokenPointer->text);
}

void insertSymbolToTable(SymbolPointer *headSymbol, SymbolPointer symbol) {
    int index = 1;
    SymbolPointer temp;
    if (*headSymbol == NULL) {
        symbol->index = 0;
        symbol->nextPointer = NULL;
        *headSymbol = symbol;
    } else {
        int hasAdded = 0;
        SymbolPointer tmp = *headSymbol;
        while (tmp != NULL) {
            if (strcmp(tmp->name, symbol->name) == 0) {
                tmp->value = symbol->value;
                hasAdded = 1;
                break;
            }
            tmp = tmp->nextPointer;
        }
        if (!hasAdded) {
            temp = *headSymbol;
            while (temp->nextPointer != NULL) {
                temp = temp->nextPointer;
                index++;
            }
            symbol->index = index;
            symbol->nextPointer = NULL;
            temp->nextPointer = symbol;
        }
    }
}

SymbolPointer getSymbolFromTable(SymbolPointer symbol, char *name) {
    while (symbol != NULL) {
        if (strcmp(symbol->name, name) == 0) return symbol;
        symbol = symbol->nextPointer;
    }
    return NULL;
}

SymbolPointer createSymbolPointer() {
    SymbolPointer symbol = (SymbolPointer) malloc(sizeof(Symbol));
    symbol->value = NULL;
    return symbol;
}

int isValidKeyword(char *token) {
    return
            strcmp(token, "int") == 0 ||
            strcmp(token, "float") == 0 ||
            strcmp(token, "bool") == 0 ||
            strcmp(token, "char") == 0 ||
            strcmp(token, "if") == 0 ||
            strcmp(token, "while") == 0 ||
            strcmp(token, "else") == 0 ||
            strcmp(token, "main") == 0 ||
            strcmp(token, "void") == 0 ||
            strcmp(token, "null") == 0;
}

int isVariableKeyword(char *token) {
    return
            strcmp(token, "int") == 0 ||
            strcmp(token, "float") == 0 ||
            strcmp(token, "bool") == 0 ||
            strcmp(token, "char") == 0;
}
Keyword getKeyword(char *token) {
    if (strcmp(token, "int") == 0) return INT;
    else if (strcmp(token, "float") == 0) return FLOAT;
    else if (strcmp(token, "char") == 0) return CHAR;
    else if (strcmp(token, "bool") == 0) return BOOL;
    else if (strcmp(token, "if") == 0) return IF;
    else if (strcmp(token, "while") == 0) return WHILE;
    else if (strcmp(token, "else") == 0) return ELSE;
    else if (strcmp(token, "main") == 0) return MAIN;
    else if (strcmp(token, "void") == 0) return VOID;
    else if (strcmp(token, "null") == 0) return NUL;
}

VariableKeyword getVariableKeyword(char *token) {
    if (strcmp(token, "int") == 0) return (VariableKeyword) INTK;
    else if (strcmp(token, "float") == 0) return (VariableKeyword) FLOATK;
    else if (strcmp(token, "char") == 0) return (VariableKeyword) CHARK;
    else if (strcmp(token, "bool") == 0) return (VariableKeyword) BOOLK;
}

int isValidIdentifier(char *token) {
    if (isValidKeyword(token)) return 0;
    else if (strcmp(token, "return") == 0) return 0;
    if (isdigit(*token)) return 0;
    else {
        while (*token != '\0') {
            if (!isalnum(*token) && *token!='_') return 0;
            token++;
        }
        return 1;
    }
}

void pushToStack(StackNodePointer *topPointer, char *data) {
    StackNodePointer newPointer = (StackNodePointer) malloc(sizeof(StackNode));
    strcpy(newPointer->data, data);
    newPointer->nextPointer = *topPointer;
    *topPointer = newPointer;
}

char *popFromStack(StackNodePointer *topPointer) {
    StackNodePointer tmp = *topPointer;
    char *result = tmp->data;
    *topPointer = (*topPointer)->nextPointer;
    return result;
}

int isStackEmpty(StackNodePointer topPointer) {
    return topPointer == NULL;
}

void emptyStack(StackNodePointer *topPointer) {
    while (!isStackEmpty(*topPointer)) popFromStack(topPointer);
}

void pushToConditionStack(ConditionStackNodePointer *topPointer, int index, int elseIndex, int isWhile) {
    ConditionStackNodePointer newPointer = (ConditionStackNodePointer) malloc(sizeof(ConditionStackNode));
    newPointer->index = index;
    newPointer->isWhile = isWhile;
    newPointer->elseIndex = elseIndex;
    newPointer->nextPointer = *topPointer;
    *topPointer = newPointer;
}

ConditionStackNode popFromConditionStack(ConditionStackNodePointer *topPointer) {
    ConditionStackNode tmp = **topPointer;
    *topPointer = (*topPointer)->nextPointer;
    return tmp;
}

int isConditionStackEmpty(ConditionStackNodePointer topPointer) {
    return topPointer == NULL;
}
