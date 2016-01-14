#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

enum ScopeState {
    START, INSIDE_MAIN
};
enum State {
    KEYWORD, ID, ASSIGN, IF_CONDITION, ELSE_CONDITION, RETURN, NIY
};
enum Keyword {
    INT, FLOAT, CHAR, BOOL, IF, ELSE, MAIN, VOID, NUL
};
enum VariableKeyword {
    INT, FLOAT, CHAR, BOOL
};

struct Token {
    char text[50];
    int lineNumber;
    struct Token *nextPointer;
};

struct Symbol {
    int index;
    char *name;
    char value[100];
    enum VariableKeyword type;
    struct Symbol *nextPointer;
};

typedef enum State State;
typedef struct Token Token;
typedef struct Symbol Symbol;
typedef enum Keyword Keyword;
typedef struct Token *TokenPointer;
typedef enum ScopeState ScopeState;
typedef struct Symbol *SymbolPointer;
typedef enum VariableKeyword VariableKeyword;

Keyword getKeyword(char*);
int isValidKeyword(char*);
int isVariableKeyword(char*);
int isValidIdentifier(char*);
TokenPointer createTokenPointer();
SymbolPointer createSymbolPointer();
void addIfToConditionStatement(int*);
void addElseToConditionStatement(int*);
void messageError(TokenPointer, char*);
void initializeConditionStatement(int*);
VariableKeyword getVariableKeyword(char*);
void removeLastFromConditionStatement(int*);
void syntaxAnalyze(TokenPointer, SymbolPointer*);
void unexpectedTokenException(TokenPointer, char*);
SymbolPointer getSymbolFromTable(SymbolPointer, char*);
void insertSymbolToTable(SymbolPointer*, SymbolPointer);

int main() {

    TokenPointer headToken = createTokenPointer();
    SymbolPointer headSymbol;
    // Let's Assume we have tokens' list (The preprocessor and LexicalAnalyzer have done their jobs)

    syntaxAnalyze(headToken, &headSymbol);

    return 0;

}

void syntaxAnalyze(TokenPointer currentToken, SymbolPointer *headSymbol) {

    char *token;
    State currentState = NIY;
    int conditionStatement = 0;
    ScopeState scopeState = START;

    TokenPointer temp;
    SymbolPointer symbol;
    int mainHasReturned=0, isAfterIfEnd=0;

    while (currentToken != NULL) {

        token = currentToken->text;
        switch (currentState) {

            case NIY: {
                if (isValidKeyword(token)) {
                    if (isVariableKeyword(token)) {
                        if (getVariableKeyword(token) == INT && strcmp(currentToken->nextPointer->text, "main")) {
                            if (scopeState == START) {
                                temp = currentToken->nextPointer->nextPointer;
                                if (strcmp(temp->text, "(") == 0) {
                                    temp = temp->nextPointer;
                                    if (strcmp(temp->text, ")") == 0) {
                                        temp = temp->nextPointer;
                                        if (strcmp(temp->text, "{") == 0) {
                                            scopeState = INSIDE_MAIN;
                                            currentToken = temp;
                                        } else unexpectedTokenException(token, "Expected }");
                                    } else unexpectedTokenException(token, "Expected )");
                                } else unexpectedTokenException(token, "Expected (");
                            } messageError(token, "int main() is defined already.");
                        } else {
                            symbol = createSymbolPointer();
                            symbol->type = getVariableKeyword(token);
                            currentState = KEYWORD; // Has Read Keyword, is going for Id.
                        }
                    } else if (strcmp(token, "if")) {
                        if (scopeState == INSIDE_MAIN) {
                            if (strcmp(currentToken->nextPointer->text, "(") == 0) {
                                currentToken = currentToken->nextPointer;
                                currentState = IF_CONDITION;
                            }
                        }
                    } else if (strcmp(token, "else") == 0) {
                        if (isAfterIfEnd) {
                            if (strcmp(currentToken->nextPointer->text, "{") == 0) {
                                currentToken = currentToken->nextPointer;
                                currentState = ELSE_CONDITION;
                            }
                        }
                    }
                    /*else if (strcmp(token, "void")==0 && strcmp(currentToken->nextPointer->text, "main")){
                        temp = currentToken->nextPointer->nextPointer;
                        if (strcmp(temp->text, "(") == 0) {
                            temp = temp->nextPointer;
                            if (strcmp(temp->text, ")") == 0) {
                                temp = temp->nextPointer;
                                if (strcmp(temp->text, "{") == 0) {
                                    scopeState = INSIDE_MAIN;
                                    currentToken = temp;
                                }
                            }
                        }
                    }*/
                } else if (isValidIdentifier(token)) {
                    if (scopeState == INSIDE_MAIN) {
                        symbol = getSymbolFromTable(*headSymbol, token);
                        if (symbol != NULL) {
                            currentState = ID;
                        }
                    }
                } else if (strcmp(token, "}") == 0) {
                    if (scopeState == INSIDE_MAIN) {
                        if (mainHasReturned == 1) {
                            scopeState = START;
                        }
                    } else if (conditionStatement != 0) {
                        removeLastFromConditionStatement(&conditionStatement);
                        if (getLastConditionStatement(conditionStatement)) {
                            isAfterIfEnd = 1;
                            break;
                        }
                    }
                } else if (strcmp(token, "return") == 0) {
                    currentState = RETURN;
                }
                isAfterIfEnd = 0;
                break;
            }

            case KEYWORD: {
                if (isValidIdentifier(token)) {
                    symbol->name = token;
                    currentState = ID;
                }
                break;
            }

            case ID: {
                if (strcmp(token, "=") == 0) {
                    currentState = ASSIGN;
                } else if (strcmp(token, ";")) {
                    currentState = NIY;
                }
                break;
            }

            case ASSIGN: {
                // Todo: Handle expressions and set currentToken to after semicolon
                currentState = NIY;
                break;
            }

            case IF_CONDITION: {
                // Todo: Handle expressions and set currentToken to after {
                initializeConditionStatement(&conditionStatement);
                addIfToConditionStatement(&conditionStatement);
                currentState = NIY;
                break;
            }

            case ELSE_CONDITION: {
                initializeConditionStatement(&conditionStatement);
                addElseToConditionStatement(&conditionStatement);
                currentState = NIY;
                break;
            }

            case RETURN: {
                // Todo: handle Return and set currentToken to after semicolon
                mainHasReturned = 1;
                break;
            }

        }
        currentToken = currentToken->nextPointer;
    }

}

void messageError(TokenPointer tokenPointer, char *message) {
    printf("Error at line %d, %s\n", tokenPointer->lineNumber, message);
}

void unexpectedTokenException(TokenPointer tokenPointer, char *description) {
    printf("Error at line %d. %s but found %s\n", tokenPointer->lineNumber, description, tokenPointer->text);
}

void addElseToConditionStatement(int *conditionStatement) {
    *conditionStatement = *conditionStatement << 1;
}

void addIfToConditionStatement(int *conditionStatement) {
    *conditionStatement = *conditionStatement << 1;
    *conditionStatement = *conditionStatement | 1;
}

void removeLastFromConditionStatement(int *conditionStatement) {
    *conditionStatement = *conditionStatement >> 1;
    if (*conditionStatement == 1) *conditionStatement = 0;
}

void initializeConditionStatement(int *conditionStatement) {
    if (*conditionStatement == 0) *conditionStatement = 1;
}

int getLastConditionStatement(int conditionStatement) {
    return conditionStatement & 1;
}

void insertSymbolToTable(SymbolPointer *headSymbol, SymbolPointer symbol) {
    int index = 1;
    SymbolPointer temp;
    if (headSymbol == NULL) {
        headSymbol = &symbol;
        symbol->index = 0;
    } else {
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

TokenPointer createTokenPointer() {
    return (TokenPointer) malloc(sizeof(Token));
}

int isValidKeyword(char *token) {
    return
            strcmp(token, "int") == 0 ||
            strcmp(token, "float") == 0 ||
            strcmp(token, "bool") == 0 ||
            strcmp(token, "char") == 0 ||
            strcmp(token, "if") == 0 ||
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
    if (strcmp(token, "int")) return INT;
    else if (strcmp(token, "float")) return FLOAT;
    else if (strcmp(token, "char")) return CHAR;
    else if (strcmp(token, "bool")) return BOOL;
    else if (strcmp(token, "if")) return IF;
    else if (strcmp(token, "else")) return ELSE;
    else if (strcmp(token, "main")) return MAIN;
    else if (strcmp(token, "void")) return VOID;
    else if (strcmp(token, "null")) return NUL;
}

VariableKeyword getVariableKeyword(char *token) {
    if (strcmp(token, "int")) return (VariableKeyword) INT;
    else if (strcmp(token, "float")) return (VariableKeyword) FLOAT;
    else if (strcmp(token, "char")) return (VariableKeyword) CHAR;
    else if (strcmp(token, "bool")) return (VariableKeyword) BOOL;
}

int isValidIdentifier(char *token) {
    if (isdigit(*token)) return 0;
    else {
        while (*token != '\0') {
            if (!isalnum(*token) && !*token=='_') return 0;
        }
        return 1;
    }
}