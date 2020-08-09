CXX=g++

OBJECTS=RestServer.o DBHandler.o SendFile.o ParseConfig.o Utils.o BPlusTree.o RpiAuth.o

OBJECTS_M=main.o

OBJECTS_T=test.o

SRCS=RestServer.cpp DBHandler.cpp SendFile.cpp ParseConfig.cpp Utils.cpp BPlusTree.cpp RpiAuth.cpp main.cpp

INCLUDES=-I ./dependency/include/asio/ -I ./dependency/include/restbed/include/  -I ./dependency/include/ -I ./dependency/include/boost/ -I ./dependency/include/

CPPFLAGS= -std=gnu++11 -fpermissive #-w -g

LIBDIRS= -L ./dependency/lib/

LIBS= -pthread -lrocksdb -lrestbed -lcrypto


Utils.o : Utils.cpp
	$(CXX) $(CPPFLAGS) -c -o Utils.o $(INCLUDES) Utils.cpp

BPlusTree.o : BPlusTree.cpp Utils.o
	$(CXX) $(CPPFLAGS) -c -o BPlusTree.o $(INCLUDES) BPlusTree.cpp

RpiAuth.o : RpiAuth.cpp Utils.o
	$(CXX) $(CPPFLAGS) -c -o RpiAuth.o $(INCLUDES) RpiAuth.cpp

ParseConfig.o : ParseConfig.cpp 
	$(CXX) $(CPPFLAGS) -c -o ParseConfig.o $(INCLUDES) ParseConfig.cpp

RestServer.o : RestServer.cpp DBHandler.o BPlusTree.o SendFile.o Utils.o
	$(CXX) $(CPPFLAGS) -c -o RestServer.o $(INCLUDES) RestServer.cpp

DBHandler.o : DBHandler.cpp
	$(CXX) $(CPPFLAGS) -c -o DBHandler.o $(INCLUDES) DBHandler.cpp

SendFile.o : SendFile.cpp
	$(CXX) $(CPPFLAGS) -c -o SendFile.o $(INCLUDES) SendFile.cpp

main.o :  main.cpp RestServer.o DBHandler.o SendFile.o ParseConfig.o Utils.cpp BPlusTree.cpp RpiAuth.cpp
	$(CXX) $(CPPFLAGS) -c -o main.o  $(INCLUDES) main.cpp

test.o : test.cpp RestServer.o DBHandler.o SendFile.o ParseConfig.o Utils.cpp BPlusTree.cpp RpiAuth.cpp
	$(CXX) $(CPPFLAGS) -c -o test.o  $(INCLUDES) test.cpp


all : $(OBJECTS) $(OBJECTS_M) $(OBJECTS_T)
	$(CXX) $(CPPFLAGS) -o $(TARGET) $(INCLUDES) $(OBJECTS_M) $(OBJECTS) $(LIBDIRS) $(LIBS) 
	$(CXX) $(CPPFLAGS) -o $(TEST_TARGET) $(INCLUDES) $(OBJECTS) $(OBJECTS_T) $(LIBDIRS) $(LIBS)

TARGET=pdpServer

TEST_TARGET=testApp

clean:
	rm *.o
	rm $(TARGET)
	rm $(TEST_TARGET)
