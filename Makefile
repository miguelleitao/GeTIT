

PROGS=getit_l1 getit_l2

CC ?= musl-gcc
CFLAGS ?= -Os -Wall -Wextra -Wno-deprecated-declarations -ffunction-sections -fdata-sections
STRIP ?= strip

ARCH ?= x86_64
BUILD_DIR = build/$(ARCH)
OBJ_DIR   = $(BUILD_DIR)/obj
BIN_DIR   = $(BUILD_DIR)/bin
BIN_PROGS = $(addprefix $(BIN_DIR)/, $(PROGS))
OBJECTS   = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(PROGS)))

# Colors
RED    := \033[31m
GREEN  := \033[32m
YELLOW := \033[33m
BLUE   := \033[34m
BOLD   := \033[1m
RESET  := \033[0m

default: progs

${BIN_DIR}:
	mkdir -p $@

${OBJ_DIR}:
	mkdir -p $@

progs: ${BIN_DIR} ${OBJ_DIR} ${BIN_PROGS}
	@echo "Done."

${BIN_DIR}/%: $(OBJ_DIR)/%.o
	@printf "${GREEN}Building executable $@${RESET}\n"
	@$(CC) -Os -o $@ $^ -lm 2>&1
	@$(STRIP) $@

$(OBJ_DIR)/%.o: %.c getit.h
	@printf "$(BLUE)Building C object $@$(RESET)\n"
	@$(CC) -c $(CFLAGS) $< -o $@ 

submodules:
	git submodule update --init ----remote
 
clean:
	-rm -rf ${PROGS}
	-rm -rf ${BIN_PROGS}
	-rm -rf ${OBJECTS}
	-rm -rf $(BUILD_DIR)
	-rm -rf *.o

