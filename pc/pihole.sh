docker run -d --name pihole -e ServerIP=192.16.18.3 \
-e WEBPASSWORD=password -e TZ=Asia/Hong_Kong -e DNS1=192.168.18.1 -e DNS2=1.1.1.1 -e DNS3=1.0.0.1 -p 80:80 -p 53:53/tcp -p 53:53/udp -p 443:443 \
--restart=unless-stopped pihole/pihole:latest
