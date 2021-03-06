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
    INTK, FLOATK, CHARK, BOOLK
};

struct Token {
    char *text;
    int lineNumber;
    struct Token *nextPointer;
};

struct StackNode {
    char data[35];
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
void messageError(TokenPointer, char*);
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
int isStackEmpty(StackNodePointer);
SymbolPointer createSymbolPointer();
char *popFromStack(StackNodePointer*);
void messageError(TokenPointer, char*);
VariableKeyword getVariableKeyword(char*);
void pushToStack(StackNodePointer*, char*);
int syntaxAnalyze(TokenPointer, SymbolPointer*);
void unexpectedTokenException(TokenPointer, char*);
SymbolPointer getSymbolFromTable(SymbolPointer, char*);
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

void preprocessor();
void save(Token* head);
void loadToken(Token * head);
void printList(Token * head);
void loadSecond(Token * head);
void tokenizer(TokenPointer*);
void load(Token*, char*);

void skipToToken(TokenPointer*, char*);

int hasError = 0;

int main() {

    TokenPointer headToken = NULL;
    SymbolPointer headSymbol = NULL;

    preprocessor();
    tokenizer(&headToken);

    syntaxAnalyze(headToken->nextPointer, &headSymbol);
    if (!hasError){
        generateIRCode(headToken->nextPointer, headSymbol);
    }

    return 0;

}

int syntaxAnalyze(TokenPointer currentToken, SymbolPointer *headSymbol) {

    char *token;
    State currentState = NIY;
    ScopeState scopeState = START;

    TokenPointer temp;
    SymbolPointer symbol;
    int mainHasReturned=0;
    ConditionStackNodePointer conditionTopPointer = NULL;

    while (currentToken != NULL) {

        token = currentToken->text;
        switch (currentState) {

            case NIY: {
                if (isValidKeyword(token)) {
                    if (isVariableKeyword(token)) {
                        if (getVariableKeyword(token)==INTK && strcmp(currentToken->nextPointer->text, "main")==0) {
                            if (scopeState == START) {
                                temp = currentToken->nextPointer->nextPointer;
                                if (strcmp(temp->text, "(") == 0) {
                                    temp = temp->nextPointer;
                                    if (strcmp(temp->text, ")") == 0) {
                                        temp = temp->nextPointer;
                                        if (strcmp(temp->text, "{") == 0) {
                                            scopeState = INSIDE_MAIN;
                                            currentToken = temp;
                                        } else {
                                            unexpectedTokenException(currentToken, "Expected {");
                                            skipToToken(&currentToken, "{");
                                            hasError = 1 ;
                                        }
                                    } else {
                                        unexpectedTokenException(currentToken, "Expected )");
                                        skipToToken(&currentToken, ")"); // Todo
                                        hasError = 1 ;
                                    }
                                } else {
                                    unexpectedTokenException(currentToken, "Expected (");
                                    skipToToken(&currentToken, "(");
                                    hasError = 1 ;
                                }
                            } else {
                                messageError(currentToken, "int main() is defined already.");
                                skipToToken(&currentToken, ";");
                                hasError = 1 ;
                            }
                        } else {
                            symbol = createSymbolPointer();
                            symbol->type = getVariableKeyword(token);
                            currentState = KEYWORD; // Has Read Keyword, is going for Id.
                        }
                    } else if (strcmp(token, "if") == 0) {
                        if (scopeState == INSIDE_MAIN) {
                            if (strcmp(currentToken->nextPointer->text, "(") == 0) {
                                currentToken = currentToken->nextPointer;
                                currentState = IF_CONDITION;
                            } else {
                                unexpectedTokenException(currentToken->nextPointer, "Expected (");
                                skipToToken(&currentToken, "("); /// Todo
                                hasError = 1 ;
                            }
                        } else {
                            messageError(currentToken, "If is not allowed outside main!");
                            skipToToken(&currentToken, "{"); // Todo
                            hasError = 1 ;
                        }
                    } else if (strcmp(token, "while") == 0) {
                        if (scopeState == INSIDE_MAIN) {
                            if (strcmp(currentToken->nextPointer->text, "(") == 0) {
                                currentToken = currentToken->nextPointer;
                                currentState = WHILE_CONDITION;
                            } else {
                                unexpectedTokenException(currentToken->nextPointer, "Expected (");
                                skipToToken(&currentToken, "(");
                                hasError = 1 ;
                            }
                        } else {
                            messageError(currentToken, "While is not allowed outside main!");
                            skipToToken(&currentToken, "{");
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
                                    currentToken = currentToken->nextPointer;
                                    if (strcmp(currentToken->nextPointer->text, "{") == 0) {
                                        currentToken = currentToken->nextPointer;
                                        pushToConditionStack(&conditionTopPointer, 0, 0, 0);
                                    } else {
                                        unexpectedTokenException(currentToken, "Expected {");
                                        skipToToken(&currentToken, "{");
                                        hasError = 1 ;
                                    }
                                }
                            } // Nothing Special Was done... !
                        } // Nothing Special Was done... !
                    } else if (scopeState == INSIDE_MAIN) {
                        if (mainHasReturned) scopeState = START;
                        else {
                            messageError(currentToken, "Can't close the main function before returning.");
                            hasError = 1 ;
                        } // Todo
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
                    symbol->name = token;
                    currentState = ID;
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
                checkDefiniteExpression(&currentToken, headSymbol, symbol);
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
                if (isExpression(token)) {
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
                    unexpectedTokenException(currentToken, "Expected an expression after return");
                    skipToToken(&currentToken, ";");
                    hasError = 1 ;
                    currentState = NIY;
                }
                break;
            }

        }

        if (currentToken != NULL) currentToken = currentToken->nextPointer;

    }

    return hasError;

}

void skipToToken(TokenPointer *currentToken, char *token) {
    while ((*currentToken) != NULL) {
        if (strcmp((*currentToken)->text, token) == 0) break;
        (*currentToken) = (*currentToken)->nextPointer;
    }
}

void preprocessor(void) {
    Token * head;
    head = (Token*) malloc(sizeof (Token));
    head->text = "";
    head->nextPointer = NULL;
    loadSecond(head);
    save(head);
    return;
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

void load(Token* head, char a[]) {
    TokenPointer newToken;
    TokenPointer current;
    current = head;
    char *str;
    FILE *s;
    s = fopen(a, "r");
    if (s == NULL) {
        printf(" ERROR!\n");
        exit(1);
    }
    while (!feof(s)) {
        str = (char *) malloc(sizeof(char)*100);
        fgets(str, 100, s);
        newToken = (Token*) malloc(sizeof(Token));
        current->nextPointer = newToken;
        newToken->text = str;
        newToken->nextPointer = NULL;
        current = current->nextPointer;
    }
    newToken = (TokenPointer) malloc(sizeof(Token));
    newToken->nextPointer = NULL;
    current->nextPointer = newToken;
    newToken->text = "\n";
    fclose(s);
    return;
}

void loadSecond(Token * head) {
    TokenPointer current = head;
    TokenPointer newToken;
    char * str;
    char a[100], b[100], c, abbas[100];
    int i = 0, j = 0;
    FILE * p;
    p = fopen("main.c", "r");
    if (p == NULL) {
        printf("ERROR!\n");
        exit(1);
    }
    c = fgetc(p);
    if (c == '#') {
        fscanf(p, "%s", abbas);
        if (strcmp(abbas, "include") == 0) {
            fscanf(p, "%s", abbas);
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
                    load(head, b);
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
            return;
        }
    }

    fclose(p);
    return;
}

void tokenizer(TokenPointer *headToken) {
    *headToken = (TokenPointer) malloc(sizeof(Token));
    loadToken(*headToken);
}

void loadToken(Token * head) {
    Token * current, * newToken;
    char * str, * delim, *token, *newLine;
    int counter = 1;
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
        while (token != NULL) {
            newToken = (Token *) malloc(sizeof (Token));
            current->nextPointer = newToken;
            newToken->nextPointer = NULL;
            newToken->text = token;
            newToken->lineNumber = counter;
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
    ConditionStackNodePointer conditionTopPointer = NULL;

    while (currentToken != NULL) {

        token = currentToken->text;

        if (isValidKeyword(token)) {
            if (isVariableKeyword(token)) {
                if (getVariableKeyword(token)==INTK && strcmp(currentToken->nextPointer->text, "main")==0) {
                    while (strcmp(currentToken->text, "{") != 0) {
                        currentToken = currentToken->nextPointer;
                    }
                } else {
                    currentToken = currentToken->nextPointer; // Set tokenPointer to the id after the variable keyword
                    symbol = getSymbolFromTable(headSymbol, currentToken->text);
                    if (strcmp(currentToken->nextPointer->text, "=") == 0) {
                        currentToken = currentToken->nextPointer->nextPointer; // Set tokenPointer to after =
                        printf("%sT%d := %s\n", buildPreTabsString(preTabs), symbol->index, generateExpressionIRCode(&currentToken, headSymbol, preTabs));
                    } else {
                        while (strcmp(currentToken->text, ";") != 0) currentToken = currentToken->nextPointer;
                        // Sets the TokenPointer on the semicolon, Handles the comma
                    }
                }
            } else if (strcmp(token, "if") == 0) {

                currentToken = currentToken->nextPointer->nextPointer; // Set The TokenPointer to after (
                char conditionResult[50];
                strcpy(conditionResult, generateConditionExpressionIRCode(&currentToken, headSymbol, preTabs));
                currentToken = currentToken->nextPointer; // set the TokenPointer on {

                if (hasElse(currentToken->nextPointer)) {

                    int index=conditionIndex, elseIndex=index+1;
                    conditionIndex += 2;

                    printf("%sIF %s THEN L%d ELSE L%d\n", buildPreTabsString(preTabs), conditionResult, index, elseIndex);
                    printf("%sL%d:\n", buildPreTabsString(preTabs), index);
                    preTabs++;
                    pushToConditionStack(&conditionTopPointer, index, elseIndex, 0);

                } else {

                    int index = conditionIndex;
                    conditionIndex++;

                    printf("%sIF %s THEN L%d\n", buildPreTabsString(preTabs), conditionResult, index);
                    printf("%sL%d:\n", buildPreTabsString(preTabs), index);
                    preTabs++;
                    pushToConditionStack(&conditionTopPointer, index, -1, 0);

                }

            } else if (strcmp(token, "while") == 0) {

                int index = conditionIndex;
                conditionIndex++;

                printf("%sW%d:\n", buildPreTabsString(preTabs), index);
                preTabs++;

                currentToken = currentToken->nextPointer->nextPointer; // Set The TokenPointer to after (
                char conditionResult[50];
                strcpy(conditionResult, generateConditionExpressionIRCode(&currentToken, headSymbol, preTabs));
                currentToken = currentToken->nextPointer; // set the TokenPointer on {

                printf("%sIF %s THEN L%d\n", buildPreTabsString(preTabs), conditionResult, index);
                printf("%sL%d:\n", buildPreTabsString(preTabs), index);
                pushToConditionStack(&conditionTopPointer, index, -1, 1);
                preTabs++;

            }

        } else if (isValidIdentifier(token)) {
            symbol = getSymbolFromTable(headSymbol, token);
            currentToken = currentToken->nextPointer->nextPointer; // Set tokenPointer to after =
            printf("%sT%d := %s\n", buildPreTabsString(preTabs),
                   symbol->index, generateExpressionIRCode(&currentToken, headSymbol, preTabs));
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
            } // Else: it was main that was closed

        }

        currentToken = currentToken->nextPointer;

    }

}

char *generateExpressionIRCode(TokenPointer *tokenPointer, SymbolPointer headSymbol, int preTabs) {

    int index = 0;
    char *result = (char*) malloc(sizeof(char)*50);

    if (strcmp((*tokenPointer)->nextPointer->text, ";") == 0) {
        strcpy(result, (*tokenPointer)->text);
        (*tokenPointer) = (*tokenPointer)->nextPointer;
        if (strcmp(result, "NULL") == 0) result = 0;
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
            printf("%s%c%d := %s\n", buildPreTabsString(preTabs), 'R', index++, token1);
            sprintf(token1, "%c%d", 'R', index-1);
        } else {
            if (strcmp(token1, "NULL") == 0) token1 = 0;
            else sprintf(token1, "%c%d", 'T', getSymbolFromTable(headSymbol, token1)->index);
        }
        if (!isValidIdentifier(token2)) {
            printf("%s%c%d := %s\n", buildPreTabsString(preTabs), 'R', index++, token2);
            sprintf(token2, "%c%d", 'R', index-1);
        } else {
            if (strcmp(token2, "NULL") == 0) token2 = 0;
            else sprintf(token2, "%c%d", 'T', getSymbolFromTable(headSymbol, token2)->index);
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
        printf("%s%c%d := %s\n", buildPreTabsString(preTabs), 'R', index++, token1);
        sprintf(token1, "%c%d", 'R', index-1);
    } else {
        if (strcmp(token1, "NULL") == 0) token1 = 0;
        else sprintf(token1, "%c%d", 'T', getSymbolFromTable(headSymbol, token1)->index);
    }
    if (!isValidIdentifier(token2)) {
        printf("%s%c%d := %s\n", buildPreTabsString(preTabs), 'R', index++, token2);
        sprintf(token2, "%c%d", 'R', index-1);
    } else {
        if (strcmp(token2, "NULL") == 0) token2 = 0;
        else sprintf(token2, "%c%d", 'T', getSymbolFromTable(headSymbol, token2)->index);
    }

    sprintf(result, "%s %s %s", token1, op, token2);
    return result;

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

int

isConditionStackEmpty(ConditionStackNodePointer topPointer) {
    return topPointer == NULL;
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
        } else {
            unexpectedTokenException(*tokenPointer, "Expected a conditional operator");
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

// calculate an arithmetic expression and set currentToken to after semicolon
int calculateDefiniteExpression(TokenPointer *tokenPointer, SymbolPointer headSymbol) {

    // If readState is 0 then should read number
    int currentPrecedence=-1, readState;
    int openPars=0, closedPars=0;
    char *data, *token, *operator;
    double result;

    StackNodePointer topPointer;

    while (strcmp((*tokenPointer)->text, ";") != 0) {

        // Todo: handle openBars without closed ends...
        data = (*tokenPointer)->text;
        if (strcmp(data, "(") == 0) openPars++;
        else if (strcmp(data, ")") == 0) closedPars++;

        if (closedPars > openPars) {
            unexpectedTokenException(*tokenPointer, "Didn't expect ) before ( ");
        } else if (strcmp(data, ")") == 0) {

            result = readState = 0;
            while (strcmp((token = popFromStack(&topPointer)), "(") != 0) {
                if (isOperator(token) && readState==0) unexpectedTokenException(*tokenPointer, "Expeted a number");
                else if (isOperator(token)) {
                    readState = 0;
                    operator = token;
                } else if (isValidIdentifier(token) || isNumber(token)) {
                    if (readState == 0) {
                        readState = 1;
                        if (isOperator(operator)) {
                            if (strcmp(operator, "+") == 0) {
                                if (isNumber(token)) result += atof(token);
                                else result += atof(getSymbolFromTable(headSymbol, token)->value);
                            }
                            if (strcmp(operator, "-") == 0) {
                                if (isNumber(token)) result -= atof(token);
                                else result -= atof(getSymbolFromTable(headSymbol, token)->value);
                            }
                            if (strcmp(operator, "*") == 0) {
                                if (isNumber(token)) result *= atof(token);
                                else result *= atof(getSymbolFromTable(headSymbol, token)->value);
                            }
                            if (strcmp(operator, "/") == 0) {
                                if (isNumber(token)) result /= atof(token);
                                else result /= atof(getSymbolFromTable(headSymbol, token)->value);
                            }
                        } else {
                            if (isNumber(token)) result = atof(token);
                            else result = atof(getSymbolFromTable(headSymbol, token)->value);
                        }
                    } else unexpectedTokenException(*tokenPointer, "Expeted an operator");
                }
            }
            char *resultingString[50];
            sprintf(resultingString, "%f", result);
            pushToStack(&topPointer, resultingString);
        } else {
            if (isOperator(data) && getPrecedence(data)<currentPrecedence) {
                result = readState = 0;
                while (!isStackEmpty(topPointer)) {
                    token = popFromStack(&topPointer);
                    if (isOperator(token) && readState==0) unexpectedTokenException(*tokenPointer, "Expeted a number");
                    else if (isOperator(token)) {
                        readState = 0;
                        operator = token;
                    } else if (isValidIdentifier(token) || isNumber(token)) {
                        if (readState == 0) {
                            readState = 1;
                            if (isOperator(operator)) {
                                if (strcmp(operator, "+") == 0) {
                                    if (isNumber(token)) result += atof(token);
                                    else result += atof(getSymbolFromTable(headSymbol, token)->value);
                                }
                                if (strcmp(operator, "-") == 0) {
                                    if (isNumber(token)) result -= atof(token);
                                    else result -= atof(getSymbolFromTable(headSymbol, token)->value);
                                }
                                if (strcmp(operator, "*") == 0) {
                                    if (isNumber(token)) result *= atof(token);
                                    else result *= atof(getSymbolFromTable(headSymbol, token)->value);
                                }
                                if (strcmp(operator, "/") == 0) {
                                    if (isNumber(token)) result /= atof(token);
                                    else result /= atof(getSymbolFromTable(headSymbol, token)->value);
                                }
                            } else {
                                if (isNumber(token)) result = atof(token);
                                else result = atof(getSymbolFromTable(headSymbol, token)->value);
                            }
                        } else unexpectedTokenException(*tokenPointer, "Expeted an operator");
                    } else messageError(*tokenPointer, "Neither operation nor identifier nor number found in expression!");
                }
                char *resultingString[50];
                sprintf(resultingString, "%f", result);
                pushToStack(&topPointer, resultingString);
                pushToStack(&topPointer, data);
            } else pushToStack(&topPointer, data);
        }

        *tokenPointer = (*tokenPointer)->nextPointer;

    }

    if (openPars > closedPars) messageError(*tokenPointer, "Opened parenthese but didn't close them!");
    *tokenPointer = (*tokenPointer)->nextPointer;

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
    return (SymbolPointer) malloc(sizeof(Symbol));
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
    if (*topPointer == NULL) *topPointer = newPointer;
    else {
        newPointer->nextPointer = *topPointer;
        *topPointer = newPointer;
    }
}

char *popFromStack(StackNodePointer *topPointer) {
    StackNodePointer tmp = *topPointer;
    char *result = tmp->data;
    *topPointer = (*topPointer)->nextPointer;
    free(tmp);
    return result;
}

int isStackEmpty(StackNodePointer topPointer) {
    return topPointer == NULL;
}
