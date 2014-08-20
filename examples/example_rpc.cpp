/*
    Boost Software License - Version 1.0 - August 17, 2003

    Permission is hereby granted, free of charge, to any person or organization
    obtaining a copy of the software and accompanying documentation covered by
    this license (the "Software") to use, reproduce, display, distribute,
    execute, and transmit the Software, and to prepare derivative works of the
    Software, and to permit third-parties to whom the Software is furnished to
    do so, all subject to the following:

    The copyright notices in the Software and this entire statement, including
    the above license grant, this restriction and the following disclaimer,
    must be included in all copies of the Software, in whole or in part, and
    all derivative works of the Software, unless such copies or derivative
    works are solely in the form of machine-executable object code generated by
    a source language processor.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
    SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
    FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <reflection/api.hpp>
#include <reflection/basic_types.hpp>

using std::string;

template <typename Return, typename Arg1, typename Arg2, typename Arg3>
Return returnValueOf(Return (*)(Arg1, Arg2, Arg3));
template <typename Return, typename Arg1, typename Arg2, typename Arg3>
Arg1 arg1TypeOf(Return (*)(Arg1, Arg2, Arg3));
template <typename Return, typename Arg1, typename Arg2, typename Arg3>
Arg2 arg2TypeOf(Return (*)(Arg1, Arg2, Arg3));
template <typename Return, typename Arg1, typename Arg2, typename Arg3>
Arg3 arg3TypeOf(Return (*)(Arg1, Arg2, Arg3));

template <typename T> void dump(const T& value) {
    string v = reflection::reflectToString(value);
    puts(v.c_str());
}

// SERVER HEADER

// generate a 3-argument forwarder newName_->functionName_
#define DEFINE_RPC_3(newName_, functionName_)\
bool newName_(const string* args, string& output) {\
    decltype(arg1TypeOf(functionName_)) arg1;\
    decltype(arg2TypeOf(functionName_)) arg2;\
    decltype(arg3TypeOf(functionName_)) arg3;\
    ::reflection::reflectFromString(arg1, args[0]);\
    ::reflection::reflectFromString(arg2, args[1]);\
    ::reflection::reflectFromString(arg3, args[2]);\
    output = ::reflection::reflectToString(functionName_(arg1, arg2, arg3));\
    return true;\
}

// SERVER CODE

int getResourceFromServer(string resource, unsigned int maxSize, bool cached) {
    printf("[SERVER] getResourceFromServer(%s, %u, %d)\n", resource.c_str(), maxSize, cached);
    return 42;
}

DEFINE_RPC_3(getResourceFromServerWrapper, getResourceFromServer)

// process incoming RPCs
void processRPC(const char* functionName, string* args, size_t numArgs, string& result) {
    if (strcmp(functionName, "getResourceFromServer") == 0 && numArgs == 3)
        getResourceFromServerWrapper(args, result);
    else
        fprintf(stderr, "Invalid RPC %s(%d)\n", functionName, numArgs);
}

// CLIENT HEADER

namespace reflection {
// perform a 3-argument RPC
template <typename Return, typename Arg1, typename Arg2, typename Arg3>
Return rpcCall(const char* functionName, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3) {
    string args[3], result;
    args[0] = reflectToString(arg1);
    args[1] = reflectToString(arg2);
    args[2] = reflectToString(arg3);

    printf("-> server ['%s', '%s', '%s', '%s']\n", functionName,
            args[0].c_str(), args[1].c_str(), args[2].c_str());
    processRPC(functionName, args, 3, result);      // talk to server via IPC/REST/socket...
    printf("<- server ['%s']\n", result.c_str());

    Return ret;
    reflectFromString(ret, result);
    return ret;
}
}

// generate a 3-argument forwarder newName_->rpcCall
#define RPC_3(newName_, functionName_)\
decltype(returnValueOf(functionName_)) newName_(\
        const decltype(arg1TypeOf(functionName_))& arg1,\
        const decltype(arg2TypeOf(functionName_))& arg2,\
        const decltype(arg3TypeOf(functionName_))& arg3) {\
    return ::reflection::rpcCall<decltype(returnValueOf(functionName_))>(#functionName_, arg1, arg2, arg3);\
}

// CLIENT CODE

int getResourceFromServer(string resource, unsigned int maxSize, bool cached);

RPC_3(getResourceFromServerRPC, getResourceFromServer)

int main(int argc, char* argv[]) {
    int result = getResourceFromServerRPC("/test", 3000, true);
    printf("[CLIENT] Result is: %d\n", result);
}

#include <reflection/default_error_handler.cpp>

/*
-> server ['getResourceFromServer', '/test', '3000', 'true']
[SERVER] getResourceFromServer(/test, 3000, 1)
<- server ['42']
[CLIENT] Result is: 42
*/
