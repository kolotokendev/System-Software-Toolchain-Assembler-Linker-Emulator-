%{
  #include <iostream>
#include <memory>
  #include "../inc/assembler/Assembler.hpp"
  using namespace std;

  extern int yylex();
  extern int lineNum;
  extern Assembler* assembler_instance;

  void yyerror(const char*);
%}

%output  "parser.cpp"
%defines "parser.hpp"

%union {
    u_int32_t ival;
    u_int8_t rval;
    char *sval;
}


/* terminal symbols */
%token <ival> LITERAL
%token <sval> SYMBOL
%token <sval> STRING
%token <rval> GPRX CSRX

%token GLOBAL EXTERN SECTION WORD SKIP ASCII EQU END
%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG
%token ADD SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR
%token ENDL

/* special symbols */
%token NOP
%token POP_CS POP_CS_DSP POP_DSP
%token LD_REG LD_PCREL LD_IMM LD_DSP LD_DSP_CSR ST_DSP
%token ST_IND
%token CALL_IND
%token JMP_IND BEQ_IND BNE_IND BGT_IND

%left '+' '-'
%nonassoc UMINUS

%start program

%%

program:
    | statements { assembler_instance->endDirective(); }

statements:
    statement
    | statements statement
    ;

statement:
      label instruction ENDL
    | label directive ENDL
    | label ENDL
    | instruction ENDL
    | directive ENDL
    | ENDL
    ;

label:
      SYMBOL ':' { assembler_instance->insertSymbolLOC($1); free($1); }
    ;


directive:
      GLOBAL global_symbol_list
    | EXTERN extern_symbol_list
    | SECTION SYMBOL                 { assembler_instance->insertSection($2); free($2); }
    | WORD initializer_list
    | SKIP LITERAL                   { assembler_instance->skipDirective($2); }
    | ASCII STRING                   { assembler_instance->asciiDirective($2); free($2); }
    | EQU SYMBOL ',' ival_expression { assembler_instance->closeTNSEntry($2); free($2); }
    | END                            { assembler_instance->endDirective(); YYACCEPT; }
    ;

instruction:
      HALT                                  { assembler_instance->insertInstruction(HALT); }
    | INT                                   { assembler_instance->insertInstruction(INT); }
    | IRET                                  { assembler_instance->insertIretIns(); }
    | CALL LITERAL                          { assembler_instance->insertCallIns($2); }
    | CALL SYMBOL                           { assembler_instance->insertCallIns($2); free($2); }
    | RET                                   { assembler_instance->insertInstruction(POP, {PC}); }
    | JMP LITERAL                           { assembler_instance->insertJmpIns(JMP, $2); }
    | JMP SYMBOL                            { assembler_instance->insertJmpIns(JMP, $2); free($2); }
    | BEQ GPRX ',' GPRX ',' LITERAL         { assembler_instance->insertJmpIns(BEQ, $6, {R0, $2, $4}); }
    | BEQ GPRX ',' GPRX ',' SYMBOL          { assembler_instance->insertJmpIns(BEQ, $6, {R0, $2, $4}); free($6); }
    | BNE GPRX ',' GPRX ',' LITERAL         { assembler_instance->insertJmpIns(BNE, $6, {R0, $2, $4}); }
    | BNE GPRX ',' GPRX ',' SYMBOL          { assembler_instance->insertJmpIns(BNE, $6, {R0, $2, $4}); free($6); }
    | BGT GPRX ',' GPRX ',' LITERAL         { assembler_instance->insertJmpIns(BGT, $6, {R0, $2, $4}); }
    | BGT GPRX ',' GPRX ',' SYMBOL          { assembler_instance->insertJmpIns(BGT, $6, {R0, $2, $4}); free($6); }
    | PUSH GPRX                             { assembler_instance->insertInstruction(PUSH, {$2}); }
    | POP GPRX                              { assembler_instance->insertInstruction(POP, {$2}); }
    | XCHG GPRX ',' GPRX                    { assembler_instance->insertInstruction(XCHG, {$4, $2}); }
    | ADD GPRX ',' GPRX                     { assembler_instance->insertInstruction(ADD, {$4, $4, $2}); }
    | SUB GPRX ',' GPRX                     { assembler_instance->insertInstruction(SUB, {$4, $4, $2}); }
    | MUL GPRX ',' GPRX                     { assembler_instance->insertInstruction(MUL, {$4, $4, $2}); }
    | DIV GPRX ',' GPRX                     { assembler_instance->insertInstruction(DIV, {$4, $4, $2}); }
    | NOT GPRX                              { assembler_instance->insertInstruction(NOT, {$2, $2}); }
    | AND GPRX ',' GPRX                     { assembler_instance->insertInstruction(AND, {$4, $4, $2}); }
    | OR GPRX ',' GPRX                      { assembler_instance->insertInstruction(OR, {$4, $4, $2}); }
    | XOR GPRX ',' GPRX                     { assembler_instance->insertInstruction(XOR, {$4, $4, $2}); }
    | SHL GPRX ',' GPRX                     { assembler_instance->insertInstruction(SHL, {$4, $4, $2}); }
    | SHR GPRX ',' GPRX                     { assembler_instance->insertInstruction(SHR, {$4, $4, $2}); }
    | LD '$' LITERAL ',' GPRX               { assembler_instance->insertLoadIns(LD_REG, $3, {$5, R0}); }
    | LD '$' SYMBOL ',' GPRX                { assembler_instance->insertLoadIns(LD_REG, $3, {$5, R0}); free($3); }
    | LD LITERAL ',' GPRX                   { assembler_instance->insertLoadIns(LD, $2, {$4, R0, R0}); }
    | LD SYMBOL ',' GPRX                    { assembler_instance->insertLoadIns(LD, $2, {$4, R0, R0}); free($2); }
    | LD GPRX ',' GPRX                      { assembler_instance->insertInstruction(LD_REG, {$4, $2}); }
    | LD '[' GPRX ']' ',' GPRX              { assembler_instance->insertInstruction(LD, {$6, $3}); }
    | LD '[' GPRX '+' LITERAL ']' ',' GPRX  { assembler_instance->insertLoadIns(LD_DSP, $5, {$8, $3, R0}); }
    | LD '[' GPRX '+' SYMBOL ']' ',' GPRX   { assembler_instance->insertLoadIns(LD_DSP, $5, {$8, $3, R0}); free($5); }
    | ST GPRX ',' LITERAL                   { assembler_instance->insertStoreIns(ST, $4, {R0, R0, $2}); }
    | ST GPRX ',' SYMBOL                    { assembler_instance->insertStoreIns(ST, $4, {R0, R0, $2}); free($4); }
    | ST GPRX ',' '[' GPRX ']'              { assembler_instance->insertInstruction(ST, {$5, R0, $2}); }
    | ST GPRX ',' '[' GPRX '+' LITERAL ']'  { assembler_instance->insertStoreIns(ST_DSP, $7, {R0, $5, $2}); }
    | ST GPRX ',' '[' GPRX '+' SYMBOL ']'   { assembler_instance->insertStoreIns(ST_DSP, $7, {R0, $5, $2}); free($7); }
    | CSRRD CSRX ',' GPRX                   { assembler_instance->insertInstruction(CSRRD, {$4, $2}); }
    | CSRWR GPRX ',' CSRX                   { assembler_instance->insertInstruction(CSRWR, {$4, $2}); }
    ;

initializer_list:
      SYMBOL                        { assembler_instance->insertConstant($1); free($1); }
    | LITERAL                       { assembler_instance->insertConstant($1); }
    | initializer_list ',' SYMBOL   { assembler_instance->insertConstant($3); free($3); }
    | initializer_list ',' LITERAL  { assembler_instance->insertConstant($3); }
    ;

global_symbol_list:
      SYMBOL                        { assembler_instance->insertSymbolGLOB($1); free($1); }
    | global_symbol_list ',' SYMBOL { assembler_instance->insertSymbolGLOB($3); free($3); }
    ;

extern_symbol_list:
      SYMBOL                        { assembler_instance->insertSymbol($1); free($1); }
    | extern_symbol_list ',' SYMBOL { assembler_instance->insertSymbol($3); free($3); }
    ;

ival_expression:
      LITERAL { assembler_instance->insertTNS($1); }
    | SYMBOL  { assembler_instance->insertTNS($1); free($1); }
    | ival_expression '+' ival_expression { assembler_instance->insertTNS('+'); }
    | ival_expression '-' ival_expression { assembler_instance->insertTNS('-'); }
    | '-' ival_expression %prec UMINUS    { assembler_instance->insertTNS('-'); }
    | '(' ival_expression ')'
    ;

%%

void yyerror(const char *s) {
  cout << "Parse error on line " << lineNum << "!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}