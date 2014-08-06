// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#include "advisor_health_window.hpp"
#include "gfx/decorator.hpp"
#include "core/gettext.hpp"
#include "pushbutton.hpp"
#include "label.hpp"
#include "game/resourcegroup.hpp"
#include "core/stringhelper.hpp"
#include "gfx/engine.hpp"
#include "core/gettext.hpp"
#include "game/enums.hpp"
#include "objects/construction.hpp"
#include "city/helper.hpp"
#include "core/foreach.hpp"
#include "objects/house.hpp"
#include "texturedbutton.hpp"
#include "objects/constants.hpp"
#include "objects/service.hpp"

using namespace constants;
using namespace gfx;

namespace gui
{

namespace advisorwnd
{

class HealthInfoLabel : public Label
{
public:
  HealthInfoLabel( Widget* parent, const Rect& rect, const TileOverlay::Type service,
                   int workBulding, int numberBuilding, int peoplesCount  )
    : Label( parent, rect )
  {
    _service = service;
    _workingBuilding = workBulding;
    _numberBuilding = numberBuilding;
    _peoplesCount = peoplesCount;

    setFont( Font::create( FONT_1 ) );
  }

  virtual void _updateTexture( gfx::Engine& painter )
  {
    Label::_updateTexture( painter );

    std::string buildingStr, peoplesStr;
    switch( _service )
    {
    case building::baths: buildingStr = _("##bath##"); peoplesStr = _("##peoples##"); break;
    case building::barber: buildingStr = _("##barber##"); peoplesStr = _("##peoples##"); break;
    case building::hospital: buildingStr = _("##hospital##"); peoplesStr = _("##patients##"); break;
    case building::doctor: buildingStr = _("##clinics##"); peoplesStr = _("##peoples##"); break;
    default: break;
    }

    PictureRef& texture = _textPictureRef();
    Font rfont = font();
    std::string buildingStrT = StringHelper::format( 0xff, "%d %s", _numberBuilding, buildingStr.c_str() );
    rfont.draw( *texture, buildingStrT, 0, 0 );

    std::string buildingWorkT = StringHelper::format( 0xff, "%d", _workingBuilding );
    rfont.draw( *texture, buildingWorkT, 165, 0 );

    std::string peoplesStrT = StringHelper::format( 0xff, "%d %s", _peoplesCount, peoplesStr.c_str() );
    rfont.draw( *texture, peoplesStrT, 255, 0 );
  }

private:
  TileOverlay::Type _service;
  int _workingBuilding;
  int _numberBuilding;
  int _peoplesCount;
};

class Health::Impl
{
public:
  HealthInfoLabel* lbBathsInfo;
  HealthInfoLabel* lbBarbersInfo;
  HealthInfoLabel* lbDoctorInfo;
  HealthInfoLabel* lbHospitalInfo;
  TexturedButton* btnHelp;

  struct InfrastructureInfo
  {
    int buildingCount;
    int buildingWork;
    int peoplesServed;
  };

  InfrastructureInfo getInfo( PlayerCityPtr city, const TileOverlay::Type service )
  {
    city::Helper helper( city );

    InfrastructureInfo ret;

    ret.buildingWork = 0;
    ret.peoplesServed = 0;
    ret.buildingCount = 0;

    ServiceBuildingList srvBuildings = helper.find<ServiceBuilding>( service );
    foreach( b, srvBuildings )
    {
      ret.buildingWork += (*b)->numberWorkers() > 0 ? 1 : 0;
      ret.buildingCount++;
    }

    return ret;
  }
};


Health::Health(PlayerCityPtr city, Widget* parent, int id )
: Window( parent, Rect( 0, 0, 640, 290 ), "", id ), _d( new Impl )
{
  setupUI( ":/gui/healthadv.gui" );
  setPosition( Point( (parent->width() - 640 )/2, parent->height() / 2 - 242 ) );

  Point startPoint( 42, 112 );
  Size labelSize( 550, 20 );
  Impl::InfrastructureInfo info = _d->getInfo( city, building::baths );
  _d->lbBathsInfo = new HealthInfoLabel( this, Rect( startPoint, labelSize ), building::baths,
                                             info.buildingWork, info.buildingCount, info.peoplesServed );

  info = _d->getInfo( city, building::barber );
  _d->lbBarbersInfo = new HealthInfoLabel( this, Rect( startPoint + Point( 0, 20), labelSize), building::barber,
                                              info.buildingWork, info.buildingCount, info.peoplesServed );

  info = _d->getInfo( city, building::doctor );
  _d->lbDoctorInfo = new HealthInfoLabel( this, Rect( startPoint + Point( 0, 40), labelSize), building::doctor,
                                          info.buildingWork, info.buildingCount, info.peoplesServed );

  info = _d->getInfo( city, building::hospital );
  _d->lbDoctorInfo = new HealthInfoLabel( this, Rect( startPoint + Point( 0, 60), labelSize), building::hospital,
                                          info.buildingWork, info.buildingCount, info.peoplesServed );

  _d->btnHelp = new TexturedButton( this, Point( 12, height() - 39), Size( 24 ), -1, ResourceMenu::helpInfBtnPicId );
}

void Health::draw( gfx::Engine& painter )
{
  if( !visible() )
    return;

  Window::draw( painter );
}

}

}//end namespace gui
