#include "FloFile.h"
#include "util/CVTException.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define TAG_FLOAT 202021.25  // check for this when READING the file
#define TAG_STRING "PIEH"    // use this when WRITING the file
#define UNKNOWN_FLOW_THRESH 1e9
#define UNKNOWN_FLOW 1e10 // value to use to represent unknown flow

// ".flo" file format used for optical flow evaluation
//
// Stores 2-band float image for horizontal (u) and vertical (v) flow components.
// Floats are stored in little-endian order.
// A flow value is considered "unknown" if either |u| or |v| is greater than 1e9.
//
//  bytes  contents
//
//  0-3     tag: "PIEH" in ASCII, which in little endian happens to be the float 202021.25
//          (just a sanity check that floats are represented correctly)
//  4-7     width as an integer
//  8-11    height as an integer
//  12-end  data (width*height*2*4 bytes total)
//          the float values for u and v, interleaved, in row order, i.e.,
//          u[row0,col0], v[row0,col0], u[row0,col1], v[row0,col1], ...
//


namespace cvt {
    namespace FloFile {

	void FloReadFile( Image& flow, std::string const& filename )
	{

	    if( filename.compare( filename.length() -4, 4, ".flo" ) != 0 )
		throw CVTException("FloReadFile " + filename + ": extension .flo expected");

	    FILE *stream = fopen( filename.c_str(), "rb" );
	    if (stream == 0)
		throw CVTException("FloReadFile: could not open " + filename );

	    int32_t width, height;
	    float tag;

	    if ( ( int ) fread(&tag,    sizeof( float ), 1, stream) != 1 ||
		( int ) fread(&width,  sizeof( int32_t ),   1, stream) != 1 ||
		( int ) fread(&height, sizeof( int32_t ),   1, stream) != 1)
		throw CVTException("FloReadFile: problem reading file " + filename);

	    if (tag != TAG_FLOAT) // simple test for correct endian-ness
		throw CVTException("FloReadFile(" + filename + "): wrong tag (possibly due to big-endian machine?)" );

	    // another sanity check to see that integers were read correctly (99999 should do the trick...)
	    if (width < 1 || width > 99999)
		throw CVTException("ReadFlowFile(" + filename + "): illegal width" );

	    if (height < 1 || height > 99999)
		throw CVTException("ReadFlowFile(" + filename + "): illegal height" );


	    flow.reallocate( width, height, CVT_GRAYALPHA, CVT_FLOAT );
	    size_t h = flow.height();
	    size_t w2 = flow.width() * 2;
	    float* ptr = ( float* ) flow.data();
	    size_t stride = flow.stride();
	    while( h-- ) {
		if( fread( ptr, sizeof( float ), w2 , stream) != w2)
		    throw CVTException("ReadFlowFile(" + filename + "): file is too short" );
		ptr += stride;
	    }

	    if( fgetc(stream) != EOF)
		throw CVTException("ReadFlowFile(" + filename + "): file is too long" );

	    fclose(stream);
	}


	void FloWriteFile( Image& flow, std::string const & filename )
	{

	    if( filename.compare( filename.length() -4, 4, ".flo" ) != 0 )
		throw CVTException("FloWriteFile " + filename + ": extension .flo expected");

	    if( flow.order() != CVT_GRAYALPHA && flow.type() != CVT_FLOAT )
		throw CVTException("FloWriteFile " + filename + ": illeagal image format");

	    FILE *stream = fopen(filename.c_str(), "wb");
	    if (stream == 0)
		throw CVTException("FloWriteFile (" + filename + "): could not open file");

	    int32_t width, height;
	    width = ( int32_t ) flow.width();
	    height = ( int32_t ) flow.height();

	    // write the header
	    fprintf(stream, TAG_STRING);
	    if ( fwrite( &width,  sizeof( int32_t ), 1, stream) != 1 ||
		 fwrite( &height, sizeof( int32_t ), 1, stream) != 1)
		throw CVTException( "FloWriteFile (" + filename + "): problem writing header" );

	    size_t h = flow.height();
	    size_t w2 = flow.width() * 2;
	    float* ptr = ( float* ) flow.data();
	    size_t stride = flow.stride();
	    while( h-- ) {
		if( fwrite( ptr, sizeof( float ), w2 , stream ) != w2)
		    throw CVTException("ReadFlowFile(" + filename + "): problem writing data" );
		ptr += stride;
	    }
	    fclose(stream);
	}
    }
}
