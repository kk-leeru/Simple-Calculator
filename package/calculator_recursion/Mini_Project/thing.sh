#!/bin/bash
cat lex.h parser.h codeGen.h lex.c parser.c codeGen.c main.c  | grep -vE "\.h"\"  > cat.c