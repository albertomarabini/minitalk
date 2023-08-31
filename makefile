CC = cc
CFLAGS = -Wall -Wextra -Werror

CLIENT_NAME = client
SERVER_NAME = server

CLIENT_SRC = client.c
SERVER_SRC = server.c

CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

all: $(CLIENT_NAME) $(SERVER_NAME)

$(CLIENT_NAME): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT_NAME) $(CLIENT_OBJ)

$(SERVER_NAME): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_NAME) $(SERVER_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ)

fclean: clean
	rm -f $(CLIENT_NAME) $(SERVER_NAME)

re: fclean all

.PHONY: all clean fclean re
