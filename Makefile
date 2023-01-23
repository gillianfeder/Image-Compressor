# Makefile for arith (Comp 40 Assignment 3)
# 
# Includes build rules for ppmdiff, compress40, and testing.
#
# This Makefile is more verbose than necessary.  In each assignment
# we will simplify the Makefile using more powerful syntax and implicit rules.
#
# Last updated: 10/24/22


############## Variables ###############

CC = gcc # The compiler being used

# Updating include path to use Comp 40 .h files and CII interfaces
IFLAGS = -I/comp/40/build/include -I/usr/sup/cii40/include/cii

# Compile flags
# Set debugging information, allow the c99 standard,
# max out warnings, and use the updated include path
# CFLAGS = -g -std=c99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)
# 
# For this assignment, we have to change things a little.  We need
# to use the GNU 99 standard to get the right items in time.h for the
# the timing support to compile.
# 
CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

# Linking flags
# Set debugging information and update linking path
# to include course binaries and CII implementations
LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64

# Libraries needed for linking
# All programs cii40 (Hanson binaries) and *may* need -lm (math)
# 40locality is a catch-all for this assignment, netpbm is needed for pnm
# rt is for the "real time" timing library, which contains the clock support
LDLIBS = -l40locality -larith40 -lnetpbm -lcii40 -lm -lrt

# Collect all .h files in your directory.
# This way, you can never forget to add
# a local .h file in your dependencies.
#
# This bugs Mark, who dislikes false dependencies, but
# he agrees with Noah that you'll probably spend hours 
# debugging if you forget to put .h files in your 
# dependency list.
INCLUDES = $(shell echo *.h)

############### Rules ###############

all: ppmdiff 40image-6    			#add others later!


## Compile step (.c files -> .o files)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


## Linking step (.o -> executable program)

ppmdiff: ppmdiff.o a2blocked.o a2plain.o uarray2.o uarray2b.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

test: testing.o bitpack.o compress.o decompress.o sharedHelpers.o a2blocked.o \
		a2plain.o uarray2.o uarray2b.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

40image-6: 40image.o compress40.o compress.o decompress.o sharedHelpers.o \
		bitpack.o a2blocked.o a2plain.o uarray2.o uarray2b.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

bitpack_test: bitpackTest.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# timing_test: timing_test.o cputiming.o
# 	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS) 

# ppmtrans: ppmtrans.o cputiming.o a2blocked.o a2plain.o uarray2.o uarray2b.o
# 	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)


clean:
	rm -f ppmdiff *.o

