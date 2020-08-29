bison:
	bison -d parser.y
	
flex:
	flex lex.l
	
out:
	gcc *.c -o glr -lfl -pthread

win:
	lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc *.c -o glr -lfl -pthread

mac: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc *.c -o glr -ll -pthread

clean : 
	rm parser.tab.* lex.yy.c glr