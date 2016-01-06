#!/bin/bash
# $1 = database dump
# $2 = mysql connect command
# $3 = generated username
# $4 = generated password

# Sanitize database name in case none is provided from export
# This handles incomplete exports from mysqldump and PMA
DBNAME="$(cat /dev/urandom | tr -cd 'a-f0-9' | head -c 32)"
if grep -qe "-- Database:" $1; then sed -i 's/-- Database: (.*?)/CREATE DATABASE \1;\nUSE DATABASE \1;/g' $1; fi
if grep -q "CREATE DATABASE" $1; then :; else sed -i "s/-- MySQL dump/CREATE DATABASE $DBNAME;\nUSE $DBNAME;\n-- MySQL dump/g" $1; fi

# Get the DB name now that there's definitely one in there
DBNAME="$(grep -oP '(?<=CREATE DATABASE )\S+(?=;)' $1)"

$2 < $1
echo "CREATE USER '$3' IDENTIFIED BY '$4'" | $2
echo "REVOKE ALL PRIVILEGES ON *.* FROM '$3'@'%'; FLUSH PRIVILEGES" | $2
echo "GRANT SELECT ON $DBNAME TO '$3'@'%'; FLUSH PRIVILEGES" | $2

echo $DBNAME
