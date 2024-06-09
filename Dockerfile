# syntax=docker/dockerfile:1

ARG NDN_CXX_VERSION=latest

FROM scratch AS psync
ARG PSYNC_VERSION=master
ADD https://github.com/named-data/PSync.git#${PSYNC_VERSION} /

FROM ghcr.io/named-data/ndn-cxx-build:${NDN_CXX_VERSION} AS build

RUN apt-get install -Uy --no-install-recommends \
        libboost-iostreams-dev \
    && apt-get distclean

ARG JOBS
ARG SOURCE_DATE_EPOCH
RUN --mount=from=psync,rw,target=/psync <<EOF
    set -eux
    cd /psync
    ./waf configure \
        --prefix=/usr \
        --libdir=/usr/lib \
        --sysconfdir=/etc \
        --localstatedir=/var \
        --sharedstatedir=/var
    ./waf build
    ./waf install
EOF
RUN --mount=rw,target=/src <<EOF
    set -eux
    cd /src
    ./waf configure \
        --prefix=/usr \
        --libdir=/usr/lib \
        --sysconfdir=/etc \
        --localstatedir=/var \
        --sharedstatedir=/var \
        --with-psync
    ./waf build
    ./waf install
    mkdir -p /deps/debian
    touch /deps/debian/control
    cd /deps
    dpkg-shlibdeps --ignore-missing-info /usr/lib/libPSync.so.* /usr/bin/nlsr /usr/bin/nlsrc -O \
        | sed -n 's|^shlibs:Depends=||p' | sed 's| ([^)]*),\?||g' > nlsr
EOF


FROM ghcr.io/named-data/ndn-cxx-runtime:${NDN_CXX_VERSION} AS nlsr

COPY --link --from=build /usr/lib/libPSync.so.* /usr/lib/
COPY --link --from=build /usr/bin/nlsr /usr/bin/
COPY --link --from=build /usr/bin/nlsrc /usr/bin/
COPY --link --from=build /etc/ndn/nlsr.conf.sample /config/nlsr.conf

RUN --mount=from=build,source=/deps,target=/deps \
    apt-get install -Uy --no-install-recommends \
        $(cat /deps/nlsr) \
    && apt-get distclean

ENV HOME=/config
VOLUME /config
VOLUME /var/lib/nlsr
VOLUME /run/nfd

ENTRYPOINT ["/usr/bin/nlsr"]
CMD ["-f", "/config/nlsr.conf"]
