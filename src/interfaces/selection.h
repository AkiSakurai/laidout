//
// $Id$
//  
// Laidout, for laying out
// Please consult http://www.laidout.org about where to send any
// correspondence about this software.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// For more details, consult the COPYING file in the top directory.
//
// Copyright (C) 2014 by Tom Lechner
//
#ifndef SELECTION_H
#define SELECTION_H

#include <lax/interfaces/viewportwindow.h>
#include "../calculator/values.h"

namespace Laidout {

//--------------------------- SelectedObject -------------------------

/*! \class SelectedObject
 */
class SelectedObject
{
  public:
	LaxInterfaces::SomeData *obj;
	LaxInterfaces::ObjectContext *oc;
	ValueHash properties;

	SelectedObject(LaxInterfaces::ObjectContext *noc);
	virtual ~SelectedObject();
};


//--------------------------- Selection -------------------------

class Selection : public Laxkit::anObject, public Laxkit::DoubleBBox
{
	Laxkit::PtrStack<SelectedObject> objects;
	int currentobject;
	anObject *base_object;

  public:
	virtual int n() { return objects.n; }
	virtual LaxInterfaces::ObjectContext *e(int i);
	virtual ValueHash *e_properties(int i);
	virtual int Add(LaxInterfaces::ObjectContext *oc, int where);
	virtual int AddNoDup(LaxInterfaces::ObjectContext *oc, int where);
	virtual int Remove(int i);
	virtual void Flush();
	virtual LaxInterfaces::ObjectContext *CurrentObject();
	virtual int CurrentObjectIndex() { return currentobject; }
	virtual void CurrentObject(int which);
	virtual int FindIndex(LaxInterfaces::ObjectContext *oc);
	//virtual void FindBBox();

	Selection();
	virtual ~Selection();
};


} //namespace Laidout

#endif

