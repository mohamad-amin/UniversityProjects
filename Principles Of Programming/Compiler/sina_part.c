//
//  main.c
//  final project test
//
//  Created by Sina on 1/14/16.
//  Copyright © 2016 Sina. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Token {
    char * text;
    int lineNumber;
    struct Token *nextPointer;
};

typedef struct Token Token;
void printList(Token * head);
void load(Token* head, char a[]);
void save( Token* head);
void loadSecond(Token * head);
void preprocessor(void);
void loadToken(Token * head);
void tokenizer(void);

int main(int argc, const char * argv[]) {
    preprocessor();
    tokenizer();
    return 0;
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

void save( Token* head){
    Token * current;
    current=head->nextPointer;
    FILE * p;
    p = fopen("/Users/sina/Desktop/tmp.c","w+");
    if ( p==NULL){
        printf("ERROR!\n");
        exit(1);
    }
    while( current!=NULL){
        fprintf(p,"%s",current->text);
        current=current->nextPointer;
    }
    fclose(p);
    return;
    
    
}

void load(Token* head, char a[]) {
    Token *new, *current;
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
        new = (Token*) malloc(sizeof(Token));
        current->nextPointer = new;
        new->text = str;
        new->nextPointer = NULL;
        current = current->nextPointer;
    }
    new = (Token *) malloc(sizeof (Token));
    new->nextPointer = NULL;
    current->nextPointer = new;
    new->text = "\n";
    fclose(s);
    return;
}

void loadSecond(Token * head) {
    Token * current = head, *new;
    char * str;
    char a[100], b[100], c, abbas[100];
    int i = 0, j = 0;
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
                    load(head, b);
                    while (!feof(p)) {
                        current = head;
                        while (current->nextPointer != NULL)
                            current = current->nextPointer;
                        str = (char*) malloc(sizeof (char)*50);
                        new = (Token *) malloc(sizeof (Token));
                        new->nextPointer = NULL;
                        current->nextPointer = new;
                        fgets(str, 100, p);
                        new->text = str;
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

void tokenizer(void) {
    Token * head;
    head = (Token *) malloc(sizeof (Token));
    loadToken(head);
    printList(head);
    return;
}

void loadToken(Token * head) {
    Token * current, * new;
    char * str, * delim, *token, *newLine;
    int counter = 1;
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
            new = (Token *) malloc(sizeof (Token));
            current->nextPointer = new;
            new->nextPointer = NULL;
            new->text = token;
            new->lineNumber = counter;
            current = current->nextPointer;
            token = strtok(NULL, delim);
            
        }
        counter++;
    }
    
    return;
}

void printList(Token * head) {
    Token * current;
    current = head->nextPointer;
    while (current != NULL) {
        printf("%s\t%d\n", current->text, current->lineNumber);
        current = current->nextPointer;
        
    }
    return;
}