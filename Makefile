# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -Ihdr

# Directories
SRCDIR = src
OBJDIR = obj
HDRDIR = hdr
OUTDIR = output

# Project name and files
NAME = $(OUTDIR)/ft_ping
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Default rule to compile everything
all: $(NAME)

# Linking object files to create the final executable
$(NAME): $(OBJS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

# Compiling C files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure the object directory exists
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Clean rule to remove object files
clean:
	rm -f $(OBJS)

# Full clean rule to remove object files and the executable
fclean: clean
	rm -f $(NAME)

# Rebuild everything
re: fclean all

# Phony targets to avoid conflicts with files named as the targets
.PHONY: all clean fclean re
