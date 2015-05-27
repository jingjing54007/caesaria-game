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
// Copyright 2012-2014 dalerank, dalerankn8@gmail.com

#include "protestor.hpp"
#include "objects/house.hpp"
#include "pathway/path_finding.hpp"
#include "constants.hpp"
#include "city/statistic.hpp"
#include "objects/house_spec.hpp"
#include "objects/constants.hpp"
#include "core/foreach.hpp"
#include "pathway/astarpathfinding.hpp"
#include "gfx/tilemap.hpp"
#include "objects/constants.hpp"
#include "pathway/pathway_helper.hpp"
#include "corpse.hpp"
#include "core/priorities.hpp"
#include "ability.hpp"
#include "core/variant_map.hpp"
#include "events/disaster.hpp"
#include "game/resourcegroup.hpp"
#include "core/variant.hpp"
#include "game/gamedate.hpp"
#include "walkers_factory.hpp"

using namespace gfx;

REGISTER_CLASS_IN_WALKERFACTORY(walker::protestor, Protestor)

class Protestor::Impl
{
public:
  typedef enum { searchHouse=0, go2destination, searchAnyBuilding,
                 decreaseSentiment, go2anyplace, gooutFromCity, wait } State;
  int houseLevel;
  State state;

public:
  Pathway findTarget( PlayerCityPtr city, ConstructionList constructions, TilePos pos );
};

Protestor::Protestor(PlayerCityPtr city) : Human( city ), _d( new Impl )
{    
  _setType( walker::protestor );

  addAbility( Illness::create( 0.3, 4) );
}

void Protestor::_reachedPathway()
{
  Walker::_reachedPathway();

  switch( _d->state )
  {
  case Impl::go2destination:
    _animation().clear();
    _animation().load( ResourceGroup::citizen2, 455, 8 );
    _animation().load( ResourceGroup::citizen2, 462, 8, Animation::reverse );
    _setAction( acFight );
    _d->state = Impl::decreaseSentiment;
  break;

  default: break;
  }
}

void Protestor::_updateThoughts()
{
  StringArray ret;
  ret << "##protestor_say_1##" << "##protestor_say_2##" << "##protestor_say_3##";

  setThinks( ret.random() );
}

void Protestor::timeStep(const unsigned long time)
{
  Walker::timeStep( time );

  switch( _d->state )
  {
  case Impl::searchHouse:
  {
    ConstructionList constructions = city::statistic::getObjects<Construction>( _city(), object::house );
    for( ConstructionList::iterator it=constructions.begin(); it != constructions.end(); )
    {
      HousePtr h = (*it).is<House>();
      if( h->spec().level() <= _d->houseLevel ) { it=constructions.erase( it ); }
      else { ++it; }
    }

    Pathway pathway = _d->findTarget( _city(), constructions, pos() );
    //find more expensive house, fire this!!!
    if( pathway.isValid() )
    {
      setPos( pathway.startPos() );
      setPathway( pathway );
      go();
      _d->state = Impl::go2destination;
    }
    else //not find house, try find any, that rioter can destroy
    {
      _d->state = Impl::go2anyplace;
    }
  }
  break;

  case Impl::searchAnyBuilding:
  {
    ConstructionList constructions = city::statistic::getObjects<Construction>( _city(), object::house );

    for( ConstructionList::iterator it=constructions.begin(); it != constructions.end(); )
    {
      object::Type type = (*it)->type();
      object::Group group = (*it)->group();
      if( type == object::house || type == object::road || _d->excludeGroups.count( group ) > 0 )
      { it=constructions.erase( it ); }
      else { it++; }
    }

    Pathway pathway = _d->findTarget( _city(), constructions, pos() );
    if( pathway.isValid() )
    {
      setPos( pathway.startPos() );
      setPathway( pathway );
      go();
      _d->state = Impl::go2destination;
    }
    else
    {
      _d->state = Impl::go2anyplace;
    }
  }
  break;

  case Impl::go2anyplace:
  {
    Pathway pathway = PathwayHelper::randomWay( _city(), pos(), 10 );

    if( pathway.isValid() )
    {
      setPathway( pathway );
      go();
      _d->state = Impl::go2destination;
    }
    else
    {
      die();
      _d->state = Impl::wait;
    }
  }
  break;

  case Impl::go2destination:
  case Impl::wait:
  break;

  case Impl::decreaseSentiment:
  {
    if( game::Date::isDayChanged() )
    {
      ConstructionList constructions = city::statistic::getObjects<Construction>( _city(),
                                                                             object::any,
                                                                             pos() - TilePos( 1, 1), pos() + TilePos( 1, 1) );

      for( ConstructionList::iterator it=constructions.begin(); it != constructions.end(); )
      {
        if( (*it)->type() == object::road || _d->excludeGroups.count( (*it)->group() ) > 0  )
        { it=constructions.erase( it ); }
        else { ++it; }
      }

       if( constructions.empty() )
      {
        _animation().clear();
        _setAction( acMove );
        _d->state = Impl::searchHouse;
      }
      else
      {
        HouseList houses = constructions.select<House>();
        foreach( it, houses )
        {
          if( (*it)->state( pr::happiness ) > 20 )
          {
            (*it)->updateState( pr::happiness, -0.5 );
          }
          break;
        }
      }
    }
  }
  break;

  default: break;
  }
}

ProtestorPtr Protestor::create(PlayerCityPtr city )
{ 
  ProtestorPtr ret( new Protestor( city ) );
  ret->drop();
  return ret;
}

Protestor::~Protestor() {}

void Protestor::send2City( BuildingPtr bld )
{
  TilesArray tiles = bld->enterArea();
  if( tiles.empty() )
    return;

  setPos( tiles.random()->pos() );
  _d->houseLevel = 0;

  if( bld.is<House>() )
  {
    HousePtr house = ptr_cast<House>( bld );
    _d->houseLevel = house->spec().level();
  }

  _d->state = Impl::searchHouse;

  attach();
}

bool Protestor::die()
{
  bool created = Walker::die();

  if( !created )
  {
    Corpse::create( _city(), pos(), ResourceGroup::citizen2, 447, 454 );
    return true;
  }

  return created;
}

void Protestor::save(VariantMap& stream) const
{
  Human::save( stream );

  VARIANT_SAVE_ANY_D( stream, _d, houseLevel )
  VARIANT_SAVE_ANY_D( stream, _d, state )
}

void Protestor::load(const VariantMap& stream)
{
  Human::load( stream );

  VARIANT_LOAD_ANY_D( _d, houseLevel, stream )
  VARIANT_LOAD_ENUM_D( _d, state, stream )
}

int Protestor::agressive() const { return 1; }

Pathway Protestor::Impl::findTarget(PlayerCityPtr city, ConstructionList constructions, TilePos pos )
{  
  if( !constructions.empty() )
  {
    Pathway pathway;
    for( int i=0; i<10; i++)
    {
      ConstructionList::iterator it = constructions.begin();
      std::advance( it, rand() % constructions.size() );

      pathway = PathwayHelper::create( pos, *it, PathwayHelper::allTerrain );
      if( pathway.isValid() )
      {
        return pathway;
      }
    }
  }

  return Pathway();
}
