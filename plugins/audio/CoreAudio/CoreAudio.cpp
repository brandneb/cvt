#include "CoreAudio.h"
/*
			CVT - Computer Vision Tools Library

 	 Copyright (c) 2012, Philipp Heise, Sebastian Klose

 	THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 	PARTICULAR PURPOSE.
 */
#include <cvt/util/PluginManager.h>

static void _init( cvt::PluginManager* pm )
{
	cvt::AudioPlugin* coreaudio = new cvt::CoreAudio();
	pm->registerPlugin( coreaudio );
}

CVT_PLUGIN( _init )
