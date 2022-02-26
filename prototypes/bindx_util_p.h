/* bindx_util.c */
const char *bindx_version(void);
char *bxis4(int n);
char *bxis5(int n);
char *bxis(int n);
void ifprintf(FILE *fp, int n_indent, const char *fmt, ...);
void bindx_fprintf_wrap(FILE *fp, int n_wrap, char *prefix, const char *fmt, va_list ap);
