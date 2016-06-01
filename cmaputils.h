/*
* CMapUtils.h - A collection of functions that works with c maps.
*
* v 1.0
*
* Copyright (C) 2009-2010 Marc-André Lamothe.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Library General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef CMAPUTILS_H_
#define CMAPUTILS_H_

#include <map>

using namespace std;

template <typename MAP>
void ReplaceKey(MAP& container, const typename MAP::key_type& oldKey, const typename MAP::key_type& newKey)
{
  if (!container.key_comp()(oldKey,newKey) && !container.key_comp()(newKey,oldKey))
    return;
  typename MAP::iterator begin(container.find(oldKey));
  if (begin != container.end())
  {
      container.insert(typename MAP::value_type(newKey, begin->second));
      container.erase(begin);
  }
}

#endif
