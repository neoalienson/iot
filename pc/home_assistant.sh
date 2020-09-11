docker run --init -d --name="home-assistant" -e "TZ=Asia/Hong_Kong" \
-v "/home/neo/nexus/pc/hassio:/config" \
--restart=unless-stopped \
-p 8123:8123 homeassistant/home-assistant:stable
