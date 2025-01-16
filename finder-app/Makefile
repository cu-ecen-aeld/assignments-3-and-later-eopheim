CC := gcc

SRC := writer.c
TARGET = writer

all: $(TARGET).o
	$(CROSS_COMPILE)$(CC) -o $(TARGET) $(TARGET).o

$(TARGET).o : $(SRC)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -c $(SRC) $(LDFLAGS)

clean:
	-rm -f *.o $(TARGET) *.elf *.map
