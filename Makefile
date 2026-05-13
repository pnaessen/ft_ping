NAME        = ft_ping

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -Iinc

SRC_DIR     = src
OBJ_DIR     = obj
INC_DIR     = inc
TEST_DIR    = tests

SRC_FILES   = main.c \
			  dns.c \
			  request.c \
			  receiver.c \
			  display.c \
			  signal.c \
			  parsing.c \
			  utils.c \
			  network_factory.c \


OBJ_FILES   = $(SRC_FILES:%.c=$(OBJ_DIR)/%.o)
DEPS        = $(OBJ_FILES:%.o=%.d)
TEST_NAME   = test_ft_ping
TEST_SRC    = $(TEST_DIR)/test_main.c
TEST_OBJS   = $(OBJ_DIR)/parsing.o \
			  $(OBJ_DIR)/utils.o \
			  $(OBJ_DIR)/request.o \
			  $(OBJ_DIR)/receiver.o \
			  $(OBJ_DIR)/display.o \
			  $(OBJ_DIR)/network_factory.o \
			  $(OBJ_DIR)/dns.o

DEST        ?= google.com

all: $(NAME)

test: $(TEST_NAME)
	./$(TEST_NAME)

$(NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(NAME) -lm

$(TEST_NAME): $(TEST_SRC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_SRC) $(TEST_OBJS) -o $(TEST_NAME) -lm

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
	rm -f $(NAME) $(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re run test
