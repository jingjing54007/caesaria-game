// This file is part of openCaesar3.
//
// openCaesar3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openCaesar3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openCaesar3.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com

#include "oc3_application.hpp"
#include "oc3_screen_wait.hpp"
#include "oc3_stringhelper.hpp"
#include "oc3_scenario.hpp"
#include "oc3_city.hpp"
#include "oc3_picture.hpp"
#include "oc3_gfx_sdl_engine.hpp"
#include "oc3_gfx_gl_engine.hpp"
#include "oc3_sound_engine.hpp"
#include "oc3_astarpathfinding.hpp"
#include "oc3_building_data.hpp"
#include "oc3_picture_bank.hpp"
#include "oc3_screen_menu.hpp"
#include "oc3_screen_game.hpp"
#include "oc3_house_level.hpp"
#include "oc3_guienv.hpp"
#include "oc3_app_config.hpp"
#include "oc3_divinity.hpp"
#include "oc3_filesystem.hpp"
#include "oc3_enums.hpp"
#include "oc3_animation_bank.hpp"
#include "oc3_filesystem_filelist.hpp"
#include "oc3_empire.hpp"
#include "oc3_exception.hpp"
#include "oc3_name_generator.hpp"

#include <libintl.h>
#include <list>

#if defined(OC3_PLATFORM_WIN)
  #undef main
#endif

class Application::Impl
{
public:
  ScreenType nextScreen;
  GfxEngine* engine;
  GuiEnv* gui;
  
  void initLocale(const std::string & localePath);
  void initPictures(const io::FilePath& resourcePath);
};

void Application::Impl::initLocale(const std::string & localePath)
{
  // init the internationalization library (gettext)
  setlocale(LC_ALL, "");
  bindtextdomain( "caesar", localePath.data() );
  textdomain( "caesar" );
}

void Application::initVideo()
{
  StringHelper::debug( 0xff, "init graphic engine" );
  _d->engine = new GfxSdlEngine();
   
  /* Typical resolutions:
   * 640 x 480; 800 x 600; 1024 x 768; 1400 x 1050; 1600 x 1200
   */
  GfxEngine::instance().setScreenSize(1024, 768);
  GfxEngine::instance().init();
}

void Application::initSound()
{
  StringHelper::debug( 0xff, "init sound engine" );
  new SoundEngine();
  SoundEngine::instance().init();
}

void Application::mountArchives()
{
  StringHelper::debug( 0xff, "mount archives begin" );

  io::FileSystem& fs = io::FileSystem::instance();

  fs.mountArchive( AppConfig::rcpath( "/pics/pics_wait.zip" ) );
  fs.mountArchive( AppConfig::rcpath( "/pics/pics.zip" ) );
  fs.mountArchive( AppConfig::rcpath( "/pics/pics_oc3.zip" ) );
}

void Application::initGuiEnvironment()
{
  _d->gui = new GuiEnv( *_d->engine );
}

void Application::Impl::initPictures(const io::FilePath& resourcePath)
{
  AnimationBank::loadCarts();
  AnimationBank::loadWalkers();
  
  StringHelper::debug( 0xff, "Load fonts" );
  FontCollection::instance().initialize( resourcePath.toString() );

  StringHelper::debug( 0xff, "Create runtime pictures" );
  PictureBank::instance().createResources();
}

void Application::setScreenWait()
{
   ScreenWait screen;
   screen.initialize( *_d->engine, *_d->gui);
   screen.drawFrame();
}

void Application::setScreenMenu()
{
  ScreenMenu screen;
  screen.initialize( *_d->engine, *_d->gui );

  int result = screen.run();
  Scenario& scenario = Scenario::instance();
  scenario.reset();

  switch( result )
  {
    case ScreenMenu::startNewGame:
    {
      /* temporary*/     
      io::FileList::Items maps = io::FileDir( AppConfig::rcpath( "/maps/" ) ).getEntries().filter( io::FileList::file, "" ).getItems();
      std::srand( DateTime::getElapsedTime() );
      std::string file = maps.at( std::rand() % maps.size() ).fullName.toString();
      StringHelper::debug( 0xff, "Loading map:%s", file.c_str() );
      bool loadok = scenario.load(file);
      _d->nextScreen = loadok ? SCREEN_GAME : SCREEN_MENU;
    }
    break;
   
    case ScreenMenu::loadSavedGame:
    {  
      std::cout<<"Loading map:" << "lepcismagna.sav" << std::endl;
      bool loadok = scenario.load(  AppConfig::rcpath( "/savs/timgad.sav" ).toString() );
      _d->nextScreen = loadok ? SCREEN_GAME : SCREEN_MENU;
    }
    break;

    case ScreenMenu::loadMap:
    {
      bool loadok = scenario.load( screen.getMapName() );
      _d->nextScreen = loadok ? SCREEN_GAME : SCREEN_MENU;
    }
    break;
   
    case ScreenMenu::closeApplication:
    {
      _d->nextScreen = SCREEN_QUIT;
    }
    break;
   
    default:
      _OC3_DEBUG_BREAK_IF( "Unexpected result event" );
   }
}

void Application::setScreenGame()
{
  ScreenGame screen;
  screen.setScenario( Scenario::instance() );
  screen.initialize( *_d->engine, *_d->gui );
  int result = screen.run();

  switch( result )
  {
    case ScreenGame::mainMenu:
      _d->nextScreen = SCREEN_MENU;
    break;

    case ScreenGame::quitGame:
      _d->nextScreen = SCREEN_QUIT;
    break;

    default:
      _d->nextScreen = SCREEN_QUIT;
  }   
}


Application::Application() : _d( new Impl )
{
   _d->nextScreen = SCREEN_NONE;
}

void Application::start()
{
   //Create right PictureBank instance in the beginning   
  StringHelper::redirectCout2( "stdout.log" );
  _d->initLocale(AppConfig::get( AppConfig::localePath ).toString());
  
  initVideo();
  initGuiEnvironment();
  initSound();
  //SoundEngine::instance().play_music("resources/sound/drums.wav");
  mountArchives();  // init some quick pictures for screenWait
  setScreenWait();

  _d->initPictures( AppConfig::rcpath() );
  NameGenerator::initialize( AppConfig::rcpath( AppConfig::ctNamesModel ) );
  HouseSpecHelper::getInstance().initialize( AppConfig::rcpath( AppConfig::houseModel ) );
  DivinePantheon::getInstance().initialize(  AppConfig::rcpath( AppConfig::pantheonModel ) );
  BuildingDataHolder::instance().initialize( AppConfig::rcpath( AppConfig::constructionModel ) );

  _d->nextScreen = SCREEN_MENU;
  _d->engine->setFlag( 0, 1 );

  while(_d->nextScreen != SCREEN_QUIT)
  {
     switch(_d->nextScreen)
     {
     case SCREEN_MENU:
        setScreenMenu();
        break;
     case SCREEN_GAME:
        setScreenGame();
        break;
     default:
        _OC3_DEBUG_BREAK_IF( "Unexpected next screen type" );
        StringHelper::debug( 0xff, "Unexpected next screen type %d", _d->nextScreen );
     }
  }
}

int main(int argc, char* argv[])
{
   for (int i = 0; i < (argc - 1); i++)
   {
     if( !strcmp( argv[i], "-R" ) )
     {
       AppConfig::set( AppConfig::resourcePath, Variant( std::string( argv[i+1] ) ) );
       AppConfig::set( AppConfig::localePath, Variant( std::string( argv[i+1] ) + "/locale" ) );
       break;
     }
   }

   try
   {
      Application app;
      app.start();
   }
   catch( Exception e )
   {
     StringHelper::debug( 0xff, "FATAL ERROR: %s", e.getDescription().c_str() );
   }

   return 0;
}
