FROM debian:buster

RUN apt update --fix-missing

RUN apt install -y \
    git curl build-essential cmake pkg-config

COPY .deploy/fake-sudo.sh /usr/local/bin/sudo
RUN chmod +x /usr/local/bin/sudo

WORKDIR /code

COPY install-gtest.sh ./
RUN bash ./install-gtest.sh

COPY install-gumbo.sh ./
RUN bash ./install-gumbo.sh

COPY install-prometheus-cpp.sh ./
RUN bash ./install-prometheus-cpp.sh

COPY setup.sh ./
RUN bash ./setup.sh

COPY ./ ./
RUN ls -lah
RUN cmake -DCMAKE_BUILD_TYPE=Release . && \
    cmake --build . -- -j4
#RUN ls -lah
RUN cp /code/adelantado /usr/local/bin/adelantado
RUN rm -rf /code

WORKDIR /app

ENTRYPOINT [ "adelantado" ]
