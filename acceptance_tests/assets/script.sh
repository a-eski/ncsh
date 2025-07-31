STR=hello
echo $STR
STR2=there
echo $STR $STR2
STR3='hello, world' && echo $STR3

ls
ls | wc -c
ls | sort | wc -c
ls > t.txt
ls | sort > t.txt
sort < t.txt
ls >> t.txt
rm t.txt

# comment is ignored

# Currently supported if statements
if [ true ]; then echo 'hello'; fi # echos hello
if [ false ]; then echo 'hello'; fi
if [ true ]; then echo 'hello'; else echo 'hi'; fi # echos hello
if [ false ]; then echo 'hello'; else echo 'hi'; fi # echo hi

if [ true && true ]; then echo 'hello'; fi # echos hello
if [ true && false ]; then echo 'hello'; fi
if [ false && true ]; then echo 'hello'; fi
if [ false && false ]; then echo 'hello'; fi

if [ true || true ]; then echo 'hello'; fi # echos hello
if [ true || false ]; then echo 'hello'; fi # echos hello
if [ false || true ]; then echo 'hello'; fi # echos hello
if [ false || false ]; then echo 'hello'; fi

if [ true && false ]; then echo 'hello'; else echo 'hi'; fi # echos hi
if [ true && true ]; then echo 'hello'; else echo 'hi'; fi # echos hello
if [ false && false ]; then echo 'hello'; else echo 'hi'; fi
if [ false && true ]; then echo 'hello'; else echo 'hi'; fi # echos hi

if [ true || false ]; then echo 'hello'; else echo 'hi'; fi # echos hello
if [ true || true ]; then echo 'hello'; else echo 'hi'; fi # echos hello
if [ false || false ]; then echo 'hello'; else echo 'hi'; fi # echos hi
if [ false || true ]; then echo 'hello'; else echo 'hi'; fi # echos hello

if [ 1 -eq 1 ]; then echo 'hello'; fi # echos hello
if [ 2 -eq 1 ]; then echo 'hello'; fi
if [ 1 -eq 1 ]; then echo 'hello'; else echo 'hi'; fi # echos hello
if [ 2 -eq 1 ]; then echo 'hello'; else echo 'hi'; fi # echos hi

if [ 2 -gt 1 ]; then echo 'hello'; fi # echos hello
if [ 1 -gt 2 ]; then echo 'hello'; fi
if [ 2 -gt 1 ]; then echo 'hello'; else echo 'hi'; fi # echos hello
if [ 1 -gt 2 ]; then echo 'hello'; else echo 'hi'; fi # echos hi

if [ 1 -lt 2 ]; then echo 'hello'; fi # echos hello
if [ 1 -lt 1 ]; then echo 'hello'; fi
if [ 1 -lt 2 ]; then echo 'hello'; else echo 'hi'; fi # echos hello
if [ 2 -lt 1 ]; then echo 'hello'; else echo 'hi'; fi # echos hi
