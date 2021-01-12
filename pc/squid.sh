docker run --privileged --tmpfs /var/log/squid -p 3128:3128 -v $(pwd)/squid:/etc/squid -it squid busybox sh
