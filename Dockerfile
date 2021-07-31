FROM ubuntu:focal
ARG BUILD_DATE
LABEL name="Maiko"
LABEL description="Virtual machine for Interlisp Medley"
LABEL url="https://github.com/Interlisp/maiko"
LABEL build-time=$BUILD_DATE

ARG TARGETPLATFORM

RUN apt-get update && apt-get install -y make clang libx11-dev gcc x11vnc xvfb

COPY --chown=nonroot:nonroot . /app/maiko
RUN rm -rf /app/maiko/linux*

WORKDIR /app/maiko/bin
RUN ./makeright x

RUN rm -rf /app/maiko/inc /app/maiko/include /app/maiko/src
