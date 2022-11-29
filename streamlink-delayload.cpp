#include "obs-streamlink.h"
#include <Windows.h>
#include "delayimp.h"

#include <filesystem>
#include <cstring>

FARPROC WINAPI obs_streamlink_delay_load(unsigned dliNotify, PDelayLoadInfo pdli);
ExternC const PfnDliHook __pfnDliNotifyHook2 = obs_streamlink_delay_load;

FARPROC WINAPI obs_streamlink_delay_load(unsigned dliNotify, PDelayLoadInfo pdli)
{
    if (dliNotify == dliNotePreLoadLibrary)
    {
        auto pathFFmpeg = obs_streamlink_data_path / "ffmpeg" / pdli->szDll;
        auto pathPython = obs_streamlink_data_path / obs_streamlink_python_ver / pdli->szDll;
        if (exists(pathFFmpeg))
        {
            auto directLoad = LoadLibraryA(pdli->szDll);
            if (!directLoad)
                return reinterpret_cast<FARPROC>(LoadLibraryW(pathFFmpeg.wstring().c_str()));
            return reinterpret_cast<FARPROC>(directLoad);
        }
        if (exists(pathPython))
            return reinterpret_cast<FARPROC>(LoadLibraryW(pathPython.wstring().c_str()));
    }

    return 0;
}
