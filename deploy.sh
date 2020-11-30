#!/bin/bash


USER=$1
HOST=$2
DIR=$3

scp docker-compose.yml $USER@$HOST:$DIR
scp -r .deploy/ $USER@$HOST:$DIR
scp -r migrations/ $USER@$HOST:$DIR

scp domain-priority.txt $USER@$HOST:$DIR
scp keyword-ignore.txt $USER@$HOST:$DIR
scp keywords.txt $USER@$HOST:$DIR
scp links.txt $USER@$HOST:$DIR

DATE=$(date +"%Y-%m-%d-%H-%M")
ssh $USER@$HOST "cd ""${DIR}"" && export PGPASSWORD=adelantado && pg_dump -h localhost -p 25432 -U adelantado -d adelantado -a --rows-per-insert=1000 --on-conflict-do-nothing -f dump-""${DATE}"".sql || false"

ssh $USER@$HOST "cd ""${DIR}"" && docker-compose pull"
ssh $USER@$HOST "cd ""${DIR}"" && docker-compose up -d"
sleep 10
ssh $USER@$HOST "cd ""${DIR}"" && docker-compose ps"

ssh $USER@$HOST "cd ""${DIR}"" && export PGPASSWORD=adelantado && pg_dump -h localhost -p 25432 -U adelantado -d adelantado -a --rows-per-insert=1000 --on-conflict-do-nothing -f dump-""${DATE}"".sql || false"

curl -X POST http://192.168.0.211:9090/-/reload

#curl 'http://192.168.0.211:9090/api/v1/query?query=up&time=2015-07-01T20:10:51.781Z'