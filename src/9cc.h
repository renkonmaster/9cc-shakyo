#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"

void error(char *fmt, ...);

int count();

extern Token *token;
extern Node *functions[100];
extern int functions_count;
extern GVar *globals;
extern StringLiteral *string_literals;