for FILE in *.test; 
do 
    echo $FILE;
    head -n 12 $FILE > U_$FILE;
    mv U_$FILE $FILE 
done
