#ifndef NSYNC_OPENSSL_H
#define NSYNC_OPENSSL_H

// suppress deprecation warnings in OSX >= 10.7

#if defined(__APPLE__)

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif // __clang__

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif // __GNUC__

#endif


#include <openssl/evp.h>

#endif // NSYNC_OPENSSL_H
