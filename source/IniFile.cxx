//
// IniFile.cxx
//

#include "prc.hxx"
#include "IniFile.hxx"
#include <wx/fileconf.h>

namespace INI
{
	CpuInfoList IniFile::s_Cpus;
	GpuInfoList IniFile::s_Gpus;
	RamInfoList IniFile::s_Rams;

	namespace
	{
		wxFileConfig CreateConfig(const wxString& filePath)
		{
			return wxFileConfig(wxEmptyString, wxEmptyString, filePath, wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
		}
	}

	IniFile& IniFile::Inst()
	{
		static IniFile s_IniFile;

		s_Cpus.clear();
		s_Gpus.clear();
		s_Rams.clear();

		{
			wxFileConfig config = CreateConfig(g_IniCpuFilePath);
			long count = 0;
			config.SetPath("/CPUS");
			config.Read("Count", &count, 0);
			for (long i = 0; i < count; ++i) {
				config.SetPath(wxString::Format("/CPU_%ld", i));
				CpuInfo info;
				config.Read("Name", &info.Name, wxEmptyString);
				long cores = 0;
				config.Read("Cores", &cores, 0);
				info.Cores = static_cast<int>(cores);
				s_Cpus.push_back(info);
			}
		}

		{
			wxFileConfig config = CreateConfig(g_IniGpuFilePath);
			long count = 0;
			config.SetPath("/GPUS");
			config.Read("Count", &count, 0);
			for (long i = 0; i < count; ++i) {
				config.SetPath(wxString::Format("/GPU_%ld", i));
				GpuInfo info;
				config.Read("Name", &info.Name, wxEmptyString);
				s_Gpus.push_back(info);
			}
		}

		{
			wxFileConfig config = CreateConfig(g_IniRamFilePath);
			long count = 0;
			config.SetPath("/RAMS");
			config.Read("Count", &count, 0);
			for (long i = 0; i < count; ++i) {
				config.SetPath(wxString::Format("/RAM_%ld", i));
				RamInfo info;
				config.Read("Name", &info.Name, wxEmptyString);
				config.Read("Type", &info.Type, wxEmptyString);
				long sizeGB = 0;
				long speedMTs = 0;
				config.Read("SizeGB", &sizeGB, 0);
				config.Read("SpeedMTs", &speedMTs, 0);
				info.SizeGB = static_cast<int>(sizeGB);
				info.SpeedMTs = static_cast<int>(speedMTs);
				s_Rams.push_back(info);
			}
		}

		return s_IniFile;
	}

	CpuInfoList IniFile::GetCpus(EIniType iniType) const
	{
		if (iniType != EiniTypeCpu) {
			return CpuInfoList();
		}

		return s_Cpus;
	}

	GpuInfoList IniFile::GetGpus(EIniType iniType) const
	{
		if (iniType != EiniTypeGpu) {
			return GpuInfoList();
		}

		return s_Gpus;
	}

	RamInfoList IniFile::GetRams(EIniType iniType) const
	{
		if (iniType != EiniTypeRam) {
			return RamInfoList();
		}

		return s_Rams;
	}
}
