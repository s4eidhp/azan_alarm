CC=gcc
CFLAGS=-lcurl
TARGET=main

$(TARGET): main.c
	$(CC) -o $(TARGET) main.c $(CFLAGS)

clean:
	rm -f $(TARGET)

