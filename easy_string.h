#include <stdarg.h>

static u32 easyString_getSizeInBytes_utf8(char *string) {
    u32 result = 0;
    u8 *at = (u8 *)string;
    while(*at) {
        result++;
        at++;
    }
    return result;
}

static u32 easyString_getStringLength_utf8(char *string) {
    u32 result = 0;
    u8 *at = (u8 *)string;
    while(*at) {
        easyUnicode_utf8ToUtf32((unsigned char **)&at, true);
        result++;
    }
    return result;
}

bool stringsMatchN(char *a, int aLength, char *b, int bLength) {
    bool result = true;
    
    int indexCount = 0;
    while(indexCount < aLength && indexCount < bLength) {
        indexCount++;
        result &= (*a == *b);
        a++;
        b++;
    }
    result &= (indexCount == bLength && indexCount == aLength);
    
    return result;
} 


bool stringsMatchNullN(char *a, char *b, int bLen) {
    bool result = stringsMatchN(a, easyString_getStringLength_utf8(a), b, bLen);
    return result;
}

bool cmpStrNull(char *a, char *b) {
    bool result = stringsMatchN(a, easyString_getStringLength_utf8(a), b, easyString_getStringLength_utf8(b));
    return result;
}


inline char *easy_createString_printf_needToFree(char *formatString, ...) {

    va_list args;
    va_start(args, formatString);

    char bogus[4];
    int stringLengthToAlloc = vsnprintf(bogus, 1, formatString, args) + 1; //for null terminator, just to be sure
    
    char *strArray = malloc(stringLengthToAlloc*sizeof(char));

    vsnprintf(strArray, stringLengthToAlloc, formatString, args); //for null terminator, just to be sure

    va_end(args);

    return strArray;
}



static char *easyString_copyToHeap(char *at) {
    u32 length = easyString_getSizeInBytes_utf8(at);
    //NOTE(ollie): Get memory from heap
    char *result = (char *)easyPlatform_allocateMemory(sizeof(char)*(length + 1), EASY_PLATFORM_MEMORY_NONE);
    //NOTE(ollie): Copy the string
    easyPlatform_copyMemory(result, at, sizeof(char)*length);
    //NOTE(ollie): Null terminate the string
    result[length] = '\0'; //Null terminate

    return result;
}

static char *easyString_copyToBuffer(char *at, char *buffer, u32 bufferLen) {
    
    assert(easyString_getSizeInBytes_utf8(at) < bufferLen); //NOTE(ollie): Accounting for the null terminator 
    //NOTE(ollie): Copy the string
    easyPlatform_copyMemory(buffer, at, sizeof(char)*bufferLen);
    //NOTE(ollie): Null terminate the string
    buffer[bufferLen - 1] = '\0'; //Null terminate

    return buffer;
}
