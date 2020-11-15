//
//	
// Laidout, for laying out
// Please consult http://www.laidout.org about where to send any
// correspondence about this software.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
// For more details, consult the COPYING file in the top directory.
//
// Copyright (C) 2013 by Tom Lechner
//

#include "lpathsdata.h"
#include "datafactory.h"
#include "../core/stylemanager.h"
#include "../language.h"
#include "../calculator/shortcuttodef.h"
#include "helpertypes.h"
#include "affinevalue.h"
#include "objectfilter.h"



namespace Laidout {



//------------------------------- LPathsData ---------------------------------------

/*! \class LPathsData 
 * \brief Subclassing LaxInterfaces::PathsData
 */



LPathsData::LPathsData(LaxInterfaces::SomeData *refobj)
  : LaxInterfaces::PathsData(0)
{
	child_clip_type = CLIP_From_Parent_Area;
}

LPathsData::~LPathsData()
{
}

void LPathsData::FindBBox()
{
	PathsData::FindBBox();
}

void LPathsData::ComputeAABB(const double *transform, DoubleBBox &box)
{
	PathsData::ComputeAABB(transform, box);
	DrawableObject::ComputeAABB(transform, box);
}

/*! Provide final pointin() definition.
 */
int LPathsData::pointin(flatpoint pp,int pin)
{
	return PathsData::pointin(pp,pin);
}


void LPathsData::touchContents()
{
	SomeData::touchContents();

	ObjectFilter *ofilter = dynamic_cast<ObjectFilter*>(filter);
	if (ofilter) {
		NodeProperty *prop = ofilter->FindProperty("out");
		//DrawableObject *fobj = dynamic_cast<DrawableObject*>(ofilter->FinalObject());
		clock_t recent = ofilter->MostRecentIn(nullptr);
		if (recent > prop->modtime) {
			// filter needs updating
			ofilter->FindProperty("in")->topropproxy->owner->Update();
			//ofilter->Update();
		}
	}
}

void LPathsData::dump_out(FILE *f,int indent,int what,LaxFiles::DumpContext *context)
{
	Attribute att;
	dump_out_atts(&att, what, context);
	att.dump_out(f, indent);
	// char spc[indent+1]; memset(spc,' ',indent); spc[indent]='\0';
	// DrawableObject::dump_out(f,indent,what,context);
	// fprintf(f,"%sconfig\n",spc);
	// PathsData::dump_out(f,indent+2,what,context);
}

LaxFiles::Attribute *LPathsData::dump_out_atts(LaxFiles::Attribute *att,int what,LaxFiles::DumpContext *context)
{
	att = DrawableObject::dump_out_atts(att, what,context);
	LaxFiles::Attribute *att2 = att->pushSubAtt("config");
	PathsData::dump_out_atts(att2, what,context);
	return att;
}

void LPathsData::dump_in_atts(LaxFiles::Attribute *att,int flag,LaxFiles::DumpContext *context)
{
	DrawableObject::dump_in_atts(att,flag,context);
	int foundconfig=0;
	for (int c=0; c<att->attributes.n; c++) {
		if (!strcmp(att->attributes.e[c]->name,"config")) {
			foundconfig=1;
			PathsData::dump_in_atts(att->attributes.e[c],flag,context);
		}
	}
	if (!foundconfig) PathsData::dump_in_atts(att,flag,context);

	if (filter) {
		ObjectFilter *of = dynamic_cast<ObjectFilter*>(filter);
		of->FindProperty("in")->topropproxy->owner->Update();
	}
}

LaxInterfaces::SomeData *LPathsData::duplicate(LaxInterfaces::SomeData *dup)
{
	if (dup && !dynamic_cast<LPathsData*>(dup)) return NULL; //wrong type for referencc object!
	if (!dup) dup=dynamic_cast<SomeData*>(LaxInterfaces::somedatafactory()->NewObject("PathsData"));
	PathsData::duplicate(dup);
	DrawableObject::duplicate(dup);
	return dup;
}


//--------Value functions: 


Value *LPathsData::duplicate()
{
	SomeData *dup=dynamic_cast<SomeData*>(LaxInterfaces::somedatafactory()->NewObject("PathsData"));
	PathsData::duplicate(dup);
	DrawableObject::duplicate(dup);
	return dynamic_cast<Value*>(dup);
}

Value *NewLPathsData() { return new LPathsData; }

/*! Add to stylemanager if not exists. Else create, along with same for LineStyle, FillStyle, and Affine.
 */
ObjectDef *LPathsData::makeObjectDef()
{
	ObjectDef *sd = stylemanager.FindDef("PathsData");
	if (sd) {
		sd->inc_count();
		return sd;
	}

	LLineStyle lstyle;
	lstyle.GetObjectDef();
	LFillStyle fstyle;
	fstyle.GetObjectDef();

	ObjectDef *gdef = stylemanager.FindDef("Group");
	if (!gdef) {
		Group g;
		gdef = g.GetObjectDef();
	}
	sd = new ObjectDef(gdef,
			"PathsData",
            _("PathsData"),
            _("A collection of paths"),
            NewLPathsData,NULL);

	sd->pushVariable("linestyle", _("Line style"), _("Default line style of all subpaths"), "LineStyle", 0, nullptr,0);
	sd->pushVariable("fillstyle", _("Fill style"), _("Default fill style of all subpaths"), "FillStyle", 0, nullptr,0);

	sd->pushFunction("moveto",_("moveto"),_("Start a new subpath"), NULL,
					"x",_("X"),_("X position"),"number", NULL,NULL,
					"y",_("Y"),_("Y position"),"number", NULL,NULL,
					"p",_("P"),_("Point"),"flatvector", NULL,NULL,
					 NULL);

	sd->pushFunction("lineto",_("lineto"),_("Add a simple straight line to the path"), NULL,
					"x",_("X"),_("X position"),"number", NULL,NULL,
					"y",_("Y"),_("Y position"),"number", NULL,NULL,
					"p",_("P"),_("Point"),"flatvector", NULL,NULL,
					 NULL);

	sd->pushFunction("curveto",_("curveto"),_("Add a bezier segment"), NULL,
					 "c1",_("c1"),_("Control for current point"),"flatvector", NULL,NULL,
                     "c2",_("c2"),_("Control for new point"),"flatvector", NULL,NULL,
                     "p",_("p"),_("Point"),"flatvector", NULL,NULL,
					 NULL);

	sd->pushFunction("appendRect",_("appendRect"),_("Append a rectangle"), NULL,
                     "x",_("x"),_("x"),"real", NULL,NULL,
                     "y",_("y"),_("y"),"real", NULL,NULL,
                     "w",_("w"),_("Width"),"real", NULL,NULL,
                     "h",_("h"),_("Height"),"real", NULL,NULL,
					 NULL);

	sd->pushFunction("close",_("close"),_("Close current path. New points will start a new subpath"), NULL, NULL);

	sd->pushFunction("NumPaths",_("NumPaths"),_("Number of subpaths"), NULL, NULL);

	sd->pushFunction("clear",_("Clear"),_("Clear all paths"), NULL, NULL);

	sd->pushFunction("pushEmpty",_("Push empty subpath"),_("Add empty subpath to current path stack"), NULL, NULL);

	sd->pushFunction("RemovePath",_("Remove path"),_("Remove path with given index"), NULL,
                     "index",_("Index"),_("Index starting at 0, or -1 for the top"),"int", NULL,NULL,
                     NULL);

	stylemanager.AddObjectDef(sd, 0);
	return sd;
}

Value *LPathsData::dereference(const char *extstring, int len)
{
	if (isName(extstring, len, "linestyle")) {
		if (!linestyle) return nullptr;
		LLineStyle *ls = new LLineStyle(linestyle);
		return ls;

	} else if (isName(extstring, len, "fillstyle")) {
		if (!fillstyle) return nullptr;
		LFillStyle *fs = new LFillStyle(fillstyle);
		return fs;
	}

	return DrawableObject::dereference(extstring, len);
}

/* Return 1 for success.
 *  2 for success, but other contents changed too.
 *  0 for total fail, as when v is wrong type.
 *  -1 for bad extension.
 *
 *  Default is return 0;
 */
int LPathsData::assign(FieldExtPlace *ext,Value *v)
{
	if (ext->n() == 1) {
		const char *what = ext->e(0);
		if (what) {
			if (!strcmp(what, "fillstyle")) {
				LFillStyle *fs = dynamic_cast<LFillStyle*>(v);
				if (!fs || !fs->fillstyle) return 0;
				InstallFillStyle(fs->fillstyle);
				return 1;

			} else if (!strcmp(what, "linestyle")) {
				LLineStyle *s = dynamic_cast<LLineStyle*>(v);
				if (!s || !s->linestyle) return 0;
				InstallLineStyle(s->linestyle);
				return 1;
			}
		}
	}

	return DrawableObject::assign(ext,v);
}

/*! Return 0 success, -1 incompatible values, 1 for error.
 */
int LPathsData::Evaluate(const char *func,int len, ValueHash *context, ValueHash *parameters, CalcSettings *settings,
	                     Value **value_ret, Laxkit::ErrorLog *log)
{
	if (isName(func,len, "clear")) {
		clear();
		return 0;

	} else if (isName(func,len, "close")) {
		close();
		return 0;

	} else if (isName(func,len, "pushEmpty")) {
		pushEmpty();
		return 0;

	} else if (isName(func,len, "RemovePath")) {
		int err = 0;
		int index = parameters->findInt("index", -1, &err);
		if (err != 0) {
			if (log) log->AddError(_("Bad index"));
			return 1;
		}
		if (index == -1) index = paths.n-1;
		if (index < 0 || index >= paths.n) {
			if (log) log->AddError(_("Bad index"));
			return 1;
		}
		RemovePath(index, nullptr);
		return 0;

	} else if (isName(func,len, "NumPaths")) {
		IntValue *i = new IntValue(paths.n);
		*value_ret = i;
		return 0;

	} else if (isName(func,len, "moveto")) {
		log->AddError("NEED TO IMPLEMENT!!!!");
		return 1;
	} else if (isName(func,len, "lineto")) {
		log->AddError("NEED TO IMPLEMENT!!!!");
		return 1;
	} else if (isName(func,len, "curveto")) {
		log->AddError("NEED TO IMPLEMENT!!!!");
		return 1;
	} else if (isName(func,len, "appendRect")) {
		log->AddError("NEED TO IMPLEMENT!!!!");
		return 1;
	} else if (isName(func,len, "appendEllipse")) {
		log->AddError("NEED TO IMPLEMENT!!!!");
		return 1;
	}

	return DrawableObject::Evaluate(func, len, context, parameters, settings, value_ret, log);
}




//------------------------------- LPathInterface --------------------------------
/*! \class LPathInterface
 * \brief add on a little custom behavior.
 */


LPathInterface::LPathInterface(int nid,Laxkit::Displayer *ndp)
  : PathInterface(nid,ndp)
{
	pathi_style |= LaxInterfaces::PATHI_Render_With_Cache;
	cache_modified = 0;
}


LaxInterfaces::anInterface *LPathInterface::duplicate(LaxInterfaces::anInterface *dup)
{
	if (dup==NULL) dup=dynamic_cast<anInterface *>(new LPathInterface(id,NULL));
	else if (!dynamic_cast<LPathInterface *>(dup)) return NULL;

	return PathInterface::duplicate(dup);
}


//! Returns this, but count is incremented.
Value *LPathInterface::duplicate()
{
    this->inc_count();
    return this;
}

void LPathInterface::Modified(int level)
{
	PathInterface::Modified(level);
	cache_modified = 1;
}

int LPathInterface::LBUp(int x,int y,unsigned int state,const Laxkit::LaxMouse *d)
{
	int ret = PathInterface::LBUp(x,y,state,d);
	if (cache_modified) {
		if (data && dynamic_cast<DrawableObject*>(data)->filter) {
			ObjectFilter *f = dynamic_cast<ObjectFilter*>(dynamic_cast<DrawableObject*>(data)->filter);
			if (f) f->ForceUpdates();
		}
		cache_modified = 0;
	}
	return ret;
}

ObjectDef *LPathInterface::makeObjectDef()
{

	ObjectDef *sd=stylemanager.FindDef("PathInterface");
    if (sd) {
        sd->inc_count();
        return sd;
    }

	sd=new ObjectDef(NULL,"PathInterface",
            _("Path Interface"),
            _("Path Interface"),
            "class",
            NULL,NULL);

	if (!sc) sc=GetShortcuts();
	ShortcutsToObjectDef(sc, sd);

	stylemanager.AddObjectDef(sd,0);
	return sd;
}


///*!
// * Return
// *  0 for success, value optionally returned.
// * -1 for no value returned due to incompatible parameters, which aids in function overloading.
// *  1 for parameters ok, but there was somehow an error, so no value returned.
// */
//int LPathInterface::Evaluate(const char *func,int len, ValueHash *context, ValueHash *parameters, CalcSettings *settings,
//	                     Value **value_ret, Laxkit::ErrorLog *log)
//{
//	return 1;
//}

/*! *** for now, don't allow assignments
 *
 * If ext==NULL, then assign v to replace what exists in this.
 * Otherwise assign v to the value at the end of the extension.
 *  
 * Return 1 for success.
 *  2 for success, but other contents changed too.
 *  0 for total fail, as when v is wrong type.
 *  -1 for bad extension.
 */
int LPathInterface::assign(FieldExtPlace *ext,Value *v)
{
	 //assignments not allowed
	return 0;
}

Value *LPathInterface::dereference(const char *extstring, int len)
{
	return NULL;
}

void LPathInterface::dump_out(FILE *f,int indent,int what,LaxFiles::DumpContext *context)
{
	PathInterface::dump_out(f,indent,what,context);
}

void LPathInterface::dump_in_atts(LaxFiles::Attribute *att,int flag,LaxFiles::DumpContext *context)
{
	PathInterface::dump_in_atts(att,flag,context);
}

LaxFiles::Attribute *LPathInterface::dump_out_atts(LaxFiles::Attribute *att,int what,LaxFiles::DumpContext *savecontext)
{
	return att;
}


} //namespace Laidout

