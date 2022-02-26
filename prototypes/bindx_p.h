/* bindx.c */
int main(int argc, char *argv[]);
int bindx_write_x(const bindx_data *bindx_int, const char *name, int n_out_files, char **out_files, int (*write)(FILE **, const bindx_data *, const char *));
void usage(void);
