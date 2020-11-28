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

ssh $USER@$HOST "cd ""${DIR}"" && docker-compose pull"
ssh $USER@$HOST "cd ""${DIR}"" && docker-compose up -d"
sleep 10
ssh $USER@$HOST "cd ""${DIR}"" && docker-compose ps"
