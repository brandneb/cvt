#ifndef CVT_KLT_WINDOW_H
#define CVT_KLT_WINDOW_H

#include <cvt/gui/TimeoutHandler.h>
#include <cvt/gui/Window.h>
#include <cvt/gui/ImageView.h>
#include <cvt/gui/Button.h>
#include <cvt/gui/WidgetLayout.h>
#include <cvt/gui/Application.h>

#include <cvt/util/Time.h>
#include <cvt/util/EigenBridge.h>

#include <cvt/math/Translation2D.h>
#include <cvt/math/Sim2.h>
#include <cvt/math/GA2.h>

#include <cvt/vision/KLTTracker.h>
#include <cvt/vision/ImagePyramid.h>

#include <cvt/gfx/GFXEngineImage.h>

namespace cvt {

	class KLTWindow : public TimeoutHandler
	{
		public:
			typedef GA2<float>					PoseType;
			//typedef Sim2<float>				PoseType;
			//typedef Translation2D<float>		PoseType;
			typedef KLTTracker<PoseType, 32>	KLTType;
			typedef KLTType::KLTPType			KLTPType;

			KLTWindow( Image & img );

			~KLTWindow();

			void onTimeout();

			void nextStep() { _step = true; }

			void singleScaleUpdate();
			void multiScaleUpdate();

		private:
			uint32_t				_timerId;
			Window					_window;
			ImageView				_imView;

			KLTType					_klt;
			Time					_kltTime;
			double					_kltTimeSum;
			size_t					_kltIters;

			std::vector<KLTPType*>	_patches;
			Time					_time;
			double					_fps;
			size_t					_iter;
			ImagePyramid			_pyramid;

			Button					_stepButton;
			bool					_step;

			Matrix3f				_groundtruth;

			void initialize();
			void drawRect( Image& img );

			void savePatchAsImage( const String& filename );

	};

}

#endif
