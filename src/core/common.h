#ifndef COMMON_H_
#define COMMON_H_

//#include "GeoAlgConfig.h"

typedef struct GALGError {
	int errnum;
	char const *msg;
} GALGError;


#define RETURNIF(assertion, errnum, msg) do {      \
	if (assertion) {                               \
		GALGError err = {errnum, msg};             \
		return err;                                \
	}                                              \
} while(0);                                        \

#define RETURNIFERROR(galgerror) do { \
	if (galgerror.errnum != 0){     \
		return galgerror;           \
	}                               \
} while(0);                         \




#endif //COMMON_H_