TARGET	= client
SRCS	= client.c connection.c daihinmin.c NN.c 
OBJS	= client.o connection.o daihinmin.o NN.o 

CC = gcc
LIBS =   
CFLAGS = -g -O2

all: $(TARGET)

$(TARGET): $(OBJS) $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)


Makefile: Makefile.in
	./configure

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
