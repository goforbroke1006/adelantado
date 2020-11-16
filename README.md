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
bash ./setup.linux.sh

docker-compose up -d
sleep 10 # wait for docker-compose warm-up

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
