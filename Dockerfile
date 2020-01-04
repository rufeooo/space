FROM debian:stretch-slim
WORKDIR /app
COPY bin/space_server /app
EXPOSE 9845/udp 9845/udp
ENTRYPOINT [ "./space_server" ]
