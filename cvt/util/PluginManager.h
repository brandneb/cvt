/*
			CVT - Computer Vision Tools Library

 	 Copyright (c) 2012, Philipp Heise, Sebastian Klose

 	THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 	PARTICULAR PURPOSE.
 */
#ifndef CVT_PLUGINMANAGER_H
#define CVT_PLUGINMANAGER_H

#include <cvt/util/Plugin.h>
#include <cvt/util/PluginFile.h>
#include <cvt/util/String.h>
#include <vector>
#include <map>

#include <cvt/gfx/IFilter.h>
#include <cvt/gfx/ILoader.h>
#include <cvt/gfx/ISaver.h>
#include <cvt/geom/scene/SceneLoader.h>
#include <cvt/io/audio/AudioPlugin.h>

namespace cvt {
	class Application;

	class PluginManager {
		friend class Application;
		public:
			static PluginManager& instance();

			void		 registerPlugin( Plugin* plugin );
			void		 loadPlugin( const String& path );

			size_t		 getIFilterSize() const;
			IFilter*	 getIFilter( size_t i ) const;
			IFilter*	 getIFilter( const String& name ) const;

			ILoader*	 getILoaderForFilename( const String& name ) const;
			ISaver*		 getISaverForFilename( const String& name ) const;
			SceneLoader* getSceneLoaderForFilename( const String& name ) const;

			void		 getAudioDevices( std::vector<AudioDevice*>& devices ) const;

		private:
			PluginManager();
			PluginManager( const PluginManager& );
			~PluginManager();

			void						loadDefault();
			static void					cleanup();

			std::vector<PluginFile*>	_plugins;
			std::vector<IFilter*>		_ifilters;
			std::vector<ILoader*>		_iloaders;
			std::vector<ISaver*>		_isavers;
			std::vector<SceneLoader*>	_sceneloaders;
			std::vector<AudioPlugin*>	_audio;

			std::vector<String>			_pluginPaths;

			static PluginManager* _instance;
	};


	inline PluginManager::~PluginManager()
	{
		for( std::vector<IFilter*>::iterator it = _ifilters.begin(), end = _ifilters.end(); it != end; ++it  )
			delete *it;
		_ifilters.clear();

		for( std::vector<ILoader*>::iterator it = _iloaders.begin(), end = _iloaders.end(); it != end; ++it  )
			delete *it;
		_iloaders.clear();

		for( std::vector<ISaver*>::iterator it = _isavers.begin(), end = _isavers.end(); it != end; ++it  )
			delete *it;
		_isavers.clear();

		for( std::vector<SceneLoader*>::iterator it = _sceneloaders.begin(), end = _sceneloaders.end(); it != end; ++it  )
			delete *it;
		_sceneloaders.clear();

		for( std::vector<AudioPlugin*>::iterator it = _audio.begin(), end = _audio.end(); it != end; ++it  )
			delete *it;
		_audio.clear();


		for( std::vector<PluginFile*>::iterator it = _plugins.begin(), end = _plugins.end(); it != end; ++it  )
			delete *it;
		_plugins.clear();
	}

	inline void PluginManager::registerPlugin( Plugin* plugin )
	{
		if( !plugin )
			return;
		switch( plugin->pluginType() ) {
			case PLUGIN_IFILTER:
				{
					_ifilters.push_back( ( IFilter* ) plugin );
				}
				break;
			case PLUGIN_ILOADER:
				{
					_iloaders.push_back( ( ILoader* ) plugin );
				}
				break;
			case PLUGIN_ISAVER:
				{
					_isavers.push_back( ( ISaver* ) plugin );
				}
				break;
			case PLUGIN_SCENELOADER:
				{
					_sceneloaders.push_back( ( SceneLoader* ) plugin );
				}
				break;
			case PLUGIN_AUDIO:
				{
					_audio.push_back( ( AudioPlugin* ) plugin );
				}
				break;
			default:
				break;
		}
	}

	inline void PluginManager::loadPlugin( const String& path )
	{
		PluginFile* p = new PluginFile( path.c_str() );
		p->load();
		_plugins.push_back( p );
	}


	inline IFilter* PluginManager::getIFilter( size_t n ) const
	{
		return _ifilters[ n ];
	}

	inline IFilter* PluginManager::getIFilter( const String& name ) const
	{
		for( std::vector<IFilter*>::const_iterator it = _ifilters.begin(), end = _ifilters.end(); it != end; ++it  ) {
			if( ( *it )->name() == name )
					return *it;
		}
		return NULL;
	}

	inline size_t PluginManager::getIFilterSize() const
	{
		return _ifilters.size();
	}

	inline ILoader* PluginManager::getILoaderForFilename( const String& name ) const
	{
		for( std::vector<ILoader*>::const_iterator it = _iloaders.begin(), end = _iloaders.end(); it != end; ++it  ) {
			for( size_t i = 0, end = ( *it )->sizeExtensions(); i < end; i++ ) {
				if( name.hasSuffix( ( *it )->extension( i ) ) )
					return *it;
			}
		}
		return NULL;
	}

	inline ISaver* PluginManager::getISaverForFilename( const String& name ) const
	{
		for( std::vector<ISaver*>::const_iterator it = _isavers.begin(), end = _isavers.end(); it != end; ++it  ) {
			for( size_t i = 0, end = ( *it )->sizeExtensions(); i < end; i++ ) {
				if( name.hasSuffix( ( *it )->extension( i ) ) )
					return *it;
			}
		}
		return NULL;
	}

	inline SceneLoader* PluginManager::getSceneLoaderForFilename( const String& name ) const
	{
		for( std::vector<SceneLoader*>::const_iterator it = _sceneloaders.begin(), end = _sceneloaders.end(); it != end; ++it  ) {
			for( size_t i = 0, end = ( *it )->sizeExtensions(); i < end; i++ ) {
				if( name.hasSuffix( ( *it )->extension( i ) ) )
					return *it;
			}
		}
		return NULL;
	}


	inline void PluginManager::getAudioDevices( std::vector<AudioDevice*>& devices ) const
	{
		devices.clear();
		for( std::vector<AudioPlugin*>::const_iterator it = _audio.begin(), end = _audio.end(); it != end; ++it  ) {
			( *it )->devices( devices );
		}
	}

}

#endif
