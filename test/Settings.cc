//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sstream>
#include "json/json.h"

#include "Settings.hh"

#include "ICouchDB.hh"

using namespace std;

REGISTER_DOCUMENT_TYPE(Settings, "http://workrave.org/settings")

Settings::Settings()
{
  // FIXME: duplication
  set_type("http://workrave.org/settings"); 
}

Settings::~Settings()
{
}

void
Settings::get_value(const std::string &key, std::string &value) const
{
  value = root[key].asString();
}

void
Settings::set_value(const std::string &key, const std::string &value)
{
  root[key] = value;
}
