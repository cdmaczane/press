typedef enum
{
	file_mode_read,
	file_mode_write
} file_mode;

static FILE*	open_file(const char* path, file_mode mode);
static void		handle_error(const char* format, ...);