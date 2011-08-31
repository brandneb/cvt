#include <cvt/cl/CLProgram.h>
#include <cvt/cl/CLContext.h>

namespace cvt {

	CLProgram::CLProgram( const CLContext& context, const char* prog )
	{
		cl_int err;
		_object = ::clCreateProgramWithSource( context, 1, &prog, NULL, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	CLProgram::CLProgram( const CLContext& context, const String& prog )
	{
		cl_int err;
		const char* str = prog.c_str();
		_object = ::clCreateProgramWithSource( context, 1, &str, NULL, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

}