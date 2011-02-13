//
// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef SETTINGS_HH
#define SETTINGS_HH

#include <string>

#include "Document.hh"

class Settings : public Document
{
public:
  Settings();
  virtual ~Settings();

  void get_value(const std::string &key, std::string &value) const;
  void set_value(const std::string &key, const std::string &value);
};


  
#endif