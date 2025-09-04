if [ 1 -eq 5 ]; then echo hello; elif [ 1 -eq 5 ]; then echo hi; elif [ false && true ]; then echo c; elif [ 2 -gt 1 ]; then echo hey; fi

if [ 1 -eq 5 ]; then echo hello; elif [ 1 -eq 5 ]; then echo hi; elif [ false && true ]; then echo hiya; elif [ true && true && false ]; then echo hey; fi

if [ false && true ]; then echo hello; elif [ false && true ]; then echo hi; elif [ 1 -lt 1 ]; then echo hey; elif [ false && true ]; then echo hiya; else echo hallo fi
