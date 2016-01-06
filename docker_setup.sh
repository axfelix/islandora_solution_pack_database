#!/bin/bash

sudo groupadd docker
sudo gpasswd -a $USER docker
sudo service docker restart
newgrp docker
docker build -t tutum/lamp github.com/tutumcloud/lamp
docker run -d -p 8008:80 -p 33060:3306 tutum/lamp
docker cp includes/adminer-4.2.1-mysql.php $(docker ps | grep 33060 | sed -e "s/\s.*//g"):/adminer-4.2.1-mysql.php
#workaround for the above command on docker <1.8
#docker exec -i $(docker ps | grep 33060 | sed -e "s/\s.*//g") sh -c 'cat > /adminer-4.2.1-mysql.php' < includes/adminer-4.2.1-mysql.php
