# adelantado

Search engine for fun.

### Env and deps

* g++ (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0
* libcurl
* [nlohmann/json](https://github.com/nlohmann/json)
* [GTest/googletest](https://github.com/google/googletest)
* [gumbo](https://github.com/google/gumbo-parser)

### How to use

```bash
sudo apt install -y git curl gcc make cmake

git clone git@github.com:goforbroke1006/adelantado.git
cd ./adelantado/

bash ./install-gtest.sh
bash ./install-gumbo.sh
bash ./install-prometheus-cpp.sh
bash ./setup.linux.sh

git submodule init
git submodule update --remote

docker-compose up -d
sleep 10 # wait for docker-compose warm-up

x-www-browser http://localhost:9090/targets
x-www-browser http://localhost:8080/metrics
x-www-browser http://localhost:3000/ # admin admin

```

or

```bash
make build
cd build

./adelantado

```

### Build files

* **adelantado** - runnable binary
* **adelantado.cfg** - config (DB, link queue, etc)
* **domain-priority.txt** - links with this domains will scanned first
* **keyword-ignore.txt** - this words will not used as keywords (pronouns, etc)
* **keywords.txt** - in progress...
* **links.txt** - initial links set, allow add new elements, solve cold start problem; load on every program start


### Useful links 

* https://prometheus.io/docs/introduction/first_steps/

### Modules

* git submodule add -- https://github.com/goforbroke1006/goxx-std.git   modules/goforbroke1006/goxx-std
* git submodule add -- https://github.com/goforbroke1006/cpp-logger.git modules/goforbroke1006/cpp-logger
* git submodule add -- https://github.com/nlohmann/json.git             modules/nlohmann/json