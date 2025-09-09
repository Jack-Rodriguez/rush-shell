CC=clang
CFLAGS=-Wall -Wextra -std=c99 -g
TARGET=rush-shell
SOURCE=shell.c

# Default target
all: $(TARGET)

# Build the shell
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Install (copy to /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Run the shell
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG
debug: $(TARGET)

.PHONY: all clean install uninstall run debug