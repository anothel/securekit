#ifndef AMV_EXPORT_HPP_
#define AMV_EXPORT_HPP_

#if defined(_WIN32) && defined(AMV_SHARED)
#if defined(AMV_BUILDING_LIBRARY)
#define AMV_API __declspec(dllexport)
#else
#define AMV_API __declspec(dllimport)
#endif
#else
#define AMV_API
#endif

#endif // AMV_EXPORT_HPP_
