read -p "please input a file name:" name
awk '{print}
	END {printf "total lines : %d\n",NR}' $name
