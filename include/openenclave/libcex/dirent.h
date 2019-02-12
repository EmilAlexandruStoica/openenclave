/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* Licensed under the MIT License. */

#ifndef _LIBCEX_DIRENT_H
#define _LIBCEX_DIRENT_H

#if defined(_MSC_VER)
#define _NO_CRT_DIRENT_INLINE
#endif

#include <openenclave/libcex/bits/common.h>
#include <openenclave/bits/defs.h>
#include <openenclave/bits/types.h>
#include <openenclave/bits/fs.h>

OE_EXTERNC_BEGIN

/*
**==============================================================================
**
** oe-prefixed standard stream I/O functions (the "oe" namespace).
**
**==============================================================================
*/

typedef struct _OE_DIR OE_DIR;
struct oe_dirent;

OE_DIR* oe_opendir(int devid, const char* pathname);

struct oe_dirent* oe_readdir(OE_DIR* dir);

int oe_closedir(OE_DIR* dir);

/*
**==============================================================================
**
** Libc extensions:
**
**==============================================================================
*/

OE_INLINE OE_DIR* oe_opendir_nonsecure(const char* pathname)
{
    return oe_opendir(OE_DEVICE_ID_HOSTFS, pathname);
}

OE_EXTERNC_END

#endif /* _LIBCEX_DIRENT_H */