#include "../common/messages.h"
#include "security.h"

#include <assert.h>
#include <crypt.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


#define EXPECTED_HASH_SIZE 256
char expected_hash[EXPECTED_HASH_SIZE];

#define ERR_PASSWORD_UNDEFINED "Can't read password's hash (define " \
		"password using \"inkmonitord passwd\" or try to use \"sudo\")"

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

#define ERR_CRYPT_VERIFY "Failed to verify password's hash"

ExcCode security_check_password(
		const char *suggested_password, int *is_correct) {
	if (expected_hash[0] == '$') {
		const char *suggested_hash = crypt(suggested_password, expected_hash);
		if (suggested_hash == NULL)
			PANIC(ERR_CRYPT_VERIFY);
		*is_correct = are_strings_equal_stable(suggested_hash, expected_hash);
	} else {
		// Password may be not hashed (defined as plaintext)
		*is_correct = are_strings_equal_stable(
				suggested_password, expected_hash);
	}
	return 0;
}


#define NEW_PASSWORD_SIZE 256
char new_password[NEW_PASSWORD_SIZE];

#define ERR_GETPASS "Failed to read password"
#define ERR_PASSWORD_CONFIRM "Passwords mismatch"
#define ERR_URANDOM "Failed to generate random bytes using /dev/urandom"

#define SALT_ALPHABET_SIZE 64
char salt_alphabet[SALT_ALPHABET_SIZE];

void fill_salt_alphabet() {
	int i = 0;
	for (int c = 'A'; c <= 'Z'; c++)
		salt_alphabet[i++] = c;
	for (int c = 'a'; c <= 'z'; c++)
		salt_alphabet[i++] = c;
	for (int c = '0'; c <= '9'; c++)
		salt_alphabet[i++] = c;
	salt_alphabet[i++] = '/';
	salt_alphabet[i++] = '.';
	assert(i == SALT_ALPHABET_SIZE);
}

#define SALT_PREFIX "$6$"
#define SALT_RANDOM_CHARS_COUNT 16

FILE *urandom_file;

void defer_fclose_urandom_file() {
	fclose(urandom_file);
}

ExcCode generate_salt(char *buffer) {
	urandom_file = fopen("/dev/urandom", "r");
	if (urandom_file == NULL)
		PANIC(ERR_URANDOM);
	push_defer(defer_fclose_urandom_file);
	
	unsigned char random_bytes[SALT_RANDOM_CHARS_COUNT];
	if (fread(random_bytes, sizeof (unsigned char), SALT_RANDOM_CHARS_COUNT,
			urandom_file) != SALT_RANDOM_CHARS_COUNT)
		PANIC_WITH_DEFER(ERR_URANDOM);
		
	pop_defer(defer_fclose_urandom_file);
	
	strcpy(buffer, SALT_PREFIX);
	int pos = strlen(SALT_PREFIX);
	for (int i = 0; i < SALT_RANDOM_CHARS_COUNT; i++)
		buffer[pos + i] = salt_alphabet[random_bytes[i] % SALT_ALPHABET_SIZE];
	buffer[pos + SALT_RANDOM_CHARS_COUNT] = '\0';
	return 0;
}

#define ERR_PASSWORD_EMPTY "Password can't be empty"
#define ERR_PASSWORD_SAVE "Can't save password (try to use \"sudo\")"
#define ERR_PASSWORD_CHMOD "Failed to set proper password file permissions"
#define ERR_CRYPT_GENERATE "Failed to hash password using defined algorithm"

#define SALT_SIZE 256
char salt[SALT_SIZE];

ExcCode security_change_password(const char *filename) {
	const char *password = getpass("New password: ");
	if (password == NULL)
		PANIC(ERR_GETPASS);
	strncpy(new_password, password, NEW_PASSWORD_SIZE - 1);
	new_password[NEW_PASSWORD_SIZE - 1] = '\0';
	password = getpass("Confirm password: ");
	if (password == NULL)
		PANIC(ERR_GETPASS);
	if (strcmp(new_password, password))
		PANIC(ERR_PASSWORD_CONFIRM);
	if (!*new_password)
		PANIC(ERR_PASSWORD_EMPTY);
	
	fill_salt_alphabet();
	TRY(generate_salt(salt));
	const char *hash = crypt(new_password, salt);
	if (hash == NULL)
		PANIC(ERR_CRYPT_GENERATE);
	
	f = fopen(filename, "w");
	if (f == NULL)
		PANIC(ERR_PASSWORD_SAVE);
	push_defer(defer_security_fclose_f);
	if (fprintf(f, "%s\n", hash) < 0)
		PANIC(ERR_PASSWORD_SAVE);
	pop_defer(defer_security_fclose_f);
	if (chmod(filename, S_IRUSR | S_IWUSR) < 0)
		PANIC(ERR_PASSWORD_CHMOD);
	return 0;
}
