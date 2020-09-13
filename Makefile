bison:
	bison -d ./src/parser.y
	mv parser.tab.c ./src
	mv parser.tab.h ./src
	
flex:
	flex ./src/lex.l
	mv lex.yy.c ./src
	
out:
	gcc ./src/*.c -o glr -lfl -pthread

win: ./src/lex.l ./src/parser.y
	bison -d ./src/parser.y
	mv parser.tab.c ./src
	mv parser.tab.h ./src
	flex ./src/lex.l
	mv lex.yy.c ./src
	clang ./src/*.c -o glr -lfl -pthread -lm -Wall

mac: ./src/lex.l ./src/parser.y
	bison -d ./src/parser.y
	mv parser.tab.c ./src
	mv parser.tab.h ./src
	flex ./src/lex.l
	mv lex.yy.c ./src
	gcc ./src/*.c -o glr -ll -pthread -lm -Wall

clean : 
	rm ./src/parser.tab.* ./src/lex.yy.c ./glr