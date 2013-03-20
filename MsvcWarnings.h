#ifndef MSVCWARNINGS_H_C0A15551_E258_4E4
#define MSVCWARNINGS_H_C0A15551_E258_4E4

//Файл отключает слишком неоднозначные предупреждения MSVC и позволяет включить
//четвёртый уровень предупреждений в MSVC
//http://www.peeep.us/e84b5941
//http://alfps.wordpress.com/2010/06/05/cppx-w4-no-warnings-part-ii-disabling-msvc-sillywarnings/
#ifdef _MSC_VER

#pragma warning( disable: 4061 )    // enum value is not *explicitly* handled in switch
#pragma warning( disable: 4099 )    // first seen using 'struct' now seen using 'class'
#pragma warning( disable: 4127 )    // conditional expression is constant
#pragma warning( disable: 4217 )    // member template isn't copy constructor
#pragma warning( disable: 4250 )    // inherits (implements) some member via dominance
#pragma warning( disable: 4251 )    // needs to have dll-interface to be used by clients
#pragma warning( disable: 4275 )    // exported class derived from non-exported class
#pragma warning( disable: 4347 )    // "behavior change", function called instead of template
#pragma warning( disable: 4355 )    // "'this': used in member initializer list
#pragma warning( disable: 4505 )    // unreferenced function has been removed
#pragma warning( disable: 4510 )    // default constructor could not be generated
#pragma warning( disable: 4511 )    // copy constructor could not be generated
#pragma warning( disable: 4512 )    // assignment operator could not be generated
#pragma warning( disable: 4513 )    // destructor could not be generated
#pragma warning( disable: 4610 )    // can never be instantiated user defined constructor required
#pragma warning( disable: 4623 )    // default constructor could not be generated
#pragma warning( disable: 4624 )    // destructor could not be generated
#pragma warning( disable: 4625 )    // copy constructor could not be generated
#pragma warning( disable: 4626 )    // assignment operator could not be generated
#pragma warning( disable: 4640 )    // a local static object is not thread-safe
#pragma warning( disable: 4661 )    // a member of the template class is not defined.
#pragma warning( disable: 4670 )    // a base class of an exception class is inaccessible for catch
#pragma warning( disable: 4672 )    // a base class of an exception class is ambiguous for catch
#pragma warning( disable: 4673 )    // a base class of an exception class is inaccessible for catch
#pragma warning( disable: 4675 )    // resolved overload was found by argument-dependent lookup
#pragma warning( disable: 4702 )    // unreachable code, e.g. in <list> header.
#pragma warning( disable: 4710 )    // call was not inlined
#pragma warning( disable: 4711 )    // call was inlined
#pragma warning( disable: 4820 )    // some padding was added
#pragma warning( disable: 4917 )    // a GUID can only be associated with a class, interface or namespace

//Дополнительно, для того, чтобы скомпилить boost
#pragma warning( disable: 4245 )    //'initializing' : conversion from 'int' to 'const boost::detail::mask_uint_t<8>::least', signed/unsigned mismatch

#endif

#endif //#define MSVCWARNINGS_H_C0A15551_E258_4E4
