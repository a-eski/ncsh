FROM ubuntu:latest

RUN apt update && \
  apt install gcc make ruby tmux -y && \
  gem install ttytest2

ENV RUBYOPT="-KU -E utf-8:utf-8"
ENV USER="alex"

WORKDIR /ncsh
COPY . /ncsh
ENTRYPOINT [ "/bin/bash", "-l", "-c" ]

CMD [ "./acceptance_tests.sh" ]
