# himond

Linux system metrics collector for statsd written in C++.


## Metrics

Metrics gathered by the application:
* memory (real + virtual) total/usage
* disk space total/usage
* processor usage
* processors count
* network in/out
* disk in/out
* uptime

In statsd they are prefixed with `himond.hostname` string (or `himond.general_hostname`
if the hostname couldn't be retrieved), e.g.:
```
himond.production.system.uptime:124|c
himond.production.cpu.usage:77|g
```

### Uptime

System uptime in seconds.
* `system.uptime` count

### Memory

Memory statistics in megabytes.
* `memory.total` gauge
* `memory.used` gauge
* `memory.free` gauge
* `memory.virtual.total` gauge
* `memory.virtual.used` gauge
* `memory.virtual.free` gauge

### Disk

Disk usage statistics in megabytes, transfer rates in kilobytes per second.
* `disk.total` gauge
* `disk.used` gauge
* `disk.free` gauge
* `disk.rate.read` count
* `disk.rate.write` count

### Net

Net IO interface transfer rates in kilobytes per second.
* `net.rate.recv` count
* `net.rate.send` count

### CPU

Usage in percent.
* `cpu.count` count
* `cpu.usage` count


## Usage

Himond gathers statistics from the local host and sends them to the selected statsd server.
It's necessary to provide the server details where the statistics will be sent (host and port).

You can also customize ethernet interface which will be monitored (default `eth0`) and the
statistics refresh rate in seconds.

```
$ ./himond -h

USAGE:

   ./himond  [--prefix <string>] [--hostname <string>] -s <string> [-p
             <int>] [-i <string>] [-r <int>] [--] [--version] [-h]


Where:

   --prefix <string>
     Prefix (default: himond)

   --hostname <string>
     Hostname

   -s <string>,  --server <string>
     (required)  StatsD address

   -p <int>,  --port <int>
     StatsD port (default: 8125)

   -i <string>,  --interface <string>
     Network interface (default: eth0)

   -r <int>,  --refresh <int>
     Refresh seconds rate (default: 1)

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.

   himond - linux system metrics collector for statsd
```

## Building

Run `make` from within the `src` directory.
Run `make clean` to delete all files generated after a build.


## Testing

A `Vagrantfile` file is attached that allows to run a test linux environment.
It also contains statsd server as a docker container so you can test himond
with statsd server just by running `himond 127.0.0.1 8125`.

Run `test_client` application to see the metrics on the screen.
Run `test_client -h` for usage.

## Troubleshooting

If Vagrant can't mount folders, try running vagrant plugin install vagrant-vbguest
as the problem may be caused by updated linux kernel in the guest system.


## Things to improve

* show processor usage separately for each core


# License

MIT
