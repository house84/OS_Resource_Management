CC = gcc
CFLAGS = -g 

TARGET1 = oss
OBJ1 = oss.o

TARGET2 = user
OBJ2 = user.o

HEADERS = headers.h shared.h oss.h user.h

.SUFFIXES: .c .o

ALL: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ1) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJ1) 

$(TARGET2): $(OBJ2) 
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJ2)


clean: 
	rm -f $(TARGET1) $(TARGET2) *.o
