#ifndef SECUREKIT_EXPORT_HPP_
#define SECUREKIT_EXPORT_HPP_

#if defined(_WIN32) && defined(SECUREKIT_SHARED)
#if defined(SECUREKIT_BUILDING_LIBRARY)
#define SECUREKIT_API __declspec(dllexport)
#else
#define SECUREKIT_API __declspec(dllimport)
#endif
#else
#define SECUREKIT_API
#endif

#endif // SECUREKIT_EXPORT_HPP_
