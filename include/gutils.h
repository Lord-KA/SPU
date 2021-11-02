#ifndef GUTILS_H
#define GUTILS_H

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

static bool gPtrValid(const void* ptr)       
{
    if (ptr == 0) {
        return false;
    }
  
    #ifdef GUTILS_USE_PTR_SYS_CHECK
        #ifdef __unix__
            size_t page_size = sysconf(_SC_PAGESIZE);
            void *base = (void *)((((size_t)ptr) / page_size) * page_size);
            return msync(base, page_size, MS_ASYNC) == 0;
        #else
            #ifdef _WIN32
                MEMORY_BASIC_INFORMATION mbi = {};
                if (!VirtualQuery(ptr, &mbi, sizeof (mbi)))
                    return false;
  
                if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
                    return false;  // Guard page -> bad ptr
    
                DWORD readRights = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY
                    | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
    
                return (mbi.Protect & readRights) != 0;
            #else   
                fprintf(stderr, "WARNING: your OS is unsupported, system pointer checks are diabled!\n");
            #endif  /* _WIN32 */
        #endif  /* __unix__ */
    #endif  /* GUTILS_USE_PTR_SYS_CHECK */
  
    return true;
}

char* strnchr(const char *haystack, char needle, size_t lenght) 
{
    return (char*)memchr(haystack, needle, strnlen(haystack, lenght));

}

bool strnConsistsChrs(const char *haystack, const char* needles, size_t haystackLen, size_t needlesLen)
{
    char *iter = (char*)needles;
    while ((iter - needles) < needlesLen && *iter != '\0') {
        if (strnchr(haystack, *iter, haystackLen))
            return 1;
        ++iter;
    }
    return 0;
}

#include <stdio.h>

bool isInteger(const char *haystack) 
{
    /* 
     * WARNING: haystack must be a null-terminated string
     */

    bool hexadecimalMode = false;
    char *iter = (char*)haystack;
    while (isspace(*iter))
        ++iter;

    /* for hexadecimal and octal support */
    if (iter - haystack < strlen(haystack) - 1
            && *iter == '0'
            && (*(iter + 1) == 'o' || *(iter + 1) == 'x')) {
        if (*(iter + 1) == 'x')
            hexadecimalMode = true;
        iter += 2;
    }
    if (hexadecimalMode) {
        while (isdigit(*iter) || ('a' <= *iter && *iter <= 'f'))
            ++iter;
    } else {
        while (isdigit(*iter))
            ++iter;
    }

    while (isspace(*iter))
        ++iter;
    if (*iter == '\0')
        return true;
    else 
        return false;
}

bool isDouble(const char *haystack) {
    /* 
     * WARNING: haystack must be a null-terminated string
     */

    char *iter = (char*)haystack;
    while (isspace(*iter))
        ++iter;

    do {
        if (!isdigit(*iter))
            return false;
        ++iter;
    } while (*iter != '\0' && *iter != '.');

    do {
        ++iter;
    } while (isdigit(*iter));
    if (*iter == '\0')
        return true;

    if (*iter != 'e')
        return false;

    if (iter - haystack >= strlen(haystack) - 2)
        return false;
    ++iter;
    if (*iter != '+' && *iter != '-')
        return false;
    ++iter;
    while (*iter != '\0' && !isspace(*iter)) {
        if (!isdigit(*iter))
            return false;
        ++iter;
    }
    while (isspace(*iter))
        ++iter;
    if (*iter == '\0')
        return true;
    else 
        return false;
}

bool getline(char *buffer, size_t bufferLen, FILE *in) 
{
    assert(gPtrValid(buffer));
    assert(gPtrValid(in));

    char c;
    size_t curLen = 0;
    c = fgetc(in);
    while (!feof(in) && c != '\n') {
        if (curLen < bufferLen - 1) 
            buffer[curLen++] = c;
        else 
            goto finish;
        c = fgetc(in);
    }
    if (ferror(in))
        return 0;
finish:
    buffer[curLen++] = '\0';
    return 1;
}


#endif
