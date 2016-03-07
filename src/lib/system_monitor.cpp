#include <string>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include "system_monitor.h"


using namespace std;

struct _SystemMonitorData {
    // general
    bool monitorProcessor;
    bool monitorNetwork;
    bool monitorDisk;
    string interface;
    int refreshRate;

    // processor
    unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
    int numProcessors;

    // network
    unsigned long long txBytes, rxBytes;

    // disk
    time_t diskStatsLastUpdated;
    unsigned long disk_size, disk_used;
    unsigned long rBytes, wBytes;

    // memory & uptime
    time_t memoryStatsLastUpdated;
    unsigned long memory_total, memory_used, virtual_total, virtual_used;
    unsigned long uptime;
};

SystemMonitor::SystemMonitor(bool monitorProcessor, bool monitorDisk,
        bool monitorNetwork, string networkInterface, int refreshRate = 1)
{
    d = new _SystemMonitorData;
    d->diskStatsLastUpdated   = 0;
    d->memoryStatsLastUpdated = 0;

    d->monitorProcessor = monitorProcessor;
    d->monitorNetwork   = monitorNetwork;
    d->monitorDisk      = monitorDisk;
    d->interface        = networkInterface;
    d->refreshRate      = refreshRate;

    if (d->monitorNetwork) {
        initNetworkStats();
    }
    if (d->monitorProcessor) {
        initProcessorStats();
    }
    if (d->monitorDisk) {
        initDiskIOStats();
    }
}

SystemMonitor::~SystemMonitor()
{
    delete d;
    d = NULL;
}

/*** PROCESSOR STATS - START ***/

void SystemMonitor::initProcessorStats()
{
    FILE* file = fopen("/proc/stat", "r");
    if (NULL != file) {
        fscanf(file, "cpu %llu %llu %llu %llu", &d->lastTotalUser, &d->lastTotalUserLow,
            &d->lastTotalSys, &d->lastTotalIdle);
        fclose(file);
    }

    char line[128];

    file = fopen("/proc/cpuinfo", "r");
    d->numProcessors = 0;
    if (NULL != file) {
        while(fgets(line, 128, file) != NULL){
            if (strncmp(line, "processor", 9) == 0) {
                d->numProcessors++;
            }
        }
        fclose(file);
    }
}

float SystemMonitor::getProcessorUsage()
{
    double percent, diff;
    FILE* file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle;

    file = fopen("/proc/stat", "r");
    if (NULL != file) {
        fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
            &totalSys, &totalIdle);
        fclose(file);
    }

    if (totalIdle < d->lastTotalIdle){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        diff = (double)(totalIdle - d->lastTotalIdle) / 100.0;
        percent = 100.0 - ((diff / (double)d->numProcessors) * 100.0);
    }
    d->lastTotalIdle = totalIdle;

    return percent;
}

int SystemMonitor::getProcessorCount()
{
    return d->numProcessors;
}

/*** PROCESSOR STATS - END ***/


/*** NETWORK STATS - START ***/

unsigned long long SystemMonitor::readInterface(string ttype)
{
    unsigned long long result;
    char fname[100];
    sprintf(fname, "/sys/class/net/%s/statistics/%s_bytes", d->interface.c_str(), ttype.c_str());

    FILE* file = fopen(fname, "r");
    if (NULL != file) {
        fscanf(file, "%llu", &result);
        fclose(file);
    }

    return result;
}

void SystemMonitor::initNetworkStats()
{
    d->rxBytes = readInterface("rx");
    d->txBytes = readInterface("tx");
}

unsigned long long SystemMonitor::getTxRate()
{
    unsigned long long currentBytes = readInterface("tx");
    unsigned long long result = (currentBytes - d->txBytes) / 1024;
    d->txBytes = currentBytes;
    return result;
}

unsigned long long SystemMonitor::getRxRate()
{
    unsigned long long currentBytes = readInterface("rx");
    unsigned long long result = (currentBytes - d->rxBytes) / 1024;
    d->rxBytes = currentBytes;
    return result;
}

/*** NETWORK STATS - END ***/


/*** MEMORY STATS - START ***/

void SystemMonitor::initMemoryStats()
{
    struct sysinfo info;

    if (sysinfo(&info) != 0) {
        d->memory_total  = 0;
        d->memory_used   = 0;
        d->virtual_total = 0;
        d->virtual_used  = 0;
        d->uptime        = 0;
        return;
    }
    d->memory_total  = info.totalram * info.mem_unit;
    d->memory_used   = (info.totalram - info.freeram) * info.mem_unit;
    d->virtual_total = info.totalswap * info.mem_unit;
    d->virtual_used  = (info.totalswap - info.freeswap) * info.mem_unit;
    d->uptime        = info.uptime;

    d->memoryStatsLastUpdated = time(0);
}

unsigned long SystemMonitor::getSystemUptime()
{
    if (checkNeedRefresh(time(0), d->memoryStatsLastUpdated)) {
        initMemoryStats();
    }
    return d->uptime;
}

unsigned long SystemMonitor::getMemoryTotal()
{
    if (checkNeedRefresh(time(0), d->memoryStatsLastUpdated)) {
        initMemoryStats();
    }
    return d->memory_total / 1024 / 1024;
}

unsigned long SystemMonitor::getMemoryUsed()
{
    if (checkNeedRefresh(time(0), d->memoryStatsLastUpdated)) {
        initMemoryStats();
    }
    return d->memory_used / 1024 / 1024;
}

unsigned long SystemMonitor::getMemoryFree()
{
    if (checkNeedRefresh(time(0), d->memoryStatsLastUpdated)) {
        initMemoryStats();
    }
    return (d->memory_total - d->memory_used) / 1024 / 1024;
}

unsigned long SystemMonitor::getVirtualMemoryTotal()
{
    if (checkNeedRefresh(time(0), d->memoryStatsLastUpdated)) {
        initMemoryStats();
    }
    return d->virtual_total / 1024 / 1024;
}

unsigned long SystemMonitor::getVirtualMemoryUsed()
{
    if (checkNeedRefresh(time(0), d->memoryStatsLastUpdated)) {
        initMemoryStats();
    }
    return d->virtual_used / 1024 / 1024;
}

unsigned long SystemMonitor::getVirtualMemoryFree()
{
    if (checkNeedRefresh(time(0), d->memoryStatsLastUpdated)) {
        initMemoryStats();
    }
    return (d->virtual_total - d->virtual_used) / 1024 / 1024;
}

/*** MEMORY STATS - END ***/


/*** DISK STATS - START ***/

unsigned long long SystemMonitor::readDiskStats(string ttype)
{
    unsigned long long readValue, writeValue;
	string line;
	unsigned long long result = 0;

	ifstream statsFile("/proc/diskstats");
	while (getline(statsFile, line)) {
		stringstream lineStream(line);
		string device_name, lineToken;
		int i = 0;
		// get necessary data
		while (lineStream >> lineToken) {
			switch (i) {
				case 2:
					device_name = lineToken;
					break;
				case 5:
					readValue = atol(lineToken.c_str());
					break;
				case 9:
					writeValue = atol(lineToken.c_str());
					break;
			}
			i++;
		}

		// consider only devices 3 letters long and with "d" in the middle
		if (device_name.length() == 3 && device_name[1] == 'd') {
			if (ttype == "r") {
				result += readValue / 2;
			}
			else {
				result += writeValue / 2;
			}
		}
	}

    return result;
}

void SystemMonitor::initDiskIOStats()
{
    d->rBytes = readDiskStats("r");
    d->wBytes = readDiskStats("w");
}

unsigned long long SystemMonitor::getDiskWriteRate()
{
    unsigned long long currentkBytes = readDiskStats("w");
    unsigned long long result = currentkBytes - d->wBytes;
    d->wBytes = currentkBytes;
    return result;
}

unsigned long long SystemMonitor::getDiskReadRate()
{
    unsigned long long currentkBytes = readDiskStats("r");
    unsigned long long result = currentkBytes - d->rBytes;
    d->rBytes = currentkBytes;
    return result;
}

void SystemMonitor::initDiskStats()
{
	struct statvfs buf;

	if (!statvfs("/", &buf)) {
		unsigned long blksize, blocks, freeblks, free;

		blksize = buf.f_bsize;
		blocks = buf.f_blocks;
		freeblks = buf.f_bfree;

		free = freeblks * blksize;
		d->disk_size = blocks * blksize;
		d->disk_used = d->disk_size - free;

		d->diskStatsLastUpdated = time(0);
	}
}

unsigned long SystemMonitor::getDiskTotal()
{
    if (checkNeedRefresh(time(0), d->diskStatsLastUpdated)) {
        initDiskStats();
    }
    return d->disk_size / 1024 / 1024;
}

unsigned long SystemMonitor::getDiskUsed()
{
    if (checkNeedRefresh(time(0), d->diskStatsLastUpdated)) {
        initDiskStats();
    }
    return d->disk_used / 1024 / 1024;
}

unsigned long SystemMonitor::getDiskFree()
{
    if (checkNeedRefresh(time(0), d->diskStatsLastUpdated)) {
        initDiskStats();
    }
    return (d->disk_size - d->disk_used) / 1024 / 1024;
}

/*** DISK STATS - END ***/


bool SystemMonitor::checkNeedRefresh(time_t clockNow, time_t clockPrev)
{
    if (difftime(clockNow, clockPrev) >= d->refreshRate) {
        return true;
    }
    return false;
}
