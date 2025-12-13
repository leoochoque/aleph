 %{
    #include <stdio.h>
    #include "aleph.h"
    int yylex();
    void yyerror(char *); 
%}

%union{
    struct ast * a;
    char * s;
    long i;
    int r;
    double f;
    struct expl* le;
    struct syml* ls;
}

%start aleph

%token '+' '-' '*' '/' '=' '{' '}' '[' ']' ',' ';' '%' '\\' ':'
%token IF ELSE THEN  ENDIF WHILE DO ENDWHILE FOR ENDFOR PRINT PRINTLN EOL POWER SQRT DEF AS END RETURN TOLIST TOSET STRUCT
%token <s> STR IDD
%token <i> NUMBER
%token <r> BOOLEAN CMP
%token <f> NUMBERFLOAT

%left ':'
%left OR
%left AND 
%right NOT
%nonassoc IN CONTAINS
%nonassoc CMP
%left SIZE TOLIST TOSET
%nonassoc ADD TO POP PRINT RANGE SETKW
%left UNION DIFF INTSC 
%nonassoc '|'
%left '+' '-'
%left '*' '/' '%' DIV
%right POWER SQRT
%left '(' ')' '[' ']'
%nonassoc UMINUS GET '.' IF ELSE //have to look up

%type <a> exp litSet litList sentence asign if block while for aleph function setComprehension listComprehension  comp_tail_opt structure structBlock structStmt
%type <le> listExp
%type <ls> listIdd

%%

aleph: %empty { if(mode) printf("aleph> "); $$ = NULL; }
    | aleph sentence ';' { eval($2); }
    | aleph sentence EOL { if(mode) { if($2) eval($2); printf("aleph> "); } }
    | aleph function { eval($2); } ';'
    | aleph structure ';'
    ;

function: DEF IDD '(' listIdd ')' AS block END { $$ = newfunc($2,$4,$7); }
    | DEF IDD '('')' AS block END { $$ = newfunc($2,NULL,$6); }
    ;

structure: STRUCT IDD '(' listIdd ')' AS structBlock END { newstruct($2,$4,$7); $$ = NULL; }
        | STRUCT IDD '('')' AS structBlock END { newstruct($2,NULL,$6); $$ = NULL; }
        ;

structBlock: structBlock structStmt ';' { $$ = newast(BLOCK, $1, $2); }
    | structStmt ';' { $$ = newast(BLOCK, $1, NULL); }
    ;

structStmt: function {$$ = $1;} 
        | asign {$$ = $1;} ;

sentence: %empty {$$ = NULL;}
        | RETURN exp { $$ = newast(RETURNKEYW, $2, NULL); }
        | PRINT exp { $$ = newast(PRINTSTMT, $2, NULL); }
        | PRINTLN exp { $$ = newast(PRINTSTMT, $2, $2); }
        | SETKW exp '[' exp ']' AS exp { $$ = newset($2, $4, $7); }
        | exp { if(mode) $$ = newast(PRINTSTMT, $1, NULL); }
        | asign
        | if
        | while
        | for
        ;

block: block sentence ';' { $$ = newast(BLOCK, $1, $2); }
    | sentence ';' { $$ = newast(BLOCK, $1, NULL); }
    ;

for : FOR IDD IN exp DO block ENDFOR { $$ = newflow(FORSTMT, newref($2), $4, $6); }
    ;

while: WHILE exp DO block ENDWHILE { $$ = newflow(WHSTMT, $2, $4, NULL);}
    ;

if: IF exp THEN block ENDIF { $$ = newflow(IFSTMT, $2, $4, NULL);}
    | IF exp THEN block ELSE block ENDIF { $$ = newflow(IFSTMT, $2, $4, $6); }
    ;

asign: listIdd '=' listExp { $$ = newasgn($1,$3); }
    | exp '[' exp ']' '=' exp { $$ = newset($1,$3,$6); }
    | exp '.' IDD '=' exp { $$ = newdotop($1, newref($3), $5); }
    ;

exp: exp UNION exp { $$ = newast(USET,$1,$3); }
    | exp INTSC exp { $$ = newast(ISET,$1,$3); }
    | exp DIFF exp { $$ = newast(DSET,$1,$3); }
    | ADD exp TO exp { $$ = newast(ADDTO,$2,$4); }
    | exp '[' exp ']' { $$ = newast(GETTER,$1,$3); }
    | RANGE '(' listExp ')' { $$ = newast(RANGEOP,(struct ast*)$3,NULL); }
    | POP exp { $$ = newast(PLIST,$2,NULL); }
    | SIZE exp { $$ = newast(SIZEOP,$2,NULL); }
    | GET exp { $$ = $2;} //sugar syntax compatibilidad con versiones anteriores
    | exp IN exp { $$ = newast(INOP,$1,$3); }
    | exp CONTAINS exp { $$ = newast(CONTOP,$1,$3); }
    | exp CMP exp { $$ = newast($2,$1,$3); }
    | exp AND exp { $$ = newast(ANDLOG,$1,$3); }
    | exp OR exp { $$ = newast(ORLOG,$1,$3); }
    | NOT exp { $$ = newast(NOTLOG,$2,NULL); }
    | exp '+' exp { $$ = newast('+',$1,$3); }
    | exp '-' exp { $$ = newast('-',$1,$3); }
    | exp '*' exp { $$ = newast('*',$1,$3); }
    | exp DIV exp { $$ = newast('/',$1,$3); }
    | exp '%' exp { $$ = newast('%',$1,$3); }
    | exp '/' exp { $$ = newast('D',$1,$3); }
    | exp POWER exp { $$ = newast('^',$1,$3); }
    | SQRT exp { $$ = newast('S',$2,NULL); }
    | '|' exp '|' { $$ = newast('|',$2,NULL);}
    | '-' exp %prec UMINUS { $$ = newast('M',$2,NULL); }
    | '(' exp ')' { $$ = $2; }
    | exp '.' IDD { $$ = newdotop($1, newref($3), NULL); }
    | exp IF exp ELSE exp { $$ = newflow(IFTERN, $3, $1, $5); }
    | litSet
    | litList
    | IDD { $$ = newref($1); }
    | STR { $$ = newElem($1); }
    | NUMBER { $$ = newNumber($1); }
    | NUMBERFLOAT { $$ = newNumberFloat($1); }
    | BOOLEAN { $$ = newBoolean($1); }
    | exp '(' listExp ')' { $$ = newcall($1,$3); }
    | exp '('')' { $$ = newcall($1,NULL); }
    | TOLIST exp { $$ = newast(TO_LIST,$2,NULL); }
    | TOSET exp { $$ = newast(TO_SET,$2,NULL); }
    | listComprehension
    | setComprehension
    | '\\' listIdd ':' exp { $$ = newlambda($2,$4); }
    ;

litSet: '{' listExp '}' { $$ = newast(TSET,(struct ast*)$2,NULL); }
    | '{' '}' { $$ = newast(TSET,NULL,NULL); }
    ;

litList: '[' listExp ']' { $$ = newast(TLIST,(struct ast*)$2,NULL); }
    | '[' ']' { $$ = newast(TLIST,NULL,NULL); }
    ;

setComprehension: '{' exp FOR IDD IN exp comp_tail_opt '}' { $$ = newcomprenshion(SETCOMP,$2,newref($4),$6,$7); }
    ;

listComprehension: '[' exp FOR IDD IN exp comp_tail_opt ']' { $$ = newcomprenshion(LISTCOMP,$2,newref($4),$6,$7); }
    ;

comp_tail_opt : %empty { $$ = NULL; }
    | IF exp {$$ = newast(IFCOMPRENSHION, $2, NULL);}
    ;   

listExp: exp ',' listExp { $$ = newexpl($1,$3); }
    | exp { $$ = newexpl($1, NULL); }
    ;

listIdd: IDD ',' listIdd { $$ = newsyml($1,$3); }
    | IDD { $$ = newsyml($1, NULL); }
    ;

%%

void yyerror(char *s){
    printf("%s",s);
}