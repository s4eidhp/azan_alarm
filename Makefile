CC=gcc
TARGET=main

$(TARGET): main.c
	$(CC) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)

