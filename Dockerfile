FROM ubuntu:20.04

#下载二进制包
RUN wget https://cmake.org/files/v3.12/cmake-3.12.2-Linux-x86_64.tar.gz \
    && tar -zxvf cmake-3.12.2-Linux-x86_64.tar.gz \
    && mv cmake-3.12.2-Linux-x86_64 cmake-3.12.2 \
    && ln -sf /cmake-3.12.2/bin/* /usr/bin
#cmake源码编译安装
COPY cmake-3.21.1.zip /home/android-toolchain
RUN cd /home/android-toolchain \
    && unzip cmake-3.21.1.zip \
    && cd cmake-3.21.1 \
    && ./bootstrap \
    && make -j$(nproc) \
    && sudo make install \
    && cmake --version \
    && cd ..
#拷贝
COPY . ./LightOi

