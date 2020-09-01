FROM thewtex/opengl:debian

RUN apt-get update && \
    apt-get install -fyqq --no-install-recommends \
        build-essential \
        cmake \
        qt4-default \
        libboost-all-dev \
        libqt4-opengl-dev

RUN mkdir /opt/diagrambuilder

WORKDIR /opt/diagrambuilder

RUN sudo ln -fs /usr/lib/libcurl.so.4 /usr/local/lib/

ADD . /opt/diagrambuilder

RUN cmake .

RUN make -j4

CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/supervisord.conf"]