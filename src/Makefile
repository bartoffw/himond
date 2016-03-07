
CXX=g++
CPPFLAGS += -Wall -g

STATSD_INC = -I ./
STATSD_LIB = -L ./lib -lstatsd_client -lsystem_monitor

TARGET1			= statsd_client
TARGET1_LIB		= lib/lib$(TARGET1).a
TARGET1_HEADER	= lib/$(TARGET1).h
TARGET1_SOURCE	= lib/$(TARGET1).cpp
TARGET1_OBJ		= lib/$(TARGET1).o

TARGET2         = system_monitor
TARGET2_LIB		= lib/lib$(TARGET2).a
TARGET2_HEADER	= lib/$(TARGET2).h
TARGET2_SOURCE	= lib/$(TARGET2).cpp
TARGET2_OBJ		= lib/$(TARGET2).o

TARGETS         = $(TARGET1_LIB) $(TARGET2_LIB) test_client himond

all: $(TARGETS)

$(TARGET1_LIB): $(TARGET1_OBJ)
	ar crus $@ $<

$(TARGET1_OBJ): $(TARGET1_SOURCE) $(TARGET1_HEADER)
	$(CXX) -c -o $@ $< $(CPPFLAGS)

$(TARGET2_LIB): $(TARGET2_OBJ)
	ar crus $@ $<

$(TARGET2_OBJ): $(TARGET2_SOURCE) $(TARGET2_HEADER)
	$(CXX) -c -o $@ $< $(CPPFLAGS)

test_client: test_client.cpp
	$(CXX) -o $@ $< $(CPPFLAGS) $(STATSD_INC) $(STATSD_LIB)

himond: himond.cpp
	$(CXX) -o $@ $< $(CPPFLAGS) $(STATSD_INC) $(STATSD_LIB)

clean:
	rm -f $(TARGET1_OBJ) $(TARGET1_LIB) $(TARGET2_OBJ) $(TARGET2_LIB) test_client himond
