end=$((16*7*24*3600))
start=$((4*7*24*3600))
interval=60

make CXXFLAGS=-DTEMPORAL=CFrelement fremen

echo Generating results for static models
../bin/fremen establish ../etc/weeks_4_order_0.fre ../etc/presence.txt $start $end $interval ../../search2/etc/aruba.txt >../results/static_0.res  	#static model
#echo Generating results for omniscient models
#../bin/fremen establish omniscient 		   ../etc/presence.txt $start $end $interval ../../search2/etc/aruba.txt >../results/omniscient_0.res	#best possible model - knows person location from ground truth

for i in $(seq 1 3);
do
echo Generating results for FreMEn model order $i 
../bin/fremen establish ../etc/weeks_4_order_$i.fre ../etc/presence.txt $start $end $interval ../../search2/etc/aruba.txt >../results/fremen_$i.res  	#FreMEn model
done

#GMM do not work at the present
#make CXXFLAGS=-DTEMPORAL=CFregement gremen
#for i in $(seq 1 3);
#do
#echo Generating results for Gaussian Mixture Model order $i 
#../bin/gremen establish ../etc/weeks_4_order_$i.gmr ../etc/presence.txt $start $end $interval ../../search2/etc/aruba.txt >../results/gremen_$i.res  	#GMM model
#done
