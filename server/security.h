#ifndef SECURITY_H
#define SECURITY_H

#include "../common/exceptions.h"


extern ExcCode security_load_password(const char *filename);
extern ExcCode security_check_password(
		const char *suggested_password, int *is_correct);

extern ExcCode security_change_password(const char *filename);


#endif
