all: build

build:
	echo "TODO: implement"

links:
	sort links.txt | uniq -u | tee links.uniq.txt
	rm -f links.txt
	mv links.uniq.txt links.txt

local:
	docker-compose up -d