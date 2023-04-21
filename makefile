CC = g++

APP_NAME = dff

PKGCONFIG = $(shell which pkg-config)
GTK_CFLAGS = $(shell $(PKGCONFIG) --cflags gtkmm-3.0)
GTK_LIBS = $(shell $(PKGCONFIG) --libs gtkmm-3.0)

CFLAGS = $(GTK_CFLAGS) -I/opt/ssl/include/
LIBS = $(GTK_LIBS) -l pthread -L/opt/ssl/lib/ -lcrypto


SRC := $(shell ls *.cpp)

OBJS = $(SRC:.cpp=.o)

all: $(APP_NAME)

%.o: %.cpp
		@$(CC) -c -o $(@F) $(CFLAGS) $<

$(APP_NAME): $(OBJS)
		@$(CC) -o $(@F) $(OBJS) $(LIBS)
run:
	./$(APP_NAME)

clean:
	rm -f $(OBJS)
	rm -f $(APP_NAME)
	rm -f vgcore.*
