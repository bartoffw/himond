#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "lib/system_monitor.h"

int main(int argc, char *argv[])
{
    string ethInterface = "eth0";
    if (argc > 1) {
        string parameter = argv[1];

        if ("--help" == parameter || "-h" == parameter) {
            printf( "Usage:   %s [<eth_interface>]\n"
                    "  where: eth_interface        - default: eth0\n"
                    "Example: %s eth1\n",
                    argv[0], argv[0]);
            exit(1);
        }
        ethInterface = parameter;
    }

    SystemMonitor *sysmon = new SystemMonitor(true, true, true, ethInterface, 1);

    system("clear");
    printf("Gathering system metrics...\n\n");

    while (true) {
        sleep(1);

        system("clear");

        printf("System uptime: %ld seconds\n\n", sysmon->getSystemUptime());

        printf("Memory total: %ld MB\n", sysmon->getMemoryTotal());
        printf("Memory used: %ld MB\n", sysmon->getMemoryUsed());
        printf("Memory free: %ld MB\n\n", sysmon->getMemoryFree());

        printf("Virtual Memory total: %ld MB\n", sysmon->getVirtualMemoryTotal());
        printf("Virtual Memory used: %ld MB\n", sysmon->getVirtualMemoryUsed());
        printf("Virtual Memory free: %ld MB\n\n", sysmon->getVirtualMemoryFree());

        printf("Disk total: %ld MB\n", sysmon->getDiskTotal());
        printf("Disk used: %ld MB\n", sysmon->getDiskUsed());
        printf("Disk free: %ld MB\n\n", sysmon->getDiskFree());

        printf("Disk read rate: %lld kBps\n", sysmon->getDiskReadRate());
        printf("Disk write rate: %lld kBps\n\n", sysmon->getDiskWriteRate());

        printf("Processor count: %d\n", sysmon->getProcessorCount());
        printf("Processor usage: %f %%\n\n", sysmon->getProcessorUsage());

        printf("Network TX rate: %lld kbps\n", sysmon->getTxRate());
        printf("Network RX rate: %lld kbps\n\n", sysmon->getRxRate());
    }

    return 0;
}
