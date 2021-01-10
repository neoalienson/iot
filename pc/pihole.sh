# --shm-size="5g" is added to prevent it from keeping restart
# enable the firewall rules on Windows
docker run -d --name pihole -e ServerIP=192.16.18.200 \
-e WEBPASSWORD=password -e TZ=Asia/Hong_Kong \
-e DNS1=192.168.18.1 -e DNS2=1.1.1.1 -e DNS3=1.0.0.1 \
-p 9080:80 -p 53:53/tcp -p 53:53/udp -p 9443:443 \
-v "/home/neo/nexus/pc/pihole/etc/pihole:/etc/pihole" \
--restart=unless-stopped --shm-size="5g" pihole/pihole:latest
