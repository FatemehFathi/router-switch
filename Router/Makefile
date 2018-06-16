all: router.o ethernet.o distVectRow.o
	g++ router.o ethernet.o distVectRow.o
router.o: router.cpp
	g++ -c router.cpp
ethernet.o: ethernet.cpp
	g++ -c ethernet.cpp
distVectRow.o: distVectRow.cpp
	g++ -c distVectRow.cpp