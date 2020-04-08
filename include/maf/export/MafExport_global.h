#pragma once

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) ||                  \
    defined(__WIN64__) || defined(WIN32) || defined(_WIN32) ||                 \
    defined(__WIN32__) || defined(__NT__)
#define MAF_DECL_EXPORT __declspec(dllexport)
#define MAF_DECL_IMPORT __declspec(dllimport)
#else
#define MAF_DECL_EXPORT __attribute__((visibility("default")))
#define MAF_DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(MAF_SHARED_LIBRARY)
#define MAF_EXPORT MAF_DECL_EXPORT
#elif MAF_STATIC_LIBRARY
#define MAF_EXPORT
#else
#define MAF_EXPORT MAF_DECL_IMPORT
#endif
