//
//  main.c
//  merging sina and mohammad amin
//
//  Created by Sina on 1/16/16.
//  Copyright © 2016 Sina. All rights reserved.
//

#include <stdio.h>
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
    INTk, FLOATk, CHARk, BOOLk, IFk, ELSEk, MAINk, VOIDk, NULk
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
void printList(TokenPointer headToken);
int load(TokenPointer headToken, char a[]);
void save(TokenPointer headToken);
int loadSecond(TokenPointer headToken);
int preprocessor(void);
void loadToken(TokenPointer headToken, int lineNumber);
TokenPointer tokenizer(int lineNumber);

int main(int argc, const char * argv[]) {
    TokenPointer currentToken;
    SymbolPointer headSymbol;
    int lineNumber;
    headSymbol = createSymbolPointer();
    lineNumber = preprocessor();
    currentToken = tokenizer(lineNumber)->nextPointer;
    syntaxAnalyze(currentToken, &headSymbol);
    return 0;
}

void syntaxAnalyze(TokenPointer currentToken, SymbolPointer *headSymbol) {
    char *token;
    SymbolPointer * currentSymbol;
    currentSymbol = headSymbol;
    State currentState = NIY;
    int conditionStatement = 0;
    ScopeState scopeState = START;
    TokenPointer temp;
    SymbolPointer symbol;

    int mainHasReturned = 0, isAfterIfEnd = 0;
    while (currentToken != NULL) {
        printf("%s\n", currentToken->text);
        token = currentToken->text;
        switch (currentState) {

            case NIY:
            {
                printf("KHAR\n");
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
                                        } else unexpectedTokenException(currentToken, "Expected {");
                                    } else unexpectedTokenException(currentToken, "Expected )");
                                } else unexpectedTokenException(currentToken, "Expected (");
                            }
                            messageError(currentToken, "int main() is defined already.");
                        } else {
                            // check shavad
                            symbol = createSymbolPointer();
                            //                            currentSymbol->nextPointer=symbol;
                            //                          currentSymbol=currentSymbol->nextPointer;
                            symbol->type = getVariableKeyword(token);

                            currentState = KEYWORD;
                            break;
                            // Has Read Keyword, is going for Id.
                        }
                    } else if (strcmp(token, "if")) {
                        if (scopeState == INSIDE_MAIN) {
                            if (strcmp(currentToken->nextPointer->text, "(") == 0) {
                                currentToken = currentToken->nextPointer;
                                currentState = IF_CONDITION;
                            } 
                            else unexpectedTokenException(currentToken, "Expected (");
                        } else messageError(currentToken, "cannot use if out side of main() func.");
                    } else if (strcmp(token, "else") == 0) {
                        if (isAfterIfEnd) {
                            if (strcmp(currentToken->nextPointer->text, "{") == 0) {
                                currentToken = currentToken->nextPointer;
                                currentState = ELSE_CONDITION;
                            } else unexpectedTokenException(currentToken, "{");
                        } else if (scopeState == INSIDE_MAIN) messageError(currentToken, "else not allowed without if");
                        else messageError(currentToken, "cannat use else out side of main() func. ");
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
                        } else messageError(currentToken, " it hasen't defined yet");
                    } else messageError(currentToken, "not allowed out  11side of main() func.");
                } else if (strcmp(token, "}") == 0) {
                    if (conditionStatement != 0) {
                        removeLastFromConditionStatement(&conditionStatement);
                        if (getLastConditionStatement(conditionStatement)) {
                            isAfterIfEnd = 1;
                            break;
                        }
                    } else if (scopeState == INSIDE_MAIN) {
                        if (mainHasReturned == 1) {
                            scopeState = START;
                        } else messageError(currentToken, "you have to return from main()");
                    }
                } else if (strcmp(token, "return") == 0) {
                    currentState = RETURN;
                }
                isAfterIfEnd = 0;
                break;
            }

            case KEYWORD:
            {
                if (isValidIdentifier(token)) {
                    symbol->name = token;
                    currentState = ID;
                } else messageError(currentToken, "we need an identifier here");
                break;
            }

            case ID:
            {
                if (strcmp(token, "=") == 0) {
                    currentState = ASSIGN;
                } else if (strcmp(token, ";")) {
                    currentState = NIY;
                }
                break;
            }

            case ASSIGN:
            {
                // Todo: Handle expressions and set currentToken to after semicolon
                currentState = NIY;
                break;
            }

            case IF_CONDITION:
            {
                // Todo: Handle expressions and set currentToken to after {
                initializeConditionStatement(&conditionStatement);
                addIfToConditionStatement(&conditionStatement);
                currentState = NIY;
                break;
            }

            case ELSE_CONDITION:
            {
                initializeConditionStatement(&conditionStatement);
                addElseToConditionStatement(&conditionStatement);
                currentState = NIY;
                break;
            }

            case RETURN:
            {
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
    exit(1);
}

void unexpectedTokenException(TokenPointer tokenPointer, char *description) {
    printf("Error at line %d. %s but found %s\n", tokenPointer->lineNumber, description, tokenPointer->text);
    exit(1);
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
    return (SymbolPointer) malloc(sizeof (Symbol));
}

TokenPointer createTokenPointer() {
    return (TokenPointer) malloc(sizeof (Token));
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
    if (strcmp(token, "int")) return INTk;
    else if (strcmp(token, "float")) return FLOATk;
    else if (strcmp(token, "char")) return CHARk;
    else if (strcmp(token, "bool")) return BOOLk;
    else if (strcmp(token, "if")) return IFk;
    else if (strcmp(token, "else")) return ELSEk;
    else if (strcmp(token, "main")) return MAINk;
    else if (strcmp(token, "void")) return VOIDk;
    else if (strcmp(token, "null")) return NULk;
}

VariableKeyword getVariableKeyword(char *token) {
    if (strcmp(token, "int")) return (VariableKeyword) INT;
    else if (strcmp(token, "float")) return (VariableKeyword) FLOAT;
    else if (strcmp(token, "char")) return (VariableKeyword) CHAR;
    else if (strcmp(token, "bool")) return (VariableKeyword) BOOL;
}

int isValidIdentifier(char * token) {
    if (isdigit(*token)) return 0;
    else {
        //AZ WHILE DAR NEMOOMAD 
        while (*token != '\0') {
            if (!(isalnum(*token)) && !(*token == '_')) return 0;
            token += (sizeof (char));
        }
        return 1;
    }
}

int preprocessor(void) {
    TokenPointer headToken;
    int lineNumber;
    headToken = (TokenPointer) malloc(sizeof (Token));
    strcpy(headToken->text, "");
    headToken->nextPointer = NULL;
    lineNumber = loadSecond(headToken);
    save(headToken);
    return lineNumber;
}

void save(TokenPointer headToken) {
    TokenPointer current;
    current = headToken->nextPointer;
    FILE * p;
    p = fopen("/Users/sina/Desktop/tmp.c", "w+");
    if (p == NULL) {
        printf("ERROR!\n");
        exit(1);
    }
    while (current != NULL) {
        fprintf(p, "%s", current->text);
        current = current->nextPointer;
    }
    fclose(p);
    return;


}

int load(TokenPointer headToken, char a[]) {
    TokenPointer new, current;
    current = headToken;
    char *str;
    FILE *s;
    int counter = 0;
    s = fopen(a, "r");
    if (s == NULL) {
        printf(" ERROR!\n");
        exit(1);
    }
    while (!feof(s)) {
        str = (char *) malloc(sizeof (char)*100);
        fgets(str, 100, s);
        new = (TokenPointer) malloc(sizeof (Token));
        current->nextPointer = new;
        strcpy(new->text, str);
        new->nextPointer = NULL;
        current = current->nextPointer;
        counter++;
    }
    new = (TokenPointer) malloc(sizeof (Token));
    new->nextPointer = NULL;
    current->nextPointer = new;
    strcpy(new->text, "\n");
    fclose(s);
    return counter;
}

int loadSecond(TokenPointer headToken) {
    TokenPointer current = headToken, new;
    char * str;
    char a[100], b[100], c, abbas[100];
    int i = 0, j = 0, lineNumber;
    FILE * p;
    p = fopen("/Users/sina/Desktop/salam/salam/main.c", "r");
    if (p == NULL) {
        printf("ERROR!\n");
        exit(1);
    }
    c = fgetc(p);
    if (c == '#') {
        fscanf(p, "%s", abbas);
        if (strcmp(abbas, "include") == 0) {
            fscanf(p, "%s", abbas);
            if (abbas[i] == '<') {
                while (abbas[i] != '>' && abbas[i] != '\0') {
                    i++;
                }
                if (abbas[i] != '>') {
                    printf("ERROR 1 :D\n");
                    exit(1);
                } else {
                    fseek(p, 0, SEEK_SET);
                    fgets(a, 100, p);
                    i = 0;
                    while (a[i] != '<')
                        i++;
                    i++;
                    while (a[i] != '>') {
                        b[j] = a[i];
                        i++;
                        j++;
                    }
                    lineNumber = load(headToken, b);
                    while (!feof(p)) {
                        current = headToken;
                        while (current->nextPointer != NULL)
                            current = current->nextPointer;
                        str = (char*) malloc(sizeof (char)*50);
                        new = (TokenPointer) malloc(sizeof (Token));
                        new->nextPointer = NULL;
                        current->nextPointer = new;
                        fgets(str, 100, p);
                        strcpy(new->text, str);
                        current = current->nextPointer;
                    }

                }
            }
        } else {
            printf("ERROR! :D\n");
            return 0;
        }
    }

    fclose(p);
    return lineNumber;
}

TokenPointer tokenizer(int lineNumber) {
    TokenPointer headToken;
    headToken = (TokenPointer) malloc(sizeof (Token));
    loadToken(headToken, lineNumber);
    printList(headToken);
    return headToken;


}

void loadToken(TokenPointer head, int lineNumber) {
    TokenPointer current, new;
    char * str, * delim, *token, *newLine;
    int counter = 2, lineNum;
    current = head;
    delim = " ";
    FILE * p;
    p = fopen("/Users/sina/Desktop/tmp.c", "r");
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
            new = (TokenPointer) malloc(sizeof (Token));
            current->nextPointer = new;
            new->nextPointer = NULL;
            strcpy(new->text, token);
            lineNum = counter - lineNumber;
            if (lineNum <= 0) lineNum = 1;
            new->lineNumber = lineNum;
            current = current->nextPointer;
            token = strtok(NULL, delim);

        }
        counter++;
    }

    return;
}

void printList(TokenPointer headToken) {
    TokenPointer current;
    current = headToken->nextPointer;
    while (current != NULL) {
        printf("%s\t%d\n", current->text, current->lineNumber);
        current = current->nextPointer;

    }
    return;
}
