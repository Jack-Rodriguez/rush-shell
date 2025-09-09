CC=clang
CFLAGS=-Wall -Wextra -std=c99 -g
TARGET=shell
SRCDIR=src

# Find all .c files in src directory
SOURCES=$(wildcard $(SRCDIR)/*.c)

# Default target
all: $(TARGET)

# Build the shell (will work when you add source files)
$(TARGET): $(SOURCES)
	@if [ -z "$(SOURCES)" ]; then \
		echo "No source files found in $(SRCDIR)/. Add your .c files there."; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Clean build artifacts  
clean:
	rm -f $(TARGET)

# Run the shell
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run