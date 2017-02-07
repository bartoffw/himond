#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <tclap/CmdLine.h>
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
    string statsdHost;
    int statsdPort;
    string ethInterface;
    int refreshSeconds;
    string ns = "";
    try
    {
        TCLAP::CmdLine cmd("himond - linux system metrics collector for statsd", ' ', "0.1");
        TCLAP::ValueArg<string> hostnameArg("", "hostname", "Hostname", false, getHostname(), "string");
        TCLAP::ValueArg<string> prefixArg("", "prefix", "Prefix name for all metrics (default: himond)", false, "himond", "string");
        TCLAP::ValueArg<string> serverArg("s", "server", "StatsD address", true, "", "string");
        TCLAP::ValueArg<int> portArg("p", "port", "StatsD port (default: 8125)", false, 8125, "int");
        TCLAP::ValueArg<string> interfaceArg("i", "interface", "Network interface (default: eth0)", false, "eth0", "string");
        TCLAP::ValueArg<int> refreshSecondsArg("r", "refresh", "Refresh seconds rate (default: 1)", false, 1, "int");

        cmd.add(refreshSecondsArg);
        cmd.add(interfaceArg);
        cmd.add(portArg);
        cmd.add(serverArg);
        cmd.add(hostnameArg);
        cmd.add(prefixArg);

        cmd.parse(argc, argv);

        string prefix = prefixArg.getValue();
        if (prefix.size() > 0)
            ns = prefix + "." + hostnameArg.getValue() + ".";
        else
            ns = hostnameArg.getValue() + ".";

        statsdHost = serverArg.getValue();
        statsdPort = portArg.getValue();
        ethInterface = interfaceArg.getValue();
        refreshSeconds = refreshSecondsArg.getValue();
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        printf("error: %s for arg %s\n", e.error().c_str(), e.argId().c_str());
        exit(1);
    }

    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN); /* will save one syscall per sleep */
    signal(SIGTERM, sigterm);

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

        if (secondsPassed >= refreshSeconds) {
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
