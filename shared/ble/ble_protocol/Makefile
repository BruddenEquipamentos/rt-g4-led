CC=gcc
CFLAGS= -Wall -Werror -Wno-unused-function -g -MMD

SRC=ble_protocol_helper.c ble_protocol_command.c

BUILD=.

OBJ=$(addprefix $(BUILD)/, $(SRC:.c=.o))

.PHONY: clean

all: $(OBJ)

$(OBJ): $(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

clean:
	rm -f main *.o *.d
