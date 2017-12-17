/* bindx_util.c */
const char *bindx_version(void);
char *bindx_indent_str(int n);
char *bxis(int n);
void ifprintf(FILE *fp, int n_indent, const char *fmt, ...);
const char *xrtm_error_conditional(int lex_bindx_type);
void bindx_fprintf_wrap(FILE *fp, int n_wrap, char *prefix, const char *fmt, va_list ap);
