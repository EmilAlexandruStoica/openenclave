// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/*
**==============================================================================
**
** host_resolver.h
**
**     Definition of the host_resolver internal types and data
**
**==============================================================================
*/

#ifndef _OE_HOST_RESOLVER_H__
#define _OE_HOST_RESOLVER_H__

OE_EXTERNC_BEGIN

oe_device_t* oe_resolver_get_hostsock();
void oe_resolver_install_hostsock();

OE_EXTERNC_END

#endif /* _OE_HOST_RESOLVER_H */