/***************************************************************************
  tag: The SourceWorks  Tue Sep 7 00:55:18 CEST 2010  PluginLoader.cpp

                        PluginLoader.cpp -  description
                           -------------------
    begin                : Tue September 07 2010
    copyright            : (C) 2010 The SourceWorks
    email                : peter@thesourceworks.com

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/


/**
 * PluginLoader.cpp
 *
 *  Created on: May 19, 2010
 *      Author: kaltan
 */

#include "PluginLoader.hpp"
#include "../TaskContext.hpp"
#include "../Logger.hpp"
#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#include "../os/StartStopManager.hpp"
#include "../os/MutexLock.hpp"
#include "../internal/GlobalService.hpp"

#include <cstdlib>
#include <dlfcn.h>


using namespace RTT;
using namespace RTT::detail;
using namespace plugin;
using namespace std;
using namespace boost::filesystem;

// chose the file extension applicable to the O/S
#ifdef  __APPLE__
static const std::string SO_EXT(".dylib");
#else
# ifdef _WIN32
static const std::string SO_EXT(".dll");
# else
static const std::string SO_EXT(".so");
# endif
#endif

// choose how the PATH looks like
# ifdef _WIN32
static const std::string delimiters(";");
static const std::string default_delimiter(";");
# else
static const std::string delimiters(":;");
static const std::string default_delimiter(":");
# endif

namespace RTT { namespace plugin {
    extern std::string default_plugin_path;
}}

namespace {
    /**
     * Reads the RTT_COMPONENT_PATH and inits the PluginLoader.
     */
    int loadPlugins()
    {
        char* paths = getenv("RTT_COMPONENT_PATH");
        string plugin_paths;
        if (paths) {
            plugin_paths = paths;
            // prepend the default search path.
            if ( !default_plugin_path.empty() )
                plugin_paths = plugin_paths + default_delimiter + default_plugin_path;
            log(Info) <<"RTT_COMPONENT_PATH was set to: " << paths << " . Searching in: "<< plugin_paths<< endlog();
        } else {
            plugin_paths = default_plugin_path;
            log(Info) <<"No RTT_COMPONENT_PATH set. Using default: " << plugin_paths <<endlog();
        }
        // we set the plugin path such that we can search for sub-directories/projects lateron
        PluginLoader::Instance()->setPluginPath(plugin_paths);
        // we load the plugins/typekits which are in each plugin path directory (but not subdirectories).
        try {
            PluginLoader::Instance()->loadPlugin("rtt", plugin_paths);
            PluginLoader::Instance()->loadTypekit("rtt", plugin_paths);
        } catch(std::exception& e) {
            log(Warning) << e.what() <<endlog();
            log(Warning) << "Corrupted files found in '" << plugin_paths << "'. Fix or remove these plugins."<<endlog();
        }
        return 0;
    }

    os::InitFunction plugin_loader( &loadPlugins );

    void unloadPlugins()
    {
        PluginLoader::Release();
    }

    os::CleanupFunction plugin_unloader( &unloadPlugins );
}

boost::shared_ptr<PluginLoader> PluginLoader::minstance;

boost::shared_ptr<PluginLoader> instance2;

namespace {

vector<string> splitPaths(string const& str)
{
    vector<string> paths;

    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        if ( !str.substr(lastPos, pos - lastPos).empty() )
            paths.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
    if ( paths.empty() )
        paths.push_back(".");
    return paths;
}

/**
 * Strips the 'lib' prefix and '.so'/'.dll'/... suffix (ie SO_EXT) from a filename.
 * Do not provide paths, only filenames, for example: "libplugin.so"
 * @param str filename.
 * @return stripped filename.
 */
string makeShortFilename(string const& str) {
    string ret = str;
    if (str.substr(0,3) == "lib")
        ret = str.substr(3);
    if (ret.rfind(SO_EXT) != string::npos)
        ret = ret.substr(0, ret.rfind(SO_EXT) );
    return ret;
}

}

PluginLoader::PluginLoader() { log(Debug) <<"PluginLoader Created" <<endlog(); }
PluginLoader::~PluginLoader(){ log(Debug) <<"PluginLoader Destroyed" <<endlog(); }


boost::shared_ptr<PluginLoader> PluginLoader::Instance() {
    if (!instance2) {
        instance2.reset( new PluginLoader() );
    }
    return instance2;
}

void PluginLoader::Release() {
    instance2.reset();
}

bool PluginLoader::loadTypekits(string const& path_list) {
    MutexLock lock( listlock );
    return loadPluginsInternal( path_list, "types", "typekit");
}

bool PluginLoader::loadTypekit(std::string const& name, std::string const& path_list) {
    MutexLock lock( listlock );
    return loadPluginInternal(name, path_list, "types", "typekit");
}

bool PluginLoader::loadPlugin(std::string const& name, std::string const& path_list) {
    MutexLock lock( listlock );
    return loadPluginInternal(name, path_list, "plugins", "plugin");
}

bool PluginLoader::loadPlugins(string const& path_list) {
    MutexLock lock( listlock );
    return loadPluginsInternal( path_list, "plugins", "plugin");
}

bool PluginLoader::loadService(string const& servicename, TaskContext* tc) {
    MutexLock lock( listlock );
    for(vector<LoadedLib>::iterator it= loadedLibs.begin(); it != loadedLibs.end(); ++it) {
        if (it->filename == servicename || it->plugname == servicename || it->shortname == servicename) {
            if (tc) {
                log(Info) << "Loading Service or Plugin " << servicename << " in TaskContext " << tc->getName() <<endlog();
                try {
                    return it->loadPlugin( tc );
                } catch(std::exception& e) {
                    log(Error) << "Service or Plugin "<< servicename <<" threw an exception during loading in " << tc->getName() << endlog();
                    log(Error) << "Exception: "<< e.what() << endlog();
                    return false;
                } catch(...) {
                    log(Error) << "Service or Plugin "<< servicename <<" threw an unknown exception during loading in " << tc->getName() << endlog();
                    return false;
                }
            } else {
                // loadPlugin( 0 ) was already called. So drop the service in the global service.
                if (it->is_service)
                    try {
                        return internal::GlobalService::Instance()->addService( it->createService()  );
                    } catch(std::exception& e) {
                        log(Error) << "Service "<< servicename <<" threw an exception during loading in global service." << endlog();
                        log(Error) << "Exception: "<< e.what() << endlog();
                        return false;
                    } catch(...) {
                        log(Error) << "Service "<< servicename <<" threw an unknown exception during loading in global service. " << endlog();
                        return false;
                    }
                log(Error) << "Plugin "<< servicename << " was found, but it's not a Service." <<endlog();
            }
        }
    }
    log(Error) << "No such service or plugin: '"<< servicename << "'"<< endlog();
    return false;
}

// This is a DUMB function and does not scan subdirs, possible filenames etc.
bool PluginLoader::loadPluginsInternal( std::string const& path_list, std::string const& subdir, std::string const& kind )
{
	// If exact match, load it directly:
    path arg( path_list );
    if (is_regular_file(arg)) {
#if BOOST_VERSION >= 104600
        if ( loadInProcess(arg.string(), makeShortFilename(arg.filename().string()), kind, true) == false)
#else
        if ( loadInProcess(arg.string(), makeShortFilename(arg.filename()), kind, true) == false)
#endif
            throw std::runtime_error("The plugin "+path_list+" was found but could not be loaded !");
        log(Warning) << "You supplied a filename to 'loadPlugins(path)' or 'loadTypekits(path)'."<< nlog();
        log(Warning) << "Please use 'loadLibrary(filename)' instead since the function you use will only scan directories in future releases."<<endlog();
        return true;
    }

    // prepare search path:
    vector<string> paths;
    if (path_list.empty())
    	return false; // nop: if no path is given, nothing to be searched.
    else
    	paths = splitPaths( path_list );

    bool all_good = true, found = false;
    // perform search in paths:
    for (vector<string>::iterator it = paths.begin(); it != paths.end(); ++it)
    {
        // Scan path/types/* (non recursive)
        path p = path(*it) / subdir;
        if (is_directory(p))
        {
            log(Info) << "Loading "<<kind<<" libraries from directory " << p.string() << " ..."<<endlog();
            for (directory_iterator itr(p); itr != directory_iterator(); ++itr)
            {
                log(Debug) << "Scanning file " << itr->path().string() << " ...";
                if (is_regular_file(itr->status()) && itr->path().extension() == SO_EXT ) {
                    found = true;
#if BOOST_VERSION >= 104600
                    all_good = loadInProcess( itr->path().string(), makeShortFilename(itr->path().filename().string() ), kind, true) && all_good;
#else
                    all_good = loadInProcess( itr->path().string(), makeShortFilename(itr->path().filename() ), kind, true) && all_good;
#endif
                } else {
                    if (!is_regular_file(itr->status()))
                        log(Debug) << "not a regular file: ignored."<<endlog();
                    else
                        log(Debug) << "not a " + SO_EXT + " library: ignored."<<endlog();
                }
            }
        }
        else
            log(Debug) << "No such directory: " << p << endlog();
    }
    if (!all_good)
        throw std::runtime_error("Some found plugins could not be loaded !");
    return found;
}

bool PluginLoader::loadLibrary( std::string const& name )
{
    // If exact match, load it directly:
    path arg( name );
    if (is_regular_file(arg)) {
#if BOOST_VERSION >= 104600
        string subdir = arg.remove_filename().filename().string();
#else
        string subdir = arg.remove_filename().leaf();
#endif
        string kind;
        // we only load it if it is in types or plugins subdir:
        if (subdir == "types") kind = "typekit";
        if (subdir == "plugins") kind = "plugin";
        if ( !kind.empty() ) {
#if BOOST_VERSION >= 104600
            if ( loadInProcess(arg.string(), makeShortFilename(arg.filename().string()), kind, true) == false)
#else
            if ( loadInProcess(arg.string(), makeShortFilename(arg.filename()), kind, true) == false)
#endif
                throw std::runtime_error("The plugin "+name+" was found but could not be loaded !");
            return true;
        }
        // file exists but not typekit or plugin:
        return false;
    }

    // bail out if absolute path
    if ( arg.is_complete() )
        return false;

    // try relative match:
    path dir = arg.parent_path();
#if BOOST_VERSION >= 104600
    string file = arg.filename().string();
#else
    string file = arg.filename();
#endif
    vector<string> paths = splitPaths(plugin_path);
    vector<string> tryouts( paths.size() * 8 );
    tryouts.clear();

    // search in plugins/types:
    string subdir = "plugins"; string kind = "plugin";
    while (true) {
        for (vector<string>::iterator it = paths.begin(); it != paths.end(); ++it)
        {
            path p = path(*it) / dir / subdir / (file + SO_EXT);
            tryouts.push_back( p.string() );
            if (is_regular_file( p ) && loadInProcess( p.string(), name, kind, true ) )
                return true;
            p = path(*it) / dir / subdir / ("lib" + file + SO_EXT);
            tryouts.push_back( p.string() );
            if (is_regular_file( p ) && loadInProcess( p.string(), name, kind, true ) )
                return true;
            p = path(*it) / OROCOS_TARGET_NAME / dir / subdir / (file + SO_EXT);
            tryouts.push_back( p.string() );
            if (is_regular_file( p ) && loadInProcess( p.string(), name, kind, true ) )
                return true;
            p = path(*it) / OROCOS_TARGET_NAME / dir / subdir / ("lib" + file + SO_EXT);
            tryouts.push_back( p.string() );
            if (is_regular_file( p ) && loadInProcess( p.string(), name, kind, true ) )
                return true;
        }
        if (subdir == "types")
            break;
        subdir = "types";
        kind   = "typekit";
    }
    log(Debug) << "No such "<< kind << " found in path: " << name << ". Tried:"<< endlog();
    for(vector<string>::iterator it=tryouts.begin(); it != tryouts.end(); ++it)
        log(Debug) << *it << endlog();

    return false;
}

bool PluginLoader::loadPluginInternal( std::string const& name, std::string const& path_list, std::string const& subdir, std::string const& kind )
{
	// If exact match, load it directly:
    // special case for ourselves, rtt plugins are not in an 'rtt' subdir:
    if (name != "rtt" && loadLibrary(name)) {
        log(Warning) << "You supplied a filename as first argument to 'loadPlugin(name,path)' or 'loadTypekit(name,path)'."<<nlog();
        log(Warning) << "Please use 'loadLibrary(filename)' instead since the function you use will only interprete 'name' as a directory name in future releases."<<endlog();
        return true;
    }

    if ( isLoadedInternal(name) ) {
        log(Debug) <<kind << " '"<< name <<"' already loaded. Not reloading it." <<endlog();
        return true;
    } else {
        log(Info) << kind << " '"<< name <<"' not loaded before." <<endlog();
    }

    string paths, trypaths;
    if (path_list.empty())
    	paths = plugin_path + default_delimiter + ".";
    else
    	paths = path_list;

    // search in '.' if really no paths are given.
    if (paths.empty())
        paths = ".";

    // append '/name' to each plugin path in order to search all of them:
    vector<string> vpaths = splitPaths(paths);
    paths.clear();
    bool path_found = false;
    string plugin_dir = name;
    if (name == "rtt" ) // special case for ourselves, rtt plugins are not in an 'rtt' subdir:
        plugin_dir = ".";
    for(vector<string>::iterator it = vpaths.begin(); it != vpaths.end(); ++it) {
        path p(*it);
        p = p / plugin_dir;
        // we only search in existing directories:
        if (is_directory( p )) {
            path_found = true;
            paths += p.string() + default_delimiter;
        } else {
            trypaths += p.string() + default_delimiter;
        }
        p = *it;
        p = p / OROCOS_TARGET_NAME / plugin_dir;
        // we only search in existing directories:
        if (is_directory( p )) {
            path_found = true;
            paths += p.string() + default_delimiter;
        } else {
            trypaths += p.string() + default_delimiter;
        }
    }

    // when at least one directory exists:
    if (path_found) {
        paths.erase( paths.size() - 1 ); // remove trailing delimiter ';'
        return loadPluginsInternal(paths,subdir,kind);
    }
    log(Error) << "No such "<< kind << " found in path: " << name << ". Looked for these directories: "<< endlog();
    if ( !paths.empty() )
        log(Error) << "Exist, but don't contain it: " << paths << endlog();
    else
        log(Error) << "None of the search paths exist !" << endlog();
    if ( !trypaths.empty() )
        log(Error) << "Don't exist: " << trypaths << endlog();
    return false;
}

bool PluginLoader::isLoaded(string file)
{
    MutexLock lock( listlock );
    return isLoadedInternal(file);
}

bool PluginLoader::isLoadedInternal(string file)
{
    path p(file);
    std::vector<LoadedLib>::iterator lib = loadedLibs.begin();
    while (lib != loadedLibs.end()) {
        // there is already a library with the same name
        if ( lib->filename == p.filename() || lib->plugname == file || lib->shortname == file ) {
            return true;
        }
        ++lib;
    }
    return false;
}

// loads a single plugin in the current process.
bool PluginLoader::loadInProcess(string file, string shortname, string kind, bool log_error) {
    path p(file);
    char* error;
    void* handle;

    if ( isLoadedInternal(shortname) || isLoadedInternal(file) ) {
        log(Debug) <<"plugin '"<< file <<"' already loaded. Not reloading it." <<endlog() ;
        return true;
    }

    handle = dlopen ( p.string().c_str(), RTLD_NOW | RTLD_GLOBAL );

    if (!handle) {
        string e( dlerror() );
        if (log_error)
            log(Error) << "could not load library '"<< p.string() <<"': "<< e <<endlog();
        else
            endlog();
        return false;
    }

    //------------- if you get here, the library has been loaded -------------
#if BOOST_VERSION >= 104600
    string libname = p.filename().string();
#else
    string libname = p.filename();
#endif
    log(Debug)<<"Found library "<<libname<<endlog();
    LoadedLib loading_lib(libname,shortname,handle);
    dlerror();    /* Clear any existing error */

    std::string(*pluginName)(void) = 0;
    std::string(*targetName)(void) = 0;
    loading_lib.loadPlugin = (bool(*)(RTT::TaskContext*))(dlsym(handle, "loadRTTPlugin") );
    if ((error = dlerror()) == NULL) {
        string plugname, targetname;
        pluginName = (std::string(*)(void))(dlsym(handle, "getRTTPluginName") );
        if ((error = dlerror()) == NULL) {
            plugname = (*pluginName)();
        } else {
            plugname  = libname;
        }
        loading_lib.plugname = plugname;
        targetName = (std::string(*)(void))(dlsym(handle, "getRTTTargetName") );
        if ((error = dlerror()) == NULL) {
            targetname = (*targetName)();
        } else {
            targetname  = OROCOS_TARGET_NAME;
        }
        if ( targetname != OROCOS_TARGET_NAME ) {
            log(Error) << "Plugin "<< plugname <<" reports to be compiled for OROCOS_TARGET "<< targetname
                    << " while we are running on target "<< OROCOS_TARGET_NAME <<". Unloading."<<endlog();
            dlclose(handle);
            return false;
        }

        // check if it is a service plugin:
        loading_lib.createService = (Service::shared_ptr(*)(void))(dlsym(handle, "createService") );
        if (loading_lib.createService)
            loading_lib.is_service = true;

        // ok; try to load it.
        bool success = false;
        try {
            // Load into process (TaskContext* == 0):
            success = (*loading_lib.loadPlugin)( 0 );
        } catch(...) {
            log(Error) << "Unexpected exception in loadRTTPlugin !"<<endlog();
        }

        if ( !success ) {
            log(Error) << "Failed to load RTT Plugin '" <<plugname<<"': plugin refused to load into this process. Unloading." <<endlog();
            dlclose(handle);
            return false;
        }
        if (kind == "typekit") {
            log(Info) << "Loaded RTT TypeKit/Transport '" + plugname + "' from '" + shortname +"'"<<endlog();
            loading_lib.is_typekit = true;
        } else {
            loading_lib.is_typekit = false;
            if ( loading_lib.is_service ) {
                log(Info) << "Loaded RTT Service '" + plugname + "' from '" + shortname +"'"<<endlog();
            }
            else {
                log(Info) << "Loaded RTT Plugin '" + plugname + "' from '" + shortname +"'"<<endlog();
            }
        }
        loadedLibs.push_back(loading_lib);
        return true;
    } else {
        if (log_error)
            log(Error) <<"Not a plugin: " << error << endlog();
    }
    dlclose(handle);
    return false;
}

std::vector<std::string> PluginLoader::listServices() const {
    MutexLock lock( listlock );
    vector<string> names;
    for(vector<LoadedLib>::const_iterator it= loadedLibs.begin(); it != loadedLibs.end(); ++it) {
        if ( it->is_service )
            names.push_back( it->plugname );
    }
    return names;
}

std::vector<std::string> PluginLoader::listPlugins() const {
    MutexLock lock( listlock );
    vector<string> names;
    for(vector<LoadedLib>::const_iterator it= loadedLibs.begin(); it != loadedLibs.end(); ++it) {
        names.push_back( it->plugname );
    }
    return names;
}

std::vector<std::string> PluginLoader::listTypekits() const {
    MutexLock lock( listlock );
    vector<string> names;
    for(vector<LoadedLib>::const_iterator it= loadedLibs.begin(); it != loadedLibs.end(); ++it) {
        if ( it->is_typekit )
            names.push_back( it->plugname );
    }
    return names;
}

std::string PluginLoader::getPluginPath() const {
    MutexLock lock( listlock );
    return plugin_path;
}

void PluginLoader::setPluginPath( std::string const& newpath ) {
    MutexLock lock( listlock );
    plugin_path = newpath;
}
