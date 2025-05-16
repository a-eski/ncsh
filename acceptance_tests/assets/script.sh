# /bin/ncsh

STR=hello
echo $STR

STR2=there
echo $STR $STR2

ls | wc -c

# ignored

ls | wc -c && ls | wc -c

echo 'finished!'
