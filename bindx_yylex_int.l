/******************************************************************************%
**
**    Copyright (C) 2011-2017 Greg McGarragh <gregm@atmos.colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

%{
#define BISON_INCLUDE

#include <gutil.h>

#include "bindx_parse.h"
#include "bindx_parse_int.h"

void comment(locus_data *locus);
%}


%option noyywrap


%{
#define YY_USER_ACTION locus->character += yyleng;
%}


%%
[ \t]+					;

"/*"					{ comment(locus); }
"//"[^\n]*              		{ /* consume //-comment */ }


,					{ return ','; }
:					{ return ':'; }
;					{ return ';'; }

"void"					{ return LEX_BINDX_TYPE_VOID; }
"enum"					{ return LEX_BINDX_TYPE_ENUM; }
"char"					{ return LEX_BINDX_TYPE_CHAR; }
"int"					{ return LEX_BINDX_TYPE_INT; }
"double"				{ return LEX_BINDX_TYPE_DOUBLE; }

"prefix"				{ return LEX_ITEM_PREFIX; }
"include"				{ return LEX_ITEM_INCLUDE; }
"enumeration"				{ return LEX_ITEM_ENUMERATION; }
"structure"				{ return LEX_ITEM_STRUCTURE; }
"subprogram_general"			{ return LEX_ITEM_SUBPROGRAM_GENERAL; }
"subprogram_init"			{ return LEX_ITEM_SUBPROGRAM_INIT; }
"subprogram_free"			{ return LEX_ITEM_SUBPROGRAM_FREE; }

"in"					{ return LEX_SUBPROGRAM_ARGUMENT_USAGE_IN; }
"out"					{ return LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT; }

"enum_external"				{ return LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_EXTERNAL; }
"enum_mask"				{ return LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_MASK; }
"enum_array"				{ return LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_ARRAY; }
"list_size"				{ return LEX_SUBPROGRAM_ARGUMENT_OPTION_LIST_SIZE; }


[A-Za-z_][A-Za-z0-9_:]*		{
     lex_type->s = strdup(yytext);
     return LEX_TYPE_IDENTIFIER;
}

\'[^\'\n]*\' |
\"[^\"\n]*\"				{
     char c;
     c = yytext[yyleng - 1];
     yytext[yyleng - 1] = '\0';
     lex_type->s = strdup(yytext + 1);
     yytext[yyleng - 1] = c;
     return LEX_TYPE_STRING;
}

-?[0-9]+"."[0-9]+([eE][+-]?[0-9]+)?	|
-?[0-9]+"."([eE][+-]?[0-9]+)?		|
-?"."[0-9]+([eE][+-]?[0-9]+)?		{
     lex_type->d = atof(yytext);
     return LEX_TYPE_DOUBLE;
}

-?[0-9]+				{
     lex_type->l = atol(yytext);
     return LEX_TYPE_LONG;
}

\n					{ locus->line++; locus->character = 1; }

.					{ return yytext[0]; }

%%


char *get_yytext() {

     return yytext;
}



void comment(locus_data *locus) {
	char c, prev = 0;
#ifdef __cplusplus
	while ((c = yyinput()) != 0)      /* (EOF maps to 0) */
#else
	while ((c =   input()) != 0)      /* (EOF maps to 0) */
#endif
	{
		if (c == '/' && prev == '*')
			return;
                else
                if (c == '\n')
                     locus->line++;
		prev = c;
	}
	parse_error(locus, "unterminated comment");
}



void yypreinclude(FILE *fp) {

     yypush_buffer_state(yy_create_buffer(fp, YY_BUF_SIZE));
}



void yypostinclude() {

     yypop_buffer_state();
}



void yyrewind(int r, lex_type_data *type) {

    int i;

    for (i = yyleng - 1; i >= 0; --i)
        unput(yytext[i]);

    if (r == LEX_TYPE_STRING || r == LEX_TYPE_IDENTIFIER)
         free(type->s);
}
