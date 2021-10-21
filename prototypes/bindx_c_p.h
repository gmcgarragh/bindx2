/* bindx_c.c */
size_t bindx_c_type_size(const type_data *type);
const char *bindx_c_error_conditional(const bindx_data *d, int lex_bindx_type);
int bindx_write_c_header(FILE *fp);
int bindx_write_c_header_top(FILE *fp);
int bindx_write_c_type(FILE *fp, const bindx_data *d, const type_data *type, const char *prefix);
int bindx_write_c_dimens(FILE *fp, const bindx_data *d, const type_data *type);
int bindx_write_c_dimens_args(FILE *fp, const bindx_data *d, const type_data *type, enum lex_subprogram_argument_usage usage);
int bindx_write_c_dimens_return(FILE *fp, const bindx_data *d, const type_data *type);
int bindx_write_c_declaration(FILE *fp, const bindx_data *d, const type_data *type, const char *prefix);
int bindx_write_c_enum_mask_init(FILE *fp, struct list_data *list, const char *name, const char *prefix, int indent);
int bindx_write_c_enum_array_init(FILE *fp, struct list_data *list, const char *name, const char *prefix, int indent);
