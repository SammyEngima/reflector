'''
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
'''

MAX_ARGS = 10

def generate_rpcSerializedCall(num_args, void):
    template_args = ', '.join(['const char* functionName', 'typename Return'] +
        list('typename Arg%d' % i for i in range(0, num_args)) +
        [])

    print('template <' + template_args + '>')

    if not void:
        print('Return rpcSerializedCall(')
    else:
        print('void rpcSerializedCallVoid(')

    print('        ' + ', '.join('Arg%d const& arg%d' % (i, i) for i in range(0, num_args)) + ') {')
    print('    IWriter* writer;')
    print('    IReader* reader;')
    print()
    print('    assert(beginRPC(functionName, ' + ('true' if not void else 'false') + ', writer, reader));')
    print()

    for i in range(0, num_args):
        print('    assert(reflectSerialize(arg%d, writer));' % i)
        if i + 1 == num_args: print()

    print('    assert(invokeRPC());')
    print()

    if not void:
        print('    Return result;')
        print('    assert(reflectDeserialize(result, reader));')
        print()
        print('    endRPC();')
        print('    return result;')
    else:
        print('    endRPC();')

    print('}')
    print()

def generate_rpcSerializedExecute(num_args, void):
    if not void:
        template_args = ['typename Return']
        template_arg_list = ['Return']
        returnType = 'Return'
    else:
        template_args = []
        template_arg_list = []
        returnType = 'void'

    arg_types = ', '.join('Arg%d' % i for i in range(0, num_args))
    func_variable = returnType + ' (*function)(' + arg_types + ')'

    template_args = ', '.join(template_args +
        list('typename Arg%d' % i for i in range(0, num_args)) +
        [func_variable])

    print('template <' + template_args + '>')

    print('bool rpcSerializedExecute(IErrorHandler* err, IReader* reader, IWriter* writer) {')
    for i in range(0, num_args):
        print('    typename std::remove_cv<typename std::remove_reference<Arg%d>::type>::type arg%d;' % (i, i))
        if i + 1 == num_args: print()

    for i in range(0, num_args):
        print('    if (!reflectDeserialize(arg%d, reader)) return false;' % i)
        if i + 1 == num_args: print()

    if not void:
        print('    const Return result = function(' + ', '.join('arg%d' % i for i in range(0, num_args)) + ');')
        print()
        print('    if (!reflectSerialize(result, writer)) return false;')
    else:
        print('    function(' + ', '.join('arg%d' % i for i in range(0, num_args)) + ');')

    print()
    print('    return true;')
    print('}')
    print()

def generate_getRpcSerializedCall(num_args, void):
    if not void:
        template_args = ['const char* functionName', 'typename Return']
        template_arg_list = ['functionName', 'Return']
        returnType = 'Return'
        makeFunctionPointer = "MakeFunctionPointer"
    else:
        template_args = ['const char* functionName']
        template_arg_list = ['functionName']
        returnType = 'void'
        makeFunctionPointer = "MakeFunctionPointerVoid"

    template_args = ', '.join(template_args +
        list('typename Arg%d' % i for i in range(0, num_args)) +
        [])

    template_arg_list = ', '.join(template_arg_list +
        list('Arg%d' % i for i in range(0, num_args)) +
        [])

    arg_types = ', '.join('Arg%d' % i for i in range(0, num_args))
    func_type = returnType + ' (*)(' + arg_types + ')'
    func_variable = returnType + ' (*functionNull)(' + arg_types + ')'

    mfp_template_args = (['typename Return'] if not void else []) + list('typename Arg%d' % i for i in range(0, num_args))
    if mfp_template_args: print('template <' + ', '.join(mfp_template_args) + '>')
    print('struct ' + makeFunctionPointer + '%d {' % num_args)
    print('    typedef ' + returnType + ' (*type)(' + ', '.join('Arg%d const&' % i for i in range(0, num_args)) + ');')
    print('};')
    print()

    print('template <' + template_args + '>')

    if not void:
        print(('RPC_CONSTEXPR_FUNC typename ' + makeFunctionPointer + '%d<' % num_args) +
            ', '.join(['Return'] + list('Arg%d' % i for i in range(0, num_args))) +
            '>::type getRpcSerializedCall(' + func_variable + ') {')
        print('    return &rpcSerializedCall<' + template_arg_list + '>;')
        print('}')
        print()
    else:
        mfp_template_args = list('Arg%d' % i for i in range(0, num_args))
        if mfp_template_args:
            print(('RPC_CONSTEXPR_FUNC typename ' + makeFunctionPointer + '%d<' % num_args) +
                ', '.join(mfp_template_args) +
                '>::type getRpcSerializedCall(' + func_variable + ') {')
        else:
            print(('RPC_CONSTEXPR_FUNC typename ' + makeFunctionPointer + '%d' % num_args) +
                '::type getRpcSerializedCall(' + func_variable + ') {')
        print('    return &rpcSerializedCallVoid<' + template_arg_list + '>;')
        print('}')
        print()

def generate_getRpcSerializedExecute(num_args, void):
    if not void:
        template_args = ['typename Function', 'Function func', 'typename Return']
        template_arg_list = ['Return']
        returnType = 'Return'
    else:
        template_args = ['typename Function', 'Function func']
        template_arg_list = []
        returnType = 'void'

    template_args = ', '.join(template_args +
        list('typename Arg%d' % i for i in range(0, num_args)) +
        [])

    template_arg_list = ', '.join(template_arg_list +
        list('Arg%d' % i for i in range(0, num_args)) +
        ['func'])

    arg_types = ', '.join('Arg%d' % i for i in range(0, num_args))
    func_type = returnType + ' (*)(' + arg_types + ')'
    func_variable = returnType + ' (*functionNull)(' + arg_types + ')'

    print('template <' + template_args + '>')

    print('RPC_CONSTEXPR_FUNC bool (*getRpcSerializedExecute(' + func_variable + '))(')
    print('        IErrorHandler* err, IReader* reader, IWriter* writer) {')
    print('    return &rpcSerializedExecute<' + template_arg_list + '>;')
    print('}')
    print()

print('#pragma once')
print()
print('// Generated by gen_rpc_header.py')
print()
print('#include "function.hpp"')
print()

print('namespace rpc {')
print()

for num_args in range(0, MAX_ARGS + 1):
    for void in [False, True]:
        generate_rpcSerializedCall(num_args, void)
        generate_rpcSerializedExecute(num_args, void)
        generate_getRpcSerializedCall(num_args, void)
        generate_getRpcSerializedExecute(num_args, void)

print('}')
