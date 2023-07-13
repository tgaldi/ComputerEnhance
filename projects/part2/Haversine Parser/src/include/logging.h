#pragma once

#define LOG_OUT_STREAM stdout
#define LOG_ERROR_STREAM stderr

#define __BASENAME__(path) (strrchr(path, '/') ? strrchr(path, '/') + 1 : (strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path))

#define __FILENAME__ (__BASENAME__(__FILE__))

#define LOG_ERROR( message, ...) fprintf( LOG_ERROR_STREAM, "ERROR: %s -> %s() Line: %d\n" message "\n\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR_EXIT( error_code, message, ...) fprintf( LOG_ERROR_STREAM, "ERROR: %s -> %s() Line: %d\nError Code: %d\n" message "\n\n", __FILENAME__, __func__, __LINE__, error_code, ##__VA_ARGS__); exit( error_code )

#define LOG_WARNING( message, ...) fprintf( LOG_ERROR_STREAM, "WARNING: %s -> %s() Line: %d\n" message "\n\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)

#define LOG( message, ...) fprintf( LOG_OUT_STREAM, "LOG: %s -> %s() Line: %d\n" message "\n\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)

#define PRINT( message, ...) fprintf( LOG_OUT_STREAM, message "\n", ##__VA_ARGS__)

#define WRITE( message, ...) fprintf( LOG_OUT_STREAM, message, ##__VA_ARGS__)

#define WRITE_LINE( message, ...) fprintf( LOG_OUT_STREAM, "\n" message "\n", ##__VA_ARGS__)