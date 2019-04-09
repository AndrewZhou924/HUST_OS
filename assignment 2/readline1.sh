while read line
do 
	echo $line
done < txtToread.txt

awk '{print} END {printf "\ntotal lines : %d\n",NR}' txtToread.txt|tail -n1
