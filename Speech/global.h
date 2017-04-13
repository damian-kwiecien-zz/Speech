#pragma once

#ifdef DEVELOPMENT
#define DLL_MAGIC __declspec(dllexport)
#else
#define DLL_MAGIC __declspec(dllimport)
#endif
