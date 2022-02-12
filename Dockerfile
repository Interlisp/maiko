#*******************************************************************************
#
#  Dockerfile to build Maiko (Stage 1) and create a Docker image and push it
#  to DockerHub (stage 2).
#
#  Copyright 2022 by Interlisp.org
#
# ******************************************************************************

#
#  Build Maiko Stage
#
FROM ubuntu:focal AS builder
SHELL ["/bin/bash", "-c"]
USER root:root
# Install build tools
RUN apt-get update && apt-get install -y make clang libx11-dev gcc x11vnc xvfb
# Copy over / clean maiko repo
COPY . /app/maiko
RUN rm -rf /app/maiko/linux*
# Build maiko
WORKDIR /app/maiko/bin
RUN ./makeright x
RUN if [ "$(./osversion)" = "linux" ] && [ "$(./machinetype)" = "x86_64" ]; then ./makeright init; fi
# Prep for Install Stage
RUN mv ../$(./osversion).$(./machinetype) ../TRANSFER
#
#  Install Maiko Stage
#
FROM ubuntu:focal
ARG BUILD_DATE="not_available"
ARG RELEASE_TAG="not_available"
LABEL name="Maiko"
LABEL description="Virtual machine for Interlisp Medley"
LABEL url="https://github.com/Interlisp/maiko"
LABEL build-time=$BUILD_DATE
LABEL release_tag=$RELEASE_TAG
ENV MAIKO_RELEASE=$RELEASE_TAG
ENV MAIKO_BUILD_DATE=$BUILD_DATE
ARG BUILD_LOCATION=/app/maiko
ARG INSTALL_LOCATION=/usr/local/interlisp/maiko
#
SHELL ["/bin/bash", "-c"]
USER root:root
# Copy release files into /usr/local/directories
COPY --from=builder ${BUILD_LOCATION}/bin/osversion    ${INSTALL_LOCATION}/bin/
COPY --from=builder ${BUILD_LOCATION}/bin/machinetype  ${INSTALL_LOCATION}/bin/
COPY --from=builder ${BUILD_LOCATION}/bin/config.guess ${INSTALL_LOCATION}/bin/
COPY --from=builder ${BUILD_LOCATION}/bin/config.sub   ${INSTALL_LOCATION}/bin/
COPY --from=builder ${BUILD_LOCATION}/TRANSFER/lde*    ${INSTALL_LOCATION}/TRANSFER/
RUN  cd ${INSTALL_LOCATION} && mv TRANSFER "$(cd bin && ./osversion).$(cd bin/ && ./machinetype)"
# Some niceties
USER root
WORKDIR /root
ENTRYPOINT /bin/bash
