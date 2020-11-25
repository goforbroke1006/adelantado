all: build

.PHONY: build
.ONESHELL:
build:
	mkdir -p build/
	cd build/
	rm -rf ./*
	cmake -DCMAKE_BUILD_TYPE=Release ..
	cmake --build . -- -j4
	#
	rm -rf CMakeFiles/
	rm -rf tests/
	rm -f cmake_install.cmake
	rm -f CMakeCache.txt
	rm -f Makefile

compress: compress/domain-priority.txt compress/keyword-ignore.txt compress/links.txt

compress/keyword-ignore.txt:
	sort keyword-ignore.txt | uniq -u | tee keyword-ignore.uniq.txt
	rm -f keyword-ignore.txt
	mv keyword-ignore.uniq.txt keyword-ignore.txt

compress/links.txt:
	sort links.txt | uniq -u | tee links.uniq.txt
	rm -f links.txt
	mv links.uniq.txt links.txt

compress/domain-priority.txt:
	sort domain-priority.txt | uniq -u | tee domain-priority.txt.uniq
	rm -f domain-priority.txt
	mv domain-priority.txt.uniq domain-priority.txt

image:
	docker build -t docker.io/goforbroke1006/adelantado:latest .
	docker login
	docker push docker.io/goforbroke1006/adelantado:latest

dev/start:
	docker-compose pull
	docker-compose up -d --remove-orphans

dev/stop:
	docker-compose stop

.ONESHELL:
dev/dump:
	mkdir -p dump/
	export PGPASSWORD=adelantado
	pg_dump \
		--host=localhost --port=25432 \
		--username=adelantado \
		--dbname=adelantado \
		--data-only --file=dump/links.sql