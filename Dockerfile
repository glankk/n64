FROM alpine:3.9 as build-stage
# Install build dependencies
RUN apk --no-cache \
    add --virtual build-dependencies \
    wget \
    tar \
    make \
    diffutils \
    texinfo \
    gcc \
    g++ \
    lua5.3-dev \
    jansson-dev \
    libusb-dev
# Prepare workspace
WORKDIR /usr/local/src/n64
COPY . .
RUN LDFLAGS="-L/usr/lib/lua5.3" ./configure \
    --prefix=/opt/n64 \
    --enable-vc
RUN make install-toolchain -j`cat /proc/cpuinfo | grep processor | wc -l`
# Compile and install
RUN make \
    && make install \
    && make install-sys

# Final image
FROM alpine:3.9
RUN apk --no-cache add \
    make \
    lua5.3 \
    jansson \
    libusb
COPY --from=build-stage /opt/n64 /usr/local
CMD ["/bin/sh"]
