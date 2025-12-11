aleph: lexxer.l
	flex lexxer.l
	bison parser.y -d -Wcounterexamples
	gcc parser.tab.c lex.yy.c aleph.c tpt/tData.c tpt/Auxiliar.c -o $@ -lm