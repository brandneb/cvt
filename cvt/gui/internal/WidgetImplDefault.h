#ifndef CVT_WIDGETIMPLDEFAULT_H
#define CVT_WIDGETIMPLDEFAULT_H

#include <cvt/gui/internal/WidgetImpl.h>
#include <cvt/math/Vector.h>


namespace cvt {
	class WidgetImplDefault : public WidgetImpl
	{
		public:
			WidgetImplDefault( Widget* w );
			~WidgetImplDefault();

			virtual void	    setSize( int width, int height );
			virtual void	    size( int& width, int& height ) const;
			virtual void	    setPosition( int width, int height );
			virtual void	    position( int& width, int& height ) const;
			virtual void	    setRect( const Recti& rect );
			virtual void	    rect( Recti& rect ) const;
			virtual void	    setVisible( bool visibility );
			virtual bool	    isVisible() const;
			virtual void	    update();
			virtual void	    update( const Recti& rect );
			virtual void	    setMinimumSize( int w, int h );
			virtual void	    setMaximumSize( int w, int h );
			virtual void	    minimumSize( int& w, int& h );
			virtual void	    maximumSize( int& w, int& h );
			virtual void	    setParent( Widget* w );
			virtual Widget*	    parent() const;

		private:
			WidgetImplDefault( const WidgetImpl& wi );

			Widget*	 _widget;
			Widget*	 _parent;
			Recti	 _rect;
			bool	 _visible;
			Vector2<int> _minSize;
			Vector2<int> _maxSize;
	};


	inline WidgetImplDefault::WidgetImplDefault( Widget* w ) : _widget( w )
	{
	}

	inline void WidgetImplDefault::setSize( int width, int height )
	{
		if( _rect.width == width && _rect.height == height )
			return;

		width  = Math::max( width, 0 );
		height = Math::max( height, 0 );
		ResizeEvent re( width, height, _rect.width, _rect.height );
		_rect.width  = width;
		_rect.height = height;
		_widget->resizeEvent( &re );
	}

	inline void WidgetImplDefault::size( int& width, int& height ) const
	{
		width  = _rect.width;
		height = _rect.height;
	}

	inline void WidgetImplDefault::setPosition( int x, int y )
	{
		if( _rect.x == x && _rect.y == y )
			return;
		MoveEvent me( x, y, _rect.x, _rect.y );
		_rect.x  = x;
		_rect.y  = y;
		_widget->moveEvent( &me );
	}

	inline void WidgetImplDefault::position( int& x, int& y ) const
	{
		x  = _rect.x;
		y  = _rect.y;
	}

	inline void WidgetImplDefault::setRect( const Recti& rect )
	{
		setPosition( rect.x, rect.y );
		setSize( rect.width, rect.height );
	}

	inline void WidgetImplDefault::rect( Recti& rect ) const
	{
		rect = _rect;
	}

	inline void WidgetImplDefault::setVisible( bool visibility )
	{
		if( _visible == visibility )
			return;
		_visible = visibility;
		if( _visible ) {
			ShowEvent se;
			_widget->showEvent( &se );
		} else {
			HideEvent he;
			_widget->hideEvent( &he );
		}
	}

	inline bool WidgetImplDefault::isVisible() const
	{
		return _visible;
	}

	inline void WidgetImplDefault::update()
	{
		if( _parent )
			_parent->update( _rect );
	}

	inline void WidgetImplDefault::update( const Recti& rect )
	{
		if( _parent ) {
			Recti r = _rect;
			r.intersect( rect );
			if( !r.isEmpty() )
				_parent->update( r );
		}
	}

	inline void WidgetImplDefault::setMinimumSize( int w, int h )
	{
		_minSize.set( w, h );
	}

	inline void WidgetImplDefault::setMaximumSize( int w, int h )
	{
		_maxSize.set( w, h );
	}

	inline void WidgetImplDefault::minimumSize( int& w, int& h )
	{
		w = _minSize[ 0 ];
		h = _minSize[ 1 ];
	}

	inline void WidgetImplDefault::maximumSize( int& w, int& h )
	{
		w = _maxSize[ 0 ];
		h = _maxSize[ 1 ];
	}

	inline void WidgetImplDefault::setParent( Widget* w )
	{
		_parent = w;
	}

	inline Widget* WidgetImplDefault::parent() const
	{
		return _parent;
	}

}
#endif
