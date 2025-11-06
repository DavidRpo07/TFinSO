CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS =
OBJDIR = build
SRCDIR = src

SRC = $(SRCDIR)/main.c \
      $(SRCDIR)/verdir.c \
      $(SRCDIR)/pipeline.c \
      $(SRCDIR)/compress/rle.c \
      $(SRCDIR)/crypto/vigenere.c

OBJ = $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
BIN = gsea

# Regla principal
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar cada .c en .o dentro de build/
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -rf $(OBJDIR) $(BIN)

.PHONY: all clean
