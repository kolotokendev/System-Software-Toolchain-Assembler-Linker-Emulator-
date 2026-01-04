/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LITERAL = 258,
    SYMBOL = 259,
    STRING = 260,
    GPRX = 261,
    CSRX = 262,
    GLOBAL = 263,
    EXTERN = 264,
    SECTION = 265,
    WORD = 266,
    SKIP = 267,
    ASCII = 268,
    EQU = 269,
    END = 270,
    HALT = 271,
    INT = 272,
    IRET = 273,
    CALL = 274,
    RET = 275,
    JMP = 276,
    BEQ = 277,
    BNE = 278,
    BGT = 279,
    PUSH = 280,
    POP = 281,
    XCHG = 282,
    ADD = 283,
    SUB = 284,
    MUL = 285,
    DIV = 286,
    NOT = 287,
    AND = 288,
    OR = 289,
    XOR = 290,
    SHL = 291,
    SHR = 292,
    LD = 293,
    ST = 294,
    CSRRD = 295,
    CSRWR = 296,
    ENDL = 297,
    NOP = 298,
    POP_CS = 299,
    POP_CS_DSP = 300,
    POP_DSP = 301,
    LD_REG = 302,
    LD_PCREL = 303,
    LD_IMM = 304,
    LD_DSP = 305,
    LD_DSP_CSR = 306,
    ST_DSP = 307,
    ST_IND = 308,
    CALL_IND = 309,
    JMP_IND = 310,
    BEQ_IND = 311,
    BNE_IND = 312,
    BGT_IND = 313,
    UMINUS = 314
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 17 "parser.y"

    u_int32_t ival;
    u_int8_t rval;
    char *sval;

#line 123 "parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
