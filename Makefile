
BIN_DIR=/usr/local/sbin
ETC_DIR=/etc

PROGRAM = router_control
FILES_TO_COMPILE = main str params message config tools dialog login arp ping
HEADERS_TO_COMPILE = define

CC = g++
OS_FLAGS = "-DFREEBSD"
CCFLAGS = -g -Wall -DBIN_DIR="\"${BIN_DIR}\"" ${OS_FLAGS}
LDFLAGS =
C_SUFFIX = cpp
H_SUFFIX = h

HEADERS = $(addsuffix .$(H_SUFFIX), $(HEADERS_TO_COMPILE))
OBJS = $(addsuffix .o, $(FILES_TO_COMPILE))
FILES = $(addsuffix .$(C_SUFFIX), $(FILES_TO_COMPILE))

MAKEFLAGS = --no-print-directory --silent

all: compile-all link-all

ifeq (${OS_FLAGS},"-DFREEBSD")
else
endif

compile-all: $(OBJS)
	echo "Compile done"

link-all: $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROGRAM) $(OBJS)

%.o: %.$(C_SUFFIX) %.$(H_SUFFIX) $(HEADERS)
	$(CC) $(CCFLAGS) -c $< -o $@
	echo "$@ - done"

clean:
	rm -rf $(OBJS) $(PROGRAM)
	echo "Clean done"

install:
	mkdir -p $(BIN_DIR)
	cp -f $(PROGRAM) $(BIN_DIR)/$(PROGRAM)
	if [ ! -f $(ETC_DIR)/$(PROGRAM) ]; then \
		cp config $(ETC_DIR)/$(PROGRAM); \
	fi
	cp -f rc_start_ping.sh $(BIN_DIR)/rc_start_ping.sh
	cp -f rc_stop_ping.sh $(BIN_DIR)/rc_stop_ping.sh
ifeq (${OS_FLAGS},"-DFREEBSD")
	chown root:wheel $(BIN_DIR)/rc_start_ping.sh
	chown root:wheel $(BIN_DIR)/rc_stop_ping.sh
else
	chown root:root $(BIN_DIR)/rc_start_ping.sh
	chown root:root $(BIN_DIR)/rc_stop_ping.sh
endif
	chmod 500 $(BIN_DIR)/rc_start_ping.sh
	chmod 500 $(BIN_DIR)/rc_stop_ping.sh
	echo "Install done"

uninstall:
	rm -rf $(BIN_DIR)/$(PROGRAM)
	rm -rf $(BIN_DIR)/rc_start_ping.sh
	rm -rf $(BIN_DIR)/rc_stop_ping.sh
	rm -rf $(ETC_DIR)/$(PROGRAM)
	echo "Uninstall done"

.NOEXPORT:
