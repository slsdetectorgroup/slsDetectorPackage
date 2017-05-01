OBJPATH=bin/obj
EXAMPLEPATH=bin/example

all:
	g++ CondVar.cpp -pthread -c  -g -o $(OBJPATH)/CondVar.o
	g++ Mutex.cpp -pthread -c  -g -o $(OBJPATH)/Mutex.o
	#g++ Task.cpp -pthread -c -g  -o $(OBJPATH)/Task.o
	g++ ThreadPool.cpp -pthread -c -g  -o $(OBJPATH)/ThreadPool.o
	g++ Multi.cpp -pthread -c -g -o $(OBJPATH)/Multi.o
	#g++ $(OBJPATH)/CondVar.o $(OBJPATH)/Mutex.o $(OBJPATH)/Task.o $(OBJPATH)/ThreadPool.o threadpool_test.cpp Single.cpp Multi.cpp -pthread -I . -g -o $(EXAMPLEPATH)threadpool_test
	g++ $(OBJPATH)/CondVar.o $(OBJPATH)/Mutex.o  $(OBJPATH)/ThreadPool.o threadpool_test.cpp Single.cpp Multi.cpp -pthread -I . -g -o $(EXAMPLEPATH)threadpool_test

#all:
#	g++ threadpool.cpp -pthread -fpic -c -o bin/obj/threadpool.o
#	g++ -L./bin bin/obj/threadpool.o -pthread threadpool_test.cpp -o bin/example/threadpool_test

#threadpool:
#	g++ threadpool.cpp -pthread -fpic -c -o bin/obj/threadpool.o
#	g++ -shared -fPIC bin/obj/threadpool.o -o bin/lib/libthreadpool.so
#example:
#	g++ -L./bin/lib -lthreadpool threadpool_test.cpp -o threadpool_test
