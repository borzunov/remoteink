#include "../common/messages.h"
#include "security.h"

#include <crypt.h>
#include <string.h>


#define EXPECTED_HASH_SIZE 256
char expected_hash[EXPECTED_HASH_SIZE];

#define ERR_PASSWORD_UNDEFINED "Can't read password's hash (define " \
		"password using \"inkmonitord-passwd\" or try to use \"sudo\")"

FILE *f;

void defer_security_fclose_f() {
	fclose(f);
}

ExcCode security_load_password(const char *filename) {
	f = fopen(filename, "r");
	if (f == NULL)
		PANIC(ERR_PASSWORD_UNDEFINED);
	push_defer(defer_security_fclose_f);
	
	if (fgets(expected_hash, EXPECTED_HASH_SIZE, f) == NULL)
		PANIC_WITH_DEFER(ERR_FILE_READ, filename);
	int len = strlen(expected_hash);
	while (expected_hash[len - 1] == '\n' || expected_hash[len - 1] == '\r')
		len--;
	expected_hash[len] = 0;
	
	pop_defer(defer_security_fclose_f);
	return 0;
}

int are_strings_equal_stable(const char *a, const char *b) {
	// Compare strings "a" and "b" in constant time to prevent timing attacks
	int res = 1;
	int i;
	for (i = 0; a[i] & b[i]; i++)
		res &= (a[i] == b[i]);
	if (a[i] | b[i])
		return 0;
	return res;
}

#define ERR_CRYPT "Failed to verify password's hash"

ExcCode security_check_password(
		const char *suggested_password, int *is_correct) {
	if (expected_hash[0] == '$') {
		const char *suggested_hash = crypt(suggested_password, expected_hash);
		if (suggested_hash == NULL)
			PANIC(ERR_CRYPT);
		*is_correct = are_strings_equal_stable(suggested_hash, expected_hash);
	} else {
		// Password may be not hashed (defined as plaintext)
		*is_correct = are_strings_equal_stable(
				suggested_password, expected_hash);
	}
	return 0;
}
