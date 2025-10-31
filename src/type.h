#ifndef TYPE_H
#define TYPE_H

#include "9cc.h"

Type *basetype(void);
Type *declarater(Type *base);
Type *int_type(void);
Type *char_type(void);
Type *ptr_to(Type *base);
int size_of(Type *type);

#endif