NAME        = ft_ping

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -Iinc

SRC_DIR     = src
OBJ_DIR     = obj
INC_DIR     = inc

SRC_FILES   = main.c \
			  dns.c \
			  request.c \
			  receiver.c \
			  display.c \
			  signal.c \


OBJ_FILES   = $(SRC_FILES:%.c=$(OBJ_DIR)/%.o)
DEPS        = $(OBJ_FILES:%.o=%.d)

DEST        ?= google.com

all: $(NAME)

$(NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(NAME) -lm

run: all
	sudo ./$(NAME) $(DEST)

valgrind: all
	sudo valgrind --track-fds=yes ./$(NAME) $(DEST)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re run
