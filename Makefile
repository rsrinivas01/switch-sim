

sim:
	gcc -o swSim swLib.c swMain.c -lpthread -lrt
sim-dbg:
	gcc -o swSim -g swLib.c swMain.c -lpthread -lrt
sim-ut:
	gcc -o swSimUT swLib.c swSimUT.c -lpthread -lrt
pkt-gen:
	gcc -o swPktGen swLib.c swPktGen.c -lpthread -lrt
clean-sim:
	rm -rf swSim

all:
	make sim
	make sim-ut
	make pkt-gen
clean:
	rm -rf swSim swSimUT swPktGen
	
