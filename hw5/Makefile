CC := gcc
AR := ar
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
LIBD := lib
INCD := include
DEMOD := demo

EXEC := charla
TEST_EXEC := $(EXEC)_tests

MAIN := $(BLDD)/main.o
LIB := $(LIBD)/$(EXEC).a
LIB_DB := $(LIBD)/$(EXEC)_debug.a

ALL_SRCF := $(wildcard $(SRCD)/*.c)
ALL_TESTF := $(wildcard $(TSTD)/*.c)
ALL_OBJF := $(patsubst $(SRCD)/%, $(BLDD)/%, $(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out $(MAIN), $(ALL_OBJF))
UTIL_OBJF := $(patsubst $(UTILD)/%, $(UTILD)/%, $(ALL_SRCF:.c=.o))

INC := -I $(INCD)

CFLAGS := -Wall -Werror -Wno-unused-function -MMD
DFLAGS := -g -DDEBUG -DCOLOR
PRINT_STAMENTS := -DERROR -DSUCCESS -DWARN -DINFO

STD := -std=gnu11
TEST_LIB := -lcriterion
LIBS := $(LIB) -lpthread -lm
LIBS_DB := $(LIB_DB) -lpthread -lm
EXCLUDES := $(INCD)/excludes.h

CFLAGS += $(STD)

.PHONY: clean all setup debug

all: setup $(EXEC) $(EXCLUDES) $(TEST_EXEC)

setup:
	mkdir -p $(BIND) $(BLDD)

debug: CFLAGS += $(DFLAGS) $(PRINT_STAMENTS)
debug: LIBS := $(LIBS_DB)
debug: all

$(EXEC): $(MAIN) $(ALL_FUNCF) $(LIB)
	$(CC) $(MAIN) $(ALL_FUNCF) -o $(BIND)/$(EXEC) $(LIBS)

$(TEST_EXEC): $(ALL_FUNCF) $(LIB)
	$(CC) $(CFLAGS) $(INC) $(ALL_TESTF) $(ALL_FUNCF) -o $(BIND)/$(TEST_EXEC) $(TEST_LIB) $(LIBS)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf $(BLDD) $(BIND)

$(EXCLUDES): $(EXEC)
	rm -f $@
	touch $@
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_CLIENT > /dev/null; then \
           echo "#define NO_CLIENT" >> $@; \
        fi
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_CLIENT_REGISTRY > /dev/null; then \
           echo "#define NO_CLIENT_REGISTRY" >> $@; \
        fi
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_MAILBOX > /dev/null; then \
           echo "#define NO_MAILBOX" >> $@; \
        fi
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_MAIN > /dev/null; then \
           echo "#define NO_MAIN" >> $@; \
        fi
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_PROTOCOL > /dev/null; then \
           echo "#define NO_PROTOCOL" >> $@; \
        fi
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_SERVER > /dev/null; then \
           echo "#define NO_SERVER" >> $@; \
        fi
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_USER > /dev/null; then \
           echo "#define NO_USER" >> $@; \
        fi
	if nm $(BIND)/$(EXEC) | grep INSTRUCTOR_USER_REGISTRY > /dev/null; then \
           echo "#define NO_USER_REGISTRY" >> $@; \
        fi

.PRECIOUS: $(BLDD)/*.d
-include $(BLDD)/*.d
