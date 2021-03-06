/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

__kernel void tvl1( __write_only image2d_t eout, __write_only image2d_t uout, __read_only image2d_t uimg, __read_only image2d_t u0img, __read_only image2d_t warp, __read_only image2d_t e1/*, __read_only image2d_t e0*/, const float lambda, const float theta/*, const float t*/, __local float4* buf, __local float4* buf2  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    const sampler_t samplerb = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = lw + 2;
	const int2 base  = ( int2 )( get_group_id( 0 ) * lw, get_group_id( 1 ) * lh );
	const int2 base2 = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
	const float lamdatheta = lambda * theta;
	float4 w, pout;
	float2 dx, dy, u, u0, norm;

	for( int y = ly; y < lh + 2; y += lh ) {
		for( int x = lx; x < lw + 2; x += lw ) {
			// px, py
			buf[ mul24( y, bstride ) + x ] = read_imagef( e1, samplerb, base2 + ( int2 )( x, y ) );// * ( 1.0f + t ) - t * read_imagef( e0, samplerb, base2 + ( int2 )( x, y ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mul24( ( y ) + 1, bstride ) + ( x ) + 1 ] )
#define BUF2( x, y ) ( buf2[ mul24( ( y ), ( lw + 1 ) ) + ( x ) ] )

	for( int y = ly; y < lh + 1; y += lh ) {
		for( int x = lx; x < lw + 1; x += lw ) {
			// get I_t, I_x, I_y
			w = read_imagef( warp, sampler, base + ( int2 )( x, y ) );
			u = read_imagef( uimg, sampler, base + ( int2 )( x, y ) ).xy;
			u0 = read_imagef( u0img, sampler, base + ( int2 )( x, y ) ).xy;

			/* thresholding scheme */
			/* I_t + \nabla I \cdot u*/
			float dt = w.x + dot( w.yz, u - u0 );
			float ltg2 = lamdatheta * dot( w.yz, w.yz );
			if( dt < - ltg2 )
				u = u + lamdatheta * w.yz;
			else if( dt > ltg2 )
				u = u - lamdatheta * w.yz;
			else
				u = u - ( dt / fmax( dot( w.yz, w.yz ), 1e-4f ) ) * w.yz;

			dx = BUF( x, y ).xz - BUF( x - 1, y ).xz;
			dy = BUF( x, y ).yw - BUF( x, y - 1 ).yw;
			// image + theta * div( p )
			BUF2( x, y ).xy = u + theta * ( ( float2 ) ( dx.x + dy.x, dx.y + dy.y ) );
			//BUF2( x, y ).xy = BUF2(x,y).xy*2-u;
			BUF2( x, y ).w  = w.w;
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( gx >= get_image_width( eout ) || gy >= get_image_height( eout ) )
		return;

	w = BUF2( lx, ly );
	dx = BUF2( lx + 1, ly ).xy - w.xy;
	dy = BUF2( lx, ly + 1 ).xy - w.xy;

	// HUBER
#define EPS 0.04f
	float4 delta = ( float4 ) ( dx.x, dy.x, dx.y, dy.y );
	pout = BUF( lx, ly ) + ( 1.0f / ( 8.0f * theta ) ) * ( delta - EPS * BUF( lx, ly) );
	//norm = fmax( ( float2 ) 1.0f, ( float2 ) ( length( pout.xy )/ w.w, length( pout.zw ) ) / w.w );
//	float n = fmax(  1.0f,  fmax( fast_length( pout.xy ), fast_length( pout.zw ) ) / w.w );

	float n = fmax(  1.0f,  fast_length( pout ) / w.w );
	pout = pout / ( float4 ) ( n );

	write_imagef( eout, ( int2 ) ( gx, gy ), pout );
	write_imagef( uout, ( int2 ) ( gx, gy ), ( float4 ) ( w.x, w.y, 0.0f, 0.0f ) );
}
