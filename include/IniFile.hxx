//
// IniFile.hxx
//

#ifndef INIFILE_HXX
#define INIFILE_HXX

#include <vector>

namespace INI
{
    enum EIniType
    {
        EiniTypeNone,
        EiniTypeCpu,
        EiniTypeGpu,
        EiniTypeRam
    };

    struct CpuInfo
    {
        wxString Name;
        int Cores = 0;
    };
    typedef std::vector<CpuInfo> CpuInfoList;

    struct GpuInfo
    {
        wxString Name;
    };
    typedef std::vector<GpuInfo> GpuInfoList;

    struct RamInfo
    {
        wxString Name;
        wxString Type;
        int SizeGB = 0;
        int SpeedMTs = 0;
    };
    typedef std::vector<RamInfo> RamInfoList;

    class IniFile
    {
    public:
        static IniFile& Inst();

        ~IniFile() = default;

        IniFile(const IniFile&) = delete;
        IniFile& operator=(const IniFile&) = delete;
        IniFile(IniFile&&) = delete;
        IniFile& operator=(IniFile&&) = delete;

        CpuInfoList GetCpus(EIniType iniType) const;
        GpuInfoList GetGpus(EIniType iniType) const;
        RamInfoList GetRams(EIniType iniType) const;

    private:
        static CpuInfoList s_Cpus;
        static GpuInfoList s_Gpus;
        static RamInfoList s_Rams;

        IniFile() = default;
    };
}

#endif // INIFILE_HXX
