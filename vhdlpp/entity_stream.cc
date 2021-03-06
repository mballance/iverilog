/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "entity.h"

using namespace std;

void ComponentBase::write_to_stream(ostream&fd) const
{
      fd << "  component " << name_ << " is" << endl;
      fd << "   port(" << endl;

      vector<InterfacePort*>::const_iterator cur = ports_.begin();
      while (cur != ports_.end()) {
	    InterfacePort*item = *cur;
	    ++cur;

	    fd << "     " << item->name << " : ";
	    switch (item->mode) {
		case PORT_NONE:
		  fd << "???? ";
		  break;
		case PORT_IN:
		  fd << "in ";
		  break;
		case PORT_OUT:
		  fd << "out ";
		  break;
	    }

	    item->type->write_to_stream(fd);

	    if (cur != ports_.end())
		  fd << ";" << endl;
	    else
		  fd << endl;
      }

      fd << "   );" << endl;
      fd << "  end component;" << endl;
}
