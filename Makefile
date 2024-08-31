# Makefile for a C project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

# Project name and files
NAME = my_project
SRCS = myMain.c
OBJS = $(SRCS:.c=.o)

# Default rule to compile everything
all: $(NAME)

# Linking object files to create the final executable
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

# Compiling C files into object filest
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

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
