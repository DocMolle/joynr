FROM alpine:3.8

RUN apk add --no-cache bash curl grep

COPY start-me-up.sh /start-me-up.sh

ENTRYPOINT ["bash","/start-me-up.sh"]
