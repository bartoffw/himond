#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include "lib/system_monitor.h"
#include "lib/statsd_client.h"


static int running = 1;

void sigterm(int sig)
{
    //printf("himond - closing...");
    running = 0;
}

string getHostname() {
    string hostname;

    ifstream statsFile("/etc/hostname");
    if (getline(statsFile, hostname)) {
        return hostname;
    }
    return "general_hostname";
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf( "Usage:   %s <statsd_host> <statsd_port> [<eth_interface>] [<refresh_rate_seconds>]\n"
                "  where: eth_interface        - default: eth0\n"
                "         refresh_rate_seconds - default: 1\n\n"
                "Example: %s 127.0.0.1 8125\n",
                argv[0], argv[0]);
        exit(1);
    }

    string statsdHost = argv[1];
    int statsdPort = atoi(argv[2]);
    string ethInterface = "eth0";
    int refreshSeconds = 1;
    if (argc > 3) {
        ethInterface = argv[3];
        if (argc == 5) {
            refreshSeconds = atoi(argv[4]);
        }
    }

    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN); /* will save one syscall per sleep */
    signal(SIGTERM, sigterm);

    string ns = string("himond.") + getHostname().c_str() + ".";
    statsd::StatsdClient client(statsdHost, statsdPort, ns);
    SystemMonitor *sysmon = new SystemMonitor(true, true, true, ethInterface, refreshSeconds);

    printf("himond - running in background...\n");
    daemon(0,0);

    int secondsPassed = 0;
    unsigned long
        uptime,
        memTotal, memUsed, memFree,
        virtMemTotal, virtMemUsed, virtMemFree,
        diskTotal, diskUsed, diskFree;
    unsigned long long
        diskReadRate, diskWriteRate,
        netRecvRate, netSendRate;
    int cpuCount;
    float cpuUsage;

    while (running) {
        sleep(1);
        secondsPassed++;

        // gathering stats
        uptime   = sysmon->getSystemUptime(); // secs
        // MB
        memTotal = sysmon->getMemoryTotal();
        memUsed  = sysmon->getMemoryUsed();
        memFree  = sysmon->getMemoryFree();
        // MB
        virtMemTotal = sysmon->getVirtualMemoryTotal();
        virtMemUsed  = sysmon->getVirtualMemoryUsed();
        virtMemFree  = sysmon->getVirtualMemoryFree();
        // MB
        diskTotal = sysmon->getDiskTotal();
        diskUsed  = sysmon->getDiskUsed();
        diskFree  = sysmon->getDiskFree();
        // kbps
        diskReadRate  = sysmon->getDiskReadRate();
        diskWriteRate = sysmon->getDiskWriteRate();
        // kbps
        netRecvRate = sysmon->getRxRate();
        netSendRate = sysmon->getTxRate();

        cpuCount = sysmon->getProcessorCount();
        cpuUsage = sysmon->getProcessorUsage();

        if (refreshSeconds >= secondsPassed) {
            // send the stats
            client.count("system.uptime", uptime);

            client.gauge("memory.total", memTotal);
            client.gauge("memory.used", memUsed);
            client.gauge("memory.free", memFree);

            client.gauge("memory.virtual.total", virtMemTotal);
            client.gauge("memory.virtual.used", virtMemUsed);
            client.gauge("memory.virtual.free", virtMemFree);

            client.gauge("disk.total", diskTotal);
            client.gauge("disk.used", diskUsed);
            client.gauge("disk.free", diskFree);

            client.count("disk.rate.read", diskReadRate);
            client.count("disk.rate.write", diskWriteRate);

            client.count("net.rate.recv", netRecvRate);
            client.count("net.rate.send", netSendRate);

            client.gauge("cpu.count", cpuCount);
            client.gauge("cpu.usage", cpuUsage);

            secondsPassed = 0;
        }
    }

    return 0;
}
