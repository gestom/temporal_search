end=$((16*7*24*3600))
interval=60
make CXXFLAGS=-DTEMPORAL=CFrelement fremen
#../bin/fremen establish ../etc/weeks_4_order_0.fre presence.txt 2419200 $end $interval ../../search2/etc/map.txt  #static
../bin/fremen establish aruba/weeks_4_order_0.fre presence.txt 2419200 $end $interval ../../search2/etc/map.txt >static_0.res  #static
#../bin/fremen establish omniscient presence.txt 2419200 $end $interval ../../search2/etc/map.txt >omniscient_0.resi	#this one knows the person position
for i in $(seq 1 5);
do
../bin/fremen establish weeks_4_order_$i.fre presence.txt 2419200 $end $interval ../../search/etc/map.txt >fremen_$i.res
done

make CXXFLAGS=-DTEMPORAL=CFregement gremen
for i in $(seq 1 5);
do
../bin/gremen establish weeks_4_order_$i.gmr presence.txt 2419200 $end $interval ../../search/etc/map.txt >gremen_$i.res
done
