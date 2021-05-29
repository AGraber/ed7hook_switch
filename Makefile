# TODO (Khangaroo): Make this process a lot less hacky (no, export did not work)
# See MakefileNSO

.PHONY: all clean skyline send

PYTHON := python3
ifeq (, $(shell which python3))
	# if no python3 alias, fall back to `python` and hope it's py3
	PYTHON   := python
endif

NAME 			:= $(shell basename $(CURDIR))
NAME_LOWER		:= $(shell echo $(NAME) | tr A-Z a-z)

SCRIPTS_DIR		:= scripts
BUILD_DIR 		:= build

SEND_PATCH		:= $(SCRIPTS_DIR)/sendPatch.py

MAKE_NSO		:= nso.mk

all: skyline

skyline:
	$(MAKE) all -f $(MAKE_NSO) MAKE_NSO=$(MAKE_NSO) BUILD=$(BUILD_DIR) TARGET=$(NAME)

send: all
	$(PYTHON) $(SEND_PATCH) $(IP)

clean:
	$(MAKE) clean -f $(MAKE_NSO)
