FROM debian:stretch-slim
WORKDIR /app
COPY bin/space_server /app
EXPOSE 9485 9485/udp
ENTRYPOINT [ "./space_server" ]
