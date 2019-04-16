# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# Detect C and C++ compiler options
# if not gcc, default to clang-7

COMPILER=$(notdir $(CC))
ifeq ($(COMPILER), gcc)
        USE_GCC = true
endif

ifeq ($(USE_GCC),)
        CC = clang-7
        COMPILER=clang
endif

CFLAGS=$(shell pkg-config oeenclave-clang --cflags)
LDFLAGS=$(shell pkg-config oeenclave-clang --libs)


build:
	@ echo "Compilers used: $(CC), $(CXX)"
	oeedger8r ../randomSample.edl --untrusted
	$(CC) -g -c $(CFLAGS) host.c
	$(CC) -g -c $(CFLAGS) randomSample_u.c
	$(CC) -o randomSamplehost randomSample_u.o host.o $(LDFLAGS)

clean:
	rm -f randomSamplehost host.o randomSample_u.o randomSample_u.c randomSample_u.h randomSample_args.h
