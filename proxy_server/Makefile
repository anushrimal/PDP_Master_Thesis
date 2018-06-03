CXX=g++

OBJECTS=RestServer.o DBHandler.o SendFile.o ParseConfig.o main.o

SRCS=RestServer.cpp DBHandler.cpp SendFile.cpp ParseConfig.cpp main.cpp

INCLUDES=-I ./dependency/include/asio/ -I ./dependency/include/restbed/include/  -I ./dependency/include/ -I ./dependency/include/boost/ -I ./dependency/include/

CPPFLAGS= -std=gnu++11 -fpermissive #-g

LIBDIRS= -L ./dependency/lib/

LIBS= -pthread -lrocksdb -lrestbed 

TARGET=pdpServer

ParseConfig.o : ParseConfig.cpp 
	$(CXX) $(CPPFLAGS) -c -o ParseConfig.o $(INCLUDES) ParseConfig.cpp

RestServer.o : RestServer.cpp DBHandler.o
	$(CXX) $(CPPFLAGS) -c -o RestServer.o $(INCLUDES) RestServer.cpp

DBHandler.o : DBHandler.cpp
	$(CXX) $(CPPFLAGS) -c -o DBHandler.o $(INCLUDES) DBHandler.cpp

SendFile.o : SendFile.cpp
	$(CXX) $(CPPFLAGS) -c -o SendFile.o $(INCLUDES) SendFile.cpp

main.o :  main.cpp RestServer.o DBHandler.o SendFile.o ParseConfig.o
	 $(CXX) $(CPPFLAGS) -c -o main.o  $(INCLUDES) main.cpp


all : $(OBJECTS) 
	$(CXX) $(CPPFLAGS) -o $(TARGET) $(INCLUDES) $(OBJECTS) $(LIBDIRS) $(LIBS) 

clean:
	rm *.o
	rm $(TARGET)
