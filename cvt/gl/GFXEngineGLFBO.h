#ifndef CVT_GFXENGINEGLFBO_H
#define CVT_GFXENGINEGLFBO_H

#include <cvt/gl/GFXEngineGL.h>

namespace cvt {
	class GFXEngineGLFBO : public GFXEngineGL
	{
		public:
			GFXEngineGLFBO( GLFBO& fbo, GLContext* ctx = NULL );
			~GFXEngineGLFBO();

			void begin();
			void end();

		private:
			GLFBO& _fbo;
	};

	inline GFXEngineGLFBO::GFXEngineGLFBO( GLFBO& fbo, GLContext* ctx ) : GFXEngineGL( ctx?ctx:GLContext::currentContext() ), _fbo( fbo )
	{
		_fliph = true;
	}

	inline GFXEngineGLFBO::~GFXEngineGLFBO()
	{
	}

	void GFXEngineGLFBO::begin()
	{
		_ctx->makeCurrent();
		glEnable( GL_BLEND );
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_POINT_SPRITE );
		glEnable( GL_SCISSOR_TEST );
		glDisable( GL_DEPTH_TEST );
		glEnable( GL_LINE_SMOOTH );
//		if( _ctx->format().samples() )
//			glEnable( GL_MULTISAMPLE );
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_fbo.bind();
		setViewport( Recti( 0, 0, _fbo.width(), _fbo.height() ) );
		setChildrect( Recti( 0, 0, _fbo.width(), _fbo.height() ) );
	}

	void GFXEngineGLFBO::end()
	{
		_fbo.unbind();
	}
}

#endif