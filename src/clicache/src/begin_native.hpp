#if defined(__begin_native__hpp__)
#error Including begin_native.hpp mulitple times without end_native.hpp
#endif
#define __begin_native__hpp__

#pragma push_macro("_ALLOW_KEYWORD_MACROS")
#undef _ALLOW_KEYWORD_MACROS
#define _ALLOW_KEYWORD_MACROS

#pragma push_macro("nullptr")
#undef nullptr
#define nullptr __nullptr

#pragma warning(push)

// Disable XML warnings
#pragma warning(disable: 4635)
#pragma warning(disable: 4638)
#pragma warning(disable: 4641)

// Disable native code generation warning
#pragma warning(disable: 4793)
