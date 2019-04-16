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

CFLAGS=$(shell pkg-config oeenclave-$(COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oeenclave-$(COMPILER) --libs)

all:
	$(MAKE) build
	$(MAKE) keys
	$(MAKE) sign

build:
	@ echo "Compilers used: $(CC), $(CXX)"
	oeedger8r ../randomSample.edl --trusted
	$(CC) -g -c $(CFLAGS) enc.c -o enc.o
	$(CC) -g -c $(CFLAGS) randomSample_t.c -o randomSample_t.o
	$(CC) -o randomSampleenc randomSample_t.o enc.o $(LDFLAGS)

sign:
	oesign sign -e randomSampleenc -c randomSample.conf -k private.pem

clean:
	rm -f enc.o randomSampleenc randomSampleenc.signed private.pem public.pem randomSample_t.o randomSample_t.h randomSample_t.c randomSample_args.h

keys:
	openssl genrsa -out private.pem -3 3072
	openssl rsa -in private.pem -pubout -out public.pem
