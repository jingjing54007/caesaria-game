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

#ifndef _CAESARIA_WORLD_MERCHANT_INCLUDE_H_
#define _CAESARIA_WORLD_MERCHANT_INCLUDE_H_

#include "object.hpp"
#include "predefinitions.hpp"
#include "core/signals.hpp"
#include "core/position.hpp"

class GoodStore;

namespace world
{

class Merchant : public ReferenceCounted
{
public:
  static MerchantPtr create( TraderoutePtr route, const std::string& start,
                             GoodStore& sell, GoodStore& buy );
  virtual ~Merchant();

  Point location() const;
  void update( unsigned int time );

  std::string baseCity() const;
  std::string destinationCity() const;
  GoodStore& getSellGoods();
  GoodStore& getBuyGoods();

  bool isDeleted() const;
  void deleteLater();

  bool isSeaRoute() const;

  VariantMap save() const;
  void load( const VariantMap& stream );

oc3_signals public:
  Signal1<MerchantPtr>& onDestination();

private:
  Merchant();

  class Impl;
  ScopedPtr< Impl > _d;
};

} //end namespace world

#endif //_OPENCAESAR3_WORLD_TRADING_INCLUDE_H_
