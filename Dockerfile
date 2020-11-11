FROM debian:stretch

RUN apt update

RUN apt install -y git

RUN apt install -y libcurl4-openssl-dev

RUN apt-get install -y libtool                                                  && \
    git clone -b v1.0.0 --depth 1 https://github.com/google/gumbo-parser.git    && \
    cd gumbo-parser && \
    ./autogen.sh    && \
    ./configure     && \
    make            && \
    make install    && \
    ldconfig        && \
    cd ..           && \
    rm -rf ./gumbo-parser

RUN apt install -y libpq-dev

RUN apt install -y libfmt-dev

COPY ./adelantado /usr/local/bin/adelantado
RUN chmod +x /usr/local/bin/adelantado

ENTRYPOINT [ "adelantado" ]
