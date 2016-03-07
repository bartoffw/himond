#!/usr/bin/env bash
# run graphite and statsd servers
docker run -d --name graphite --restart=always -p 80:80 -p 2003:2003 -p 8125:8125/udp hopsoft/graphite-statsd
sudo ldconfig