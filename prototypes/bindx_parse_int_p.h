/* bindx_parse_int.c */
int subprogram_n_in_args(subprogram_data *d);
int subprogram_n_out_args(subprogram_data *d);
int subprogram_n_scaler_in_args(subprogram_data *d);
int subprogram_n_scaler_out_args(subprogram_data *d);
void bindx_init(bindx_data *d);
void bindx_parse(bindx_data *d, locus_data *locus);
void bindx_finialize(bindx_data *d);
void bindx_free(bindx_data *d);
int bindx_write(FILE *fp, const bindx_data *d);
int min_argument_rank(argument_data *args);
int max_argument_rank(argument_data *args);
