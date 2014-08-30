# Makefile
PROG = SmartVehicle
CC = g++
CPPFLAGS = -g -Wall
LDFLAGS = -lpthread -lwiringPi -lm -lrt

SRCS = SmartVehicle.cpp ZWaitObj.cpp ZSyncObj.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

$(PROG): $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(PROG) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(PROG) $(OBJS)
