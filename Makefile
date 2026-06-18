assignment109: queue.o worldgen.o mapgen.o pathfind.o heap.o turnpq.o trainer.o movement.o csv_parser.o pokemon.o battle.o assignment109.o
	g++ -Wall -Werror -g queue.o worldgen.o mapgen.o pathfind.o heap.o turnpq.o trainer.o movement.o csv_parser.o pokemon.o battle.o assignment109.o -o assignment109 -lncurses -lm

queue.o: queue.cpp queue.h
	g++ -Wall -Werror -g -c queue.cpp

worldgen.o: worldgen.cpp worldgen.h
	g++ -Wall -Werror -g -c worldgen.cpp

mapgen.o: mapgen.cpp mapgen.h
	g++ -Wall -Werror -g -c mapgen.cpp

pathfind.o: pathfind.cpp pathfind.h
	g++ -Wall -Werror -g -c pathfind.cpp

heap.o: heap.cpp heap.h
	g++ -Wall -Werror -g -c heap.cpp

turnpq.o: turnpq.cpp turnpq.h
	g++ -Wall -Werror -g -c turnpq.cpp

trainer.o: trainer.cpp trainer.h
	g++ -Wall -Werror -g -c trainer.cpp

movement.o: movement.cpp movement.h
	g++ -Wall -Werror -g -c movement.cpp

csv_parser.o: csv_parser.cpp csv_parser.h
	g++ -Wall -Werror -g -c csv_parser.cpp

pokemon.o: pokemon.cpp pokemon.h
	g++ -Wall -Werror -g -c pokemon.cpp

battle.o: battle.cpp battle.h
	g++ -Wall -Werror -g -c battle.cpp
	
assignment109.o: assignment109.cpp queue.h worldgen.h mapgen.h pathfind.h heap.h turnpq.h trainer.h movement.h csv_parser.h pokemon.h battle.h
	g++ -Wall -Werror -g -c assignment109.cpp 

clean:
	rm -f *.o assignment109 *~