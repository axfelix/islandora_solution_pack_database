#!/bin/bash

sudo groupadd docker
sudo gpasswd -a $USER docker
sudo service docker restart
newgrp docker
docker build -t tutum/lamp github.com/tutumcloud/lamp.git#master
docker run -d -p 8008:80 -p 33060:3306 tutum/lamp
docker exec -i $(docker ps | grep 33060 | sed -e "s/\s.*//g") mkdir /plugins
#docker cp includes/adminer-4.2.1-mysql.php $(docker ps | grep 33060 | sed -e "s/\s.*//g"):/app/adminer.php
#docker cp includes/plugins/plugin.php $(docker ps | grep 33060 | sed -e "s/\s.*//g"):/app/plugins/plugin.php
#docker cp includes/plugins/frames.php $(docker ps | grep 33060 | sed -e "s/\s.*//g"):/app/plugins/frames.php
workaround for the above cp commands on docker <1.8
docker exec -i $(docker ps | grep 33060 | sed -e "s/\s.*//g") sh -c 'cat > /app/adminer.php' < includes/adminer-4.2.1-mysql.php
docker exec -i $(docker ps | grep 33060 | sed -e "s/\s.*//g") sh -c 'cat > /app/plugins/plugin.php' < includes/plugins/plugin.php
docker exec -i $(docker ps | grep 33060 | sed -e "s/\s.*//g") sh -c 'cat > /app/plugins/frames.php' < includes/plugins/frames.php
