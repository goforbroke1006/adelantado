all: build

build:
	echo "TODO: implement"

keyword-ignore:
	sort keyword-ignore.txt | uniq -u | tee keyword-ignore.uniq.txt
	rm -f keyword-ignore.txt
	mv keyword-ignore.uniq.txt keyword-ignore.txt

links:
	sort links.txt | uniq -u | tee links.uniq.txt
	rm -f links.txt
	mv links.uniq.txt links.txt

local:
	docker-compose up -d