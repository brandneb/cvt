#include <CLCommandQueue.h>
#include <CLContext.h>
#include <CLDevice.h>

namespace cvt {
	CLCommandQueue::CLCommandQueue( cl_command_queue q ) : CLObject<cl_command_queue>( q )
	{
	}

	CLCommandQueue::CLCommandQueue( const CLCommandQueue& q ) : CLObject<cl_command_queue>( q._object )
	{
	}

	CLCommandQueue::CLCommandQueue( const CLContext& context, const CLDevice& device, cl_command_queue_properties props )
	{
		cl_int err;
		_object = ::clCreateCommandQueue( context, device, props, &err );
		if( err != CL_SUCCESS )
			throw CLException( err );
	}

	CLContext CLCommandQueue::context() const
	{
		return CLContext( queueContext() );
	}

	CLDevice CLCommandQueue::device() const
	{
		return CLDevice( queueDevice() );
	}

}
