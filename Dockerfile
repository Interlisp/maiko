FROM ubuntu:focal
SHELL ["/bin/bash", "-c"]`

RUN apt-get update && apt-get install -y make clang libx11-dev

COPY --chown=nonroot:nonroot . /app/maiko
RUN rm -rf /app/maiko/linux*

WORKDIR /app/maiko/bin
RUN ./makeright x

RUN rm -rf /app/maiko/inc /app/maiko/include /app/maiko/src 