# Name of resulting executable
NAME := stw

CC := gcc

# Used only when building with Musl (only Linux)
LD := ld

CFLAGS := -Wall
LDFLAGS := 

# Number of jobs to build Musl (only Linux)
JOBS := 8

### Don't edit anything below unless you know what you are doing ###

SRC_MOD := stw.c

LDFLAGS += -o $(NAME)

DEFGOALS := cmp

ifdef MUSL
	CFLAGS += -nostdinc -I local/include
	LDSTART += -static -nostdlib -L local/lib local/lib/crt1.o
	LDEND := -lc
	DEFGOALS := musl $(DEFGOALS)
else
	LD = $(CC)
endif

def: $(DEFGOALS)

musl: musl-install.sh
	./musl-install.sh $(MUSL) `pwd`/local $(JOBS)

cmp: $(SRC_MOD) 
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $(LDSTART) $(SRC_MOD:.c=.o) $(LDEND)

clean:
	rm -f *.o $(NAME)

distclean: clean
	rm -rf local musl-1.2.2
