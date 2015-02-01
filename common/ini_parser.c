#include "exceptions.h"
#include "ini_parser.h"
#include "messages.h"

#include <stdio.h>
#include <string.h>

#define ERR_INI_PARAM_BEFORE_SECTION "Can't define parameter \"%s\" \
before section definition in \"%s\""
#define ERR_INI_UNKNOWN_SECTION "Unknown section \"%s\" in \"%s\""
#define ERR_INI_UNKNOWN_KEY "Unknown key \"%s\" in section \"%s\" in \"%s\""
#define ERR_INI_AMBIGOUS_LINE "Ambiguous line \"%s\" in \"%s\""
#define ERR_INI_SAVE_ANY_KEY "Can't save parameters declared with INI_ANY_KEY"
#define ERR_INI_REQUIRED_PARAM "Required parameter \"%s\" in section \"%s\" "\
		"not found in \"%s\""

FILE *f;

void defer_ini_parser_fclose_f() {
	fclose(f);
}

#define LINE_BUFFER_SIZE 256
char line_buffer[LINE_BUFFER_SIZE];

ExcCode ini_load(const char *filename, const struct IniSection sections[]) {
	f = fopen(filename, "r");
	if (f == NULL)
		PANIC(ERR_FILE_OPEN_FOR_READING, filename);
	push_defer(defer_ini_parser_fclose_f);
	
	const struct IniSection *cur_section = NULL;
	while (fgets(line_buffer, LINE_BUFFER_SIZE, f)) {
		if (ferror(f))
			PANIC_WITH_DEFER(ERR_FILE_READ, filename);
		
		int len = strlen(line_buffer);
		if (line_buffer[len - 1] == '\n')
			len--;
		line_buffer[len] = 0;
		
		// Handle empty lines and comments
		if (!len || line_buffer[0] == ';' || line_buffer[0] == '#')
			continue;
		
		// Handle sections
		if (line_buffer[0] == '[' && line_buffer[len - 1] == ']') {
			line_buffer[len - 1] = 0;
			cur_section = NULL;
			int i;
			for (i = 0; sections[i].title != NULL; i++)
				if (!strcmp(line_buffer + 1, sections[i].title)) {
					cur_section = &sections[i];
					break;
				}
			if (cur_section == NULL)
				PANIC_WITH_DEFER(ERR_INI_UNKNOWN_SECTION,
						line_buffer + 1, filename);
			continue;
		}
		
		// Handle keys
		int i;
		for (i = 0; i < len - 2; i++)
			if (
				line_buffer[i] == ' ' && line_buffer[i + 1] == '=' &&
				line_buffer[i + 2] == ' '
			) {
				line_buffer[i] = 0;
				
				if (cur_section == NULL)
					PANIC_WITH_DEFER(ERR_INI_PARAM_BEFORE_SECTION,
							line_buffer, filename);
				
				int j;
				for (j = 0; cur_section->params[j].key != NULL; j++) {
					struct IniParam *cur_param = &cur_section->params[j];
					if (
						!strcmp(INI_ANY_KEY, cur_param->key) ||
						!strcmp(line_buffer, cur_param->key)
					) {
						TRY_WITH_DEFER(cur_param->setter(
								line_buffer, line_buffer + i + 3));
						cur_param->expected = 0;
						break;
					}
				}
				if (cur_section->params[j].key == NULL)
					PANIC_WITH_DEFER(ERR_INI_UNKNOWN_KEY,
							line_buffer, cur_section->title, filename);
				break;
			}
			
		if (i == len - 2)
			PANIC_WITH_DEFER(ERR_INI_AMBIGOUS_LINE, line_buffer, filename);
	}
	
	pop_defer(defer_ini_parser_fclose_f);
	
	for (int i = 0; sections[i].title != NULL; i++) {
		const struct IniSection *cur_section = &sections[i];
		for (int j = 0; cur_section->params[j].key != NULL; j++) {
			const struct IniParam *cur_param = &cur_section->params[j];
			if (cur_param->expected)
				PANIC(ERR_INI_REQUIRED_PARAM, cur_param->key,
						cur_section->title, filename);
		}
	}
	return 0;
}

#define VALUE_BUFFER_SIZE 256
char value_buffer[VALUE_BUFFER_SIZE];

ExcCode ini_save(const char *filename, const struct IniSection sections[]) {
	f = fopen(filename, "w");
	if (f == NULL)
		PANIC(ERR_FILE_OPEN_FOR_WRITING, filename);
	push_defer(defer_ini_parser_fclose_f);
	
	for (int i = 0; sections[i].title != NULL; i++) {
		const struct IniSection *cur_section = &sections[i];
		if (fprintf(f, "[%s]\n", cur_section->title) < 0)
			PANIC_WITH_DEFER(ERR_FILE_WRITE, filename);
		for (int j = 0; cur_section->params[j].key != NULL; j++) {
			const struct IniParam *cur_param = &cur_section->params[j];
			if (!strcmp(INI_ANY_KEY, cur_param->key))
				PANIC_WITH_DEFER(ERR_INI_SAVE_ANY_KEY);
			TRY_WITH_DEFER(cur_param->getter(cur_param->key,
					value_buffer, VALUE_BUFFER_SIZE));
			if (fprintf(f, "%s = %s\n", cur_param->key, value_buffer) < 0)
				PANIC_WITH_DEFER(ERR_FILE_WRITE, filename);
		}
	}
	
	pop_defer(defer_ini_parser_fclose_f);
	return 0;
}
