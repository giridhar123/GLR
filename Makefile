bison:
	bison -d parser.y
	
flex:
	flex lex.l
	
out:
	gcc *.c -o glr -lfl -pthread

all : lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc *.c -o glr -lfl -pthread
clean : 
	rm program.tab.* lex.yy.c glr.out 