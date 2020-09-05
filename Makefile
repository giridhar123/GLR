bison:
	bison -d ./src/parser.y
	mv parser.tab.c ./src
	mv parser.tab.h ./src
	
flex:
	flex ./src/lex.l
	mv lex.yy.c ./src
	
out:
	gcc ./src/*.c -o glr -lfl -pthread

win: lex.l parser.y
	bison -d ./src/parser.y
	mv parser.tab.c ./src
	mv parser.tab.h ./src
	flex ./src/lex.l
	mv lex.yy.c ./src
	gcc ./src/*.c -o glr -lfl -pthread

mac: ./src/lex.l ./src/parser.y
	bison -d ./src/parser.y
	mv parser.tab.c ./src
	mv parser.tab.h ./src
	flex ./src/lex.l
	mv lex.yy.c ./src
	gcc ./src/*.c -o glr -ll -pthread

clean : 
	rm ./src/parser.tab.* ./src/lex.yy.c ./glr