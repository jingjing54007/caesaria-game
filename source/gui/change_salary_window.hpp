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

#ifndef _CAESARIA_CHANGE_SALARY_WINDOW_H_INCLUDE_
#define _CAESARIA_CHANGE_SALARY_WINDOW_H_INCLUDE_

#include "gui/widget.hpp"
#include "core/signals.hpp"

namespace gui
{

class ChangeSalaryWindow : public Widget
{
public:
  ChangeSalaryWindow(Widget* p, unsigned int salary );

  virtual ~ChangeSalaryWindow();
public oc3_signals:
  Signal1<int>& onChangeSalary();

private:
  __DECLARE_IMPL(ChangeSalaryWindow)
};

} //end namespace gui

#endif //_CAESARIA_CHANGE_SALARY_WINDOW_H_INCLUDE_
