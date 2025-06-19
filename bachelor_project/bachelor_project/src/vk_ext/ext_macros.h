#pragma once

//#define EXT_FUNCS()
#define DECL_EXT_FN(func_name) PFN_##func_name func_name
#define LD_EXT_FN_INS(func_name) func_name = (PFN_##func_name)vkGetInstanceProcAddr(instance, #func_name)