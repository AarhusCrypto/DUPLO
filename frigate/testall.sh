cd tests

for f in *.wir
do

	echo "  ---"
	echo " ----- $f -----"
	echo "  ---"
	../frigate $f -nowarn
	echo "  "
	echo "  "
done

