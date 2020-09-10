docker run --init -d --name="home-assistant" -e "TZ=Asia/Hong_Kong" -v "/home/neo/nexus/pc:/config" -p 8123:8123 homeassistant/home-assistant:stable
