#if !defined(_SCVREP_EXPORT_ROBOT_LANG_)
#define _SCVREP_EXPORT_ROBOT_LANG_ 1


#if defined(WIN32)
#   if defined(VREPEXT_ROBOTLANG)
#       define VREP_ROBOTLANG_API __declspec(dllexport)
#   else
#       define VREP_ROBOTLANG_API __declspec(dllimport)
#		if defined(_DEBUG)
#			pragma message("===== Debug Build Mode: VrepExtRobotLangD.lib automatically linked! =====")
#			pragma comment(lib, "VrepExtRobotLangD.lib")
#		else
#			pragma message("===== Release Build Mode: VrepExtRobotLang.lib automatically linked! =====")
#			pragma comment(lib, "VrepExtRobotLang.lib")
#		endif  //  _DEBUG
#   endif  //  SC_KERNEL_MA_EXPORTS
#else
#   define VREP_ROBOTLANG_API
#endif  //  WIN32


#endif  //  _SCVREP_EXPORT_ROBOT_LANG_
