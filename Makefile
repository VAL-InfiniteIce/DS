.SUFFIXES:
CC = gcc

CCFLAGS = -std=c11 -Wall
CHFLAGS = -std0c11 -Wall -Werror
CEFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror

FLAGS = $(CEFLAGS)
NAME = loesung

FILE = $(NAME).c
FOLDER = test_cases
CFILES = loesung.c
EXAMPLE = example01.dat

TARGET = $(CFILES:%.c=%.o)

# Compilierung
%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

# Targets
all: $(TARGET)
	$(CC) $(FLAGS) $^ -o $(NAME)

run: all
	cat $(FOLDER)/$(EXAMPLE) | ./$(NAME)
	$(RM) $(NAME) *.o

val: all
	cat $(FOLDER)/$(EXAMPLE) | valgrind --leak-check=full ./$(NAME) > /dev/null 	
	$(RM) $(NAME) *.o

clean: 
	$(RM) $(NAME) *.o

test: all
	cat $(FOLDER)/example01.dat | ./$(NAME) | ./check_result $(FOLDER)/example01.out
	cat $(FOLDER)/example02.dat | ./$(NAME) | ./check_result $(FOLDER)/example02.out
	cat $(FOLDER)/example03.dat | ./$(NAME) | ./check_result $(FOLDER)/example03.out
	cat $(FOLDER)/example04.dat | ./$(NAME) | ./check_result $(FOLDER)/example04.out
	cat $(FOLDER)/example05.dat | ./$(NAME) | ./check_result $(FOLDER)/example05.out
	cat $(FOLDER)/example06.dat | ./$(NAME) | ./check_result $(FOLDER)/example06.out
	cat $(FOLDER)/example07.dat | ./$(NAME) | ./check_result $(FOLDER)/example07.out
	cat $(FOLDER)/example08.dat | ./$(NAME) | ./check_result $(FOLDER)/example08.out
	cat $(FOLDER)/example09.dat | ./$(NAME) | ./check_result $(FOLDER)/example09.out
	$(RM) $(NAME) *.o

vtest: all
	cat $(FOLDER)/example01.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example01.out
	cat $(FOLDER)/example02.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example02.out
	cat $(FOLDER)/example03.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example03.out
	cat $(FOLDER)/example04.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example04.out
	cat $(FOLDER)/example05.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example05.out
	cat $(FOLDER)/example06.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example06.out
	cat $(FOLDER)/example07.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example07.out
	cat $(FOLDER)/example08.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example08.out
	cat $(FOLDER)/example09.dat | valgrind ./$(NAME) | ./check_result $(FOLDER)/example09.out
	$(RM) $(NAME) *.o
