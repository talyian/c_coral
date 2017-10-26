#include "../core/expr.hh"

using namespace coral;

#include "generated/parser.hh"
Module * parse(FILE * in, const char * src);

#ifndef YY_EXTRA_TYPE
#define YY_EXTRA_TYPE void *
#endif

#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif

#define YYSTYPE yy::parser::semantic_type
#define YYLTYPE yy::location

typedef void * yyscan_t;
int yylex_init (yyscan_t* scanner);
int yylex_init_extra (YY_EXTRA_TYPE user_defined,yyscan_t* scanner);
int yylex_destroy (yyscan_t yyscanner );
int yyget_debug (yyscan_t yyscanner );
void yyset_debug (int debug_flag ,yyscan_t yyscanner );
void yyset_extra (YY_EXTRA_TYPE user_defined ,yyscan_t yyscanner );
void yyset_in (FILE * _in_str ,yyscan_t yyscanner );
void yyset_out (FILE * _out_str ,yyscan_t yyscanner );
int yyget_lineno (yyscan_t yyscanner );
void yyset_lineno (int _line_number ,yyscan_t yyscanner );
int yyget_column (yyscan_t yyscanner );
void yyset_column (int _column_no ,yyscan_t yyscanner );
void yyset_lval (YYSTYPE * yylval_param ,yyscan_t yyscanner );
yy_size_t yyget_leng (yyscan_t yyscanner );
char *yyget_text (yyscan_t yyscanner );
char *yyget_text (yyscan_t yyscanner );

YY_BUFFER_STATE yy_scan_string (const char * yystr , yyscan_t yyscanner);

int yylex(YYSTYPE * lval, YYLTYPE * loc, yyscan_t scanner);

Module * parse(FILE * in, const char * src);
