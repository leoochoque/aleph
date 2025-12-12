%{
    #include "parser.tab.h"
    #include "aleph.h"
    YY_BUFFER_STATE bp;
    int mode = 0;
%}

%option noyywrap

ID [a-zA-Z_][a-zA-Z0-9_]*
STR \"[^\"]*\"
NUMBER [0-9]+
NUMBERFLOAT ([0-9]+"."[0-9]*)
BLOCKCOMMENT "{-"([^\}]|\}[^-])*"-\}"

%%

"range" {return RANGE;}
"if" {return IF;}
"then" {return THEN;}
"else" {return ELSE;}
"endif" {return ENDIF;}
"while" {return WHILE;}
"do" {return DO;}
"endwhile" {return ENDWHILE;}
"for" {return FOR;}
"endfor" {return ENDFOR; }
"def" {return DEF;}
"end" {return END;}
"as" {return AS;}
"and" {return AND;}
"or" {return OR;}
"not" {return NOT;}
"print" {return PRINT;}
"println" {return PRINTLN;}
"exit" { return 0; }
";" |
"+" |
"-" |
"*" |
"/" |
"%" |
"|" |
"=" |
"{" |
"}" |
"(" |
")" |
"," |
"[" |
"]" |
":" |
"\\" {return yytext[0];}
">" {yylval.r = 1; return CMP;}
"<" {yylval.r = 2; return CMP;}
"<>" {yylval.r = 3; return CMP;}
"==" {yylval.r = 4; return CMP;}
">=" {yylval.r = 5; return CMP;}
"<=" {yylval.r = 6; return CMP;}
"//" {return DIV;}
"**" {return POWER;}
"sqrt" {return SQRT;}
"union" {return UNION;}
"intsc" {return INTSC;}
"diff" {return DIFF;}
"add" {return ADD;}
"to" {return TO;}
"set" {return SETKW;}
"get" {return GET;}
"pop" {return POP;}
"in" {return IN;}
"contains" {return CONTAINS;}
"size" {return SIZE;}
"true"|"false" {yylval.r = compara_cad(yytext, "false");return BOOLEAN; }
"return" {return RETURN;}
"tolist" {return TOLIST;}
"toset" {return TOSET;}
{ID} { yylval.s = devuelve_cad(yytext); return IDD; }
{NUMBER} {yylval.i = atoi(yytext);return NUMBER; }
{NUMBERFLOAT} {yylval.f = atof(yytext); return NUMBERFLOAT; }
{STR} {yytext[tam_cad(yytext)-1] = '\0'; yylval.s = ++yytext; return STR; }
[ \t\r] {}
\\\n {printf("aleph> ");}
\n { if(mode) return EOL; }
{BLOCKCOMMENT}
"#".*
. {printf("Lexical error"); exit(1);}

%%

int main(int argc, char** argv){
   init_env();

   if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Could not open file %s\n", argv[1]);
            return 1;
        }
        yyin = file;
        yyparse();
        fclose(file);
    } else {
        mode = 1;
        yyin = stdin;
        yyparse();
    }
    return 0;

}
