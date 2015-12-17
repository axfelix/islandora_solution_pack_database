#!/bin/bash
# $1 = database dump
# $2 = mysql connect command

# Sanitize database name in case none is provided from export
DBNAME="$(cat /dev/urandom | tr -cd 'a-f0-9' | head -c 32)"
sed -i 's/-- Database: (.*?)/CREATE DATABASE \1;\nUSE DATABASE \1;/g' $1
if grep -q "CREATE DATABASE" $1; then :; else sed -i 's/-- MySQL dump/CREATE DATABASE `$(echo $DBNAME)`;\nUSE DATABASE `$(echo $DBNAME)`;\n-- MySQL dump/g' $1; fi

echo "CREATE USER '$user' IDENTIFIED BY '$password'" | $2
echo "REVOKE ALL PRIVILEGES ON *.* FROM '$user'@'%'; FLUSH PRIVILEGES" | $2
echo "GRANT SELECT ON $database TO '$user'@'%'; FLUSH PRIVILEGES" | $2
