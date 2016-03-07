#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <string>
#include <ctime>

using namespace std;

struct _SystemMonitorData;

class SystemMonitor {
    public:
        SystemMonitor(bool monitorProcessor, bool monitorNetwork, bool monitorDisk, string networkInterface, int refreshRate);
        ~SystemMonitor();

    public:
        unsigned long getMemoryTotal();
        unsigned long getMemoryUsed();
        unsigned long getMemoryFree();
        
        unsigned long getVirtualMemoryTotal();
        unsigned long getVirtualMemoryUsed();
        unsigned long getVirtualMemoryFree();
        
        unsigned long getSystemUptime();

    public:
        unsigned long getDiskTotal();
        unsigned long getDiskUsed();
        unsigned long getDiskFree();
        unsigned long long getDiskReadRate();
        unsigned long long getDiskWriteRate();
        
    public:
        unsigned long long getRxRate();
        unsigned long long getTxRate();
        
    public:
        float getProcessorUsage();
        int getProcessorCount();

    protected:
        unsigned long long readInterface(string ttype);
        unsigned long long readDiskStats(string ttype);
        bool checkNeedRefresh(clock_t clockNow, clock_t clockPrev);
        
        void initProcessorStats();
        void initNetworkStats();
        void initMemoryStats();
        void initDiskStats();
        void initDiskIOStats();

    protected:
        struct _SystemMonitorData* d;
};

#endif
