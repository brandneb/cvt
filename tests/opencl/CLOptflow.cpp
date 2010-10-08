#include <CLOptflow.h>
#include <cvt/gfx/IFilterScalar.h>
#include <string>
#include <iostream>

#include <highgui.h>
#include <cvt/io/ImageIO.h>

/* Kernel include */
#include "calcp1.h"
#include "calcp2.h"
#include "clearimg.h"
#include "copyimg.h"
#include "bilindown.h"
#include "bilinup.h"
#include "warp.h"
#include "warpsub.h"
#include "grad.h"
#include "threshold.h"
#include "median.h"
#include "colorcode.h"
#include "gradxy.h"

#define LAMBDA 80.0f
#define THETA 0.2f
#define NUMWARP 5
#define NUMROF 10
#define TAU 0.249f

namespace cvt {

	CLOptflow::CLOptflow() : pyridx( 0 )
	{
		std::string log;
		kernelcopyimg.build( "Denoise_COPYIMG", _copyimg_source, strlen( _copyimg_source ), log );
		std::cout << "Log: " << log << std::endl;
		kernelclearimg.build( "Denoise_CLEARIMG", _clearimg_source, strlen( _clearimg_source ), log );
		std::cout << "Log: " << log << std::endl;
		kernelp1.build( "Denoise_CALCP1", _calcp1_source, strlen( _calcp1_source ), log );
		std::cout << "WG-Size CALCP1: " << kernelp1.workGroupSize() << std::endl;
		std::cout << "Log P1: " << log << std::endl;
		kernelp2 .build( "Denoise_CALCP2", _calcp2_source, strlen( _calcp2_source ), log );
		std::cout << "WG-Size CALCP2: " << kernelp2.workGroupSize() << std::endl;
		std::cout << "Log P2: " << log << std::endl;
		kernelbidown.build("BILINDOWN", _bilindown_source, strlen( _bilindown_source), log );
		std::cout << "Log BILINDOWN: " << log << std::endl;
		kernelbiup.build("BILINUP", _bilinup_source, strlen( _bilinup_source), log );
		std::cout << "Log BILINUP: " << log << std::endl;
		kernelwarp.build("WARP", _warp_source, strlen( _warp_source ), log );
		std::cout << "Log WARP: " << log << std::endl;
		std::cout << "WG-Size WARP: " << kernelwarp.workGroupSize() << std::endl;
		kernelwarpsub.build("WARPSUB", _warpsub_source, strlen( _warpsub_source ), log );
		std::cout << "Log WARPSUB: " << log << std::endl;
		kernelgrad.build("GRAD", _grad_source, strlen( _grad_source ), log );
		std::cout << "WG-Size GRAD: " << kernelgrad.workGroupSize() << std::endl;
		std::cout << "Log GRAD: " << log << std::endl;

		kernelgradxy.build("GRADXY", _gradxy_source, strlen( _gradxy_source ), log );
		std::cout << "WG-Size GRADXY: " << kernelgradxy.workGroupSize() << std::endl;
		std::cout << "Log GRADXY: " << log << std::endl;

		kernelth.build("THRESHOLD", _threshold_source, strlen( _threshold_source ), log );
		std::cout << "WG-Size THRESHOLD: " << kernelth.workGroupSize() << std::endl;
		std::cout << "Log TH: " << log << std::endl;
		kernelmed.build("MEDIAN3", _median_source, strlen( _median_source ), log );
		std::cout << "WG-Size MEDIAN: " << kernelth.workGroupSize() << std::endl;
		std::cout << "Log MEDIAN: " << log << std::endl;
		kernelcflow.build("FlowColorCode", _colorcode_source, strlen( _colorcode_source ), log );
		std::cout << "WG-Size FlowColorCode: " << kernelcflow.workGroupSize() << std::endl;
		std::cout << "Log FlowColorCode: " << log << std::endl;

		/* FIXME: lowest level defines input format */
		pyr[ 0 ][ 0 ] = new Image( 160, 480, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 0 ][ 1 ] = new Image( 160, 480, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 1 ][ 0 ] = new Image( 80, 240, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 1 ][ 1 ] = new Image( 80, 240, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 2 ][ 0 ] = new Image( 40, 120, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 2 ][ 1 ] = new Image( 40, 120, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 3 ][ 0 ] = new Image( 20, 60, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 3 ][ 1 ] = new Image( 20, 60, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 4 ][ 0 ] = new Image( 10, 30, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
		pyr[ 4 ][ 1 ] = new Image( 10, 30, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL  );
	}

	CLOptflow::~CLOptflow()
	{
		delete pyr[ 0 ][ 0 ];
		delete pyr[ 0 ][ 1 ];
		delete pyr[ 1 ][ 0 ];
		delete pyr[ 1 ][ 1 ];
		delete pyr[ 2 ][ 0 ];
		delete pyr[ 2 ][ 1 ];
		delete pyr[ 3 ][ 0 ];
		delete pyr[ 3 ][ 1 ];
		delete pyr[ 4 ][ 0 ];
		delete pyr[ 4 ][ 1 ];
	}

	Image* CLOptflow::updateFlow( const Image& img )
	{
		int pyridx2 = pyridx;
		std::vector<cl::Event> sync;
		cl::Event event;

		pyridx ^= 0x01;

		pyr[ 0 ][ pyridx ];
		{
			size_t dstride;
			uint8_t* dbase;
			size_t sstride;
			const uint8_t* sbase;
			size_t h, n;
			const uint8_t* src;
			uint8_t* dst;

			sbase = img.map( &sstride );
			h = img.height();
			n = img.width() * img.bpp();
			dbase = pyr[ 0 ][ pyridx ]->map( &dstride );
			dst = dbase;
			src = sbase;
			while( h-- ) {
				memcpy( dst, src, n );
				dst += dstride;
				src += sstride;
			}
			img.unmap( sbase );
			pyr[ 0 ][ pyridx ]->unmap( dbase );
		}

		for( int i = 0; i < 4; i++ ) {
			kernelbidown.setArg( 0, pyr[ i + 1 ][ pyridx ] );
			kernelbidown.setArg( 1, pyr[ i ][ pyridx ] );
			kernelbidown.run( *pyr[ i + 1 ][ pyridx ], cl::NDRange( 10, 10 ), &sync, &event );
			sync.clear();
			sync.push_back( event );
		}

		Image* u = new Image( 20, 30, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image* v = new Image( 20, 30, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image* px = new Image( 20, 30, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image* py = new Image( 20, 30, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		clear( u );
		clear( v );
		clear( px );
		clear( py );
		warp( u, v, px, py, pyr[ 4 ][ pyridx2 ], pyr[ 4 ][ pyridx ], NUMWARP );

		for( int level = 3; level >= 0; level-- ) {
			size_t w = pyr[ level ][ 0 ]->width();
			size_t h = pyr[ level ][ 0 ]->height();
			Image* unew = biup( u, 2.0f );
			Image* vnew = biup( v, 2.0f );
			Image* pxnew = biup( px, 1.0f );
			Image* pynew = biup( py, 1.0f );
			delete u;
			delete v;
			delete px;
			delete py;
			u = unew;
			v = vnew;
			px = pxnew;
			py = pynew;
			warp( u, v, px, py, pyr[ level ][ pyridx2 ], pyr[ level ][ pyridx ], NUMWARP );
		}

		showColorCode( "Flow", u, pyr[ 0 ][ pyridx2 ] );
/*		{
			Image tmp( 320, 240, CVT_GRAY, IType::FLOAT );
			pyr[ 1 ][ pyridx ]->readData( tmp.data(), tmp.stride() );
			cvShowImage( "Input", tmp.iplimage() );
		}*/
		delete px;
		delete py;
		delete v;
		return u;
	}

	void CLOptflow::warp( Image* u, Image* v, Image* px, Image* py, Image* img1, Image* img2, size_t iter )
	{
		std::vector<cl::Event> sync;
		cl::Event event;
		size_t wx, wy;
		IFilterScalar mul( 1.0 );

		Image dx1( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image dy1( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image dxy1( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image dx( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image dy( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image dxy( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image iwx( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image iwy( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image iwxy( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image it( img1->width(), img1->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		Image v0( v->width(), v->height(), IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );


		if( img1->width() <= 20  ) {
			wx = wy = 10;
		} else {
			wx = 40;
			wy = 10;
		}

		/* calc gradients */
		kernelgrad.setArg( 0, &dxy );
		kernelgrad.setArg( 1, &dx );
		kernelgrad.setArg( 2, &dy );
		kernelgrad.setArg( 3, img2 );
		kernelgrad.setArg( 4, sizeof( cl_float4 ) * ( wx + 2 ) * ( wy + 2 ) );
		kernelgrad.run( *img2, cl::NDRange( wx, wy ), NULL, &event );

		kernelgradxy.setArg( 0, &dxy1 );
		kernelgradxy.setArg( 1, &dx1 );
		kernelgradxy.setArg( 2, &dy1 );
		kernelgradxy.setArg( 3, img1 );
		kernelgradxy.setArg( 4, sizeof( cl_float4 ) * ( wx + 2 ) * ( wy + 2 ) );
		kernelgradxy.run( *img1, cl::NDRange( wx, wy ), NULL, &event );
		sync.push_back( event );

//		v0.copy( *u );
		while( iter-- ) {

#define MX 10
#define MY 10
			kernelmed.setArg( 0, &v0 );
			kernelmed.setArg( 1, v );
			kernelmed.setArg( 2, sizeof( cl_float4 ) * ( MX + 2 ) * ( MY + 2 ) );
			kernelmed.run( *v, cl::NDRange( MX, MY ), &sync, &event );
			sync[ 0 ] =  event;


/*			showColorCode( "FlowX", &v0, img2 );
			cvWaitKey( 10 );
			cvWaitKey( 10 );
			sleep( 1 );*/

			mul.set( 0.5f );
			/* warp dx */
			kernelwarpsub.setArg( 0, &iwx );
			kernelwarpsub.setArg( 1, &dx );
			kernelwarpsub.setArg( 2, &v0 );
			kernelwarpsub.setArg( 3, &dx1 );
			kernelwarpsub.setArg( 4, &mul );
			kernelwarpsub.run( iwx, cl::NDRange( wx, wy), &sync );

			/* warp dy */
			kernelwarpsub.setArg( 0, &iwy );
			kernelwarpsub.setArg( 1, &dy );
			kernelwarpsub.setArg( 2, &v0 );
			kernelwarpsub.setArg( 3, &dy1 );
			kernelwarpsub.setArg( 4, &mul );
			kernelwarpsub.run( iwy, cl::NDRange( wx, wy), &sync );

			/* warp dxy */
			kernelwarpsub.setArg( 0, &iwxy );
			kernelwarpsub.setArg( 1, &dxy );
			kernelwarpsub.setArg( 2, &v0 );
			kernelwarpsub.setArg( 3, &dxy1 );
			kernelwarpsub.setArg( 4, &mul );
			kernelwarpsub.run( iwxy, cl::NDRange( wx, wy), &sync );

			mul.set( 1.0f );
			/* warpsub dt */
			kernelwarpsub.setArg( 0, &it );
			kernelwarpsub.setArg( 1, img2 );
			kernelwarpsub.setArg( 2, &v0 );
			kernelwarpsub.setArg( 3, img1 );
			kernelwarpsub.setArg( 4, &mul );
			kernelwarpsub.run( it, cl::NDRange( wx, wy), &sync );

			tvl1( u, v, px, py, LAMBDA, THETA, &iwxy, &it, &iwx, &iwy, &v0, NUMROF );
		}
	}

	void CLOptflow::tvl1( Image* u, Image* v, Image* px, Image* py, float lambda, float _theta, Image* ig2, Image* it, Image* ix, Image* iy, Image* v0, size_t iter )
	{
		Image pxt, pyt;
//		IFilterScalar tautheta( TAU / _theta );
		IFilterScalar tautheta( 1.0f / ( 4.0f * _theta + 0.01f ) );
		IFilterScalar theta( _theta );
		IFilterScalar lambdatheta( _theta * lambda );
		std::vector<cl::Event> sync;
		cl::Event event;
		size_t wx, wy;

		pxt.reallocate( *px, IALLOCATOR_CL );
		pyt.reallocate( *py, IALLOCATOR_CL );

		/* Set threshold kernel parameter */
		kernelth.setArg( 0, v );
		kernelth.setArg( 1, u );
		kernelth.setArg( 2, ig2 );
		kernelth.setArg( 3, it );
		kernelth.setArg( 4, ix );
		kernelth.setArg( 5, iy );
		kernelth.setArg( 6, v0 );
		kernelth.setArg( 7, &lambdatheta  );

		if( u->width() <= 20 || u->height() <= 20 ) {
			wx = 10;
			wy = 10;
		} else {
			wx = 20;
			wy = 15;
		}

		/* Set kernel p1 parameter */
		//	kernelp1.setArg( 0, &pxt );
		//	kernelp1.setArg( 1, &pyt );
		//	kernelp1.setArg( 2, &px );
		//	kernelp1.setArg( 3, &py );
		kernelp1.setArg( 4, u );
		kernelp1.setArg( 5, &tautheta );
		kernelp1.setArg( 6, sizeof( cl_float4 ) * ( wx + 1 ) * ( wy + 1 ) );

		/* Set kernel p2 parameter */
		kernelp2.setArg( 0, u );
		kernelp2.setArg( 1, v );
		kernelp2.setArg( 4, &theta );
		kernelp2.setArg( 5, sizeof( cl_float4 ) * ( wx + 1 ) * ( wy + 1 ) );
		kernelp2.setArg( 6, sizeof( cl_float4 ) * ( wx + 1 ) * ( wy + 1 ) );


		while( iter-- ) {
			kernelth.run( *ig2, cl::NDRange( 10, 10 ), &sync, &event );
			sync.clear();
			sync.push_back( event );

			kernelp1.setArg( 0, &pxt );
			kernelp1.setArg( 1, &pyt );
			kernelp1.setArg( 2, px );
			kernelp1.setArg( 3, py );

			kernelp2.setArg( 2, &pxt );
			kernelp2.setArg( 3, &pyt );

			kernelp1.run( *u, cl::NDRange( wx, wy ), &sync, &event );
			sync[ 0 ] = event;
			kernelp2.run( *u, cl::NDRange( wx, wy ), &sync, &event );
			sync[ 0 ] = event;
			kernelth.run( *ig2, cl::NDRange( 10, 10 ), &sync, &event );
			sync[ 0 ] = event;

			kernelp1.setArg( 0, px );
			kernelp1.setArg( 1, py );
			kernelp1.setArg( 2, &pxt );
			kernelp1.setArg( 3, &pyt );

			kernelp2.setArg( 2, px );
			kernelp2.setArg( 3, py );

			kernelp1.run( *u, cl::NDRange( wx, wy ), &sync, &event );
			sync[ 0 ] = event;
			kernelp2.run( *u, cl::NDRange( wx, wy ), &sync, &event );
			sync[ 0 ] = event;
		}
		kernelth.run( *ig2, cl::NDRange( 10, 10 ), &sync, &event );
	}

	void CLOptflow::clear( Image* i )
	{
		kernelclearimg.setArg( 0, i );
		kernelclearimg.run( *i, ::cl::NDRange( 10, 10) );
	}

	Image* CLOptflow::biup( Image* in, float mul )
	{
		IFilterScalar m( mul );
		Image* ret = new Image( in->width() * 2, in->height() * 2, IOrder::RGBA, IType::FLOAT, IALLOCATOR_CL );
		kernelbiup.setArg( 0, ret );
		kernelbiup.setArg( 1, in );
		kernelbiup.setArg( 2, &m );
		kernelbiup.run( *in, cl::NDRange( 10, 10 ) );
		return ret;
	}

	Image* CLOptflow::colorcode( Image* in, Image* bg )
	{
		Image* ret = new Image( in->width() * 2, in->height(), IOrder::BGRA, IType::UBYTE, IALLOCATOR_CL );
		size_t wx, wy;

		if( in->width() <= 20  ) {
			wx = wy = 10;
		} else {
			wx = 20;
			wy = 15;
		}

		kernelcflow.setArg( 0, ret );
		kernelcflow.setArg( 1, in );
		kernelcflow.setArg( 2, bg );
		kernelcflow.run( *in, cl::NDRange( wx, wy ) );
		return ret;
	}

	void CLOptflow::showColorCode( const char* name, Image* i, Image* bg )
	{
		Image* ret = colorcode( i, bg );
		Image iflow( *ret );
		{
			Image tmp( 512, 480, IOrder::BGRA, IType::UBYTE );
			tmp.copyRect( 0, 0, iflow, 0, 0, 512, 480 );
			ImageIO::savePNG( tmp, "out.png" );
		}
		cvShowImage( name, iflow.iplimage() );
		delete ret;
	}
}