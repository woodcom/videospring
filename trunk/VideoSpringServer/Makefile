videosprings: videosprings.o ../common/videospringcommon.o
	g++ -g -Wall ../common/videospringcommon.o videosprings.o -o videosprings

videosprings.o: ./VideoSpringServer/main.cpp
	g++ -g -c ./VideoSpringServer/main.cpp -o videosprings.o

../common/videospringcommon.o: ../common/VideoSpringCommon.cpp ../common/VideoSpringCommon.h
	g++ -g -c ../common/VideoSpringCommon.cpp -o ../common/videospringcommon.o
