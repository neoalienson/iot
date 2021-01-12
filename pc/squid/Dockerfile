FROM alpine:3.12.3

RUN apk add squid 
RUN echo "*/10 * * * * squid -k rotate" > cron.file
RUN crontab cron.file

EXPOSE 3128

CMD [ "/bin/busybox", "sh" ]

