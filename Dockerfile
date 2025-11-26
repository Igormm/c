# Dockerfile для сборки программы gradus
# Мультистадийная сборка для минимизации размера образа

# Стадия сборки
FROM ubuntu:22.04 AS builder

# Установка зависимостей для сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    clang \
    cmake \
    make \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Установка рабочей директории
WORKDIR /build

# Копирование исходного кода
COPY gradus_enhanced.c CMakeLists.txt Makefile build.sh ./
COPY README_ENHANCED.md ENHANCED_FEATURES.md ./

# Сборка программы
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install DESTDIR=/tmp/install

# Стадия выполнения (минимальный образ)
FROM ubuntu:22.04

# Установка только необходимых библиотек
RUN apt-get update && apt-get install -y \
    libc6 \
    libm6 \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

# Копирование собранной программы
COPY --from=builder /tmp/install/usr/local/bin/gradus /usr/local/bin/gradus

# Создание непривилегированного пользователя
RUN useradd -m -s /bin/bash gradus

# Переключение на непривилегированного пользователя
USER gradus
WORKDIR /home/gradus

# Тестирование программы
RUN gradus -T > /dev/null && \
    gradus -G > /dev/null && \
    echo "0 20 100" | gradus -a -s C -t F > /dev/null && \
    echo "Программа работает корректно"

# Установка entrypoint
ENTRYPOINT ["gradus"]
CMD ["--help"]