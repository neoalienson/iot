docker run --init -d --name="home-assistant" -e "TZ=Asia/Hong_Kong" \
-v "/home/neo/nexus/pc/hassio:/config" \
--restart=unless-stopped \
-p 8123:8123/tcp \
-p 5353:5353/udp \
-p 51827:51827 \
homeassistant/home-assistant:stable
#homeassistant/home-assistant:0.118.0.dev20201115
