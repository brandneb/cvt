#include <cvt/gui/Window.h>
#include <cvt/gui/ImageView.h>
#include <cvt/gui/Moveable.h>
#include <cvt/gui/TimeoutHandler.h>
#include <cvt/gui/Button.h>
#include <cvt/io/RGBDParser.h>
#include <VOKeyframe.h>
#include <RGBDAlignment.h>
#include <GLSceneView.h>

#include <vector>

namespace cvt
{
	class RGBDVOApp : public TimeoutHandler
	{
            public:
                RGBDVOApp( const String& folder, const Matrix3f& K );
                ~RGBDVOApp();

                void onTimeout();

            private:
                RGBDParser                  _parser;
                std::vector<VOKeyframe*>    _keyframes;
                RGBDAlignment               _aligner;
                Window                      _mainWindow;
                uint32_t                    _timerId;

                ImageView                   _keyframeImage;
                Moveable                    _kfMov;
                ImageView                   _currentImage;
                Moveable                    _imageMov;
                ImageView                   _gxView;
                Moveable                    _gxMov;
                ImageView                   _gyView;
                Moveable                    _gyMov;

                GLSceneView                 _sceneView;

                Button                      _nextButton;
                bool                        _nextPressed;

                // pose relative to the keyframe
                SE3<float>                  _relativePose;
                Matrix4f                    _absolutePose;
                VOKeyframe*                 _activeKeyframe;

                void setupGui();

                void addNewKeyframe( const RGBDParser::RGBDSample& sample, const Matrix4f& kfPose );

                void nextPressed();
	};
}
