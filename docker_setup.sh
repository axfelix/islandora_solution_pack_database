#!/bin/bash

sudo groupadd docker
sudo gpasswd -a $USER docker
sudo service docker restart
newgrp docker
docker build -t tutum/lamp github.com/tutumcloud/lamp
docker run -d -p 8008:80 -p 33060:3306 tutum/lamp