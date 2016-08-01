/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game/npc/Dismemberment.h"

#include <string>
#include <boost/algorithm/string.hpp>

#include "core/GameTime.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Item.h"
#include "game/NPC.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/data/MeshManipulation.h"
#include "physics/Box.h"
#include "physics/CollisionShapes.h"
#include "scene/GameSound.h"

static bool IsNearSelection(EERIE_3DOBJ * obj, long vert, ObjSelection tw) {
	
	if(!obj || tw == ObjSelection() || vert < 0)
		return false;

	const EERIE_SELECTIONS & sel = obj->selections[tw.handleData()];
	
	for(size_t i = 0; i < sel.selected.size(); i++) {
		float d = glm::distance(obj->vertexlist[sel.selected[i]].v, obj->vertexlist[vert].v);

		if(d < 8.f)
			return true;
	}

	return false;
}

/*!
 * \brief Spawns a body part from NPC
 * \param ioo
 * \param num
 */
static void ARX_NPC_SpawnMember(Entity * ioo, ObjSelection num) {
	
	if(!ioo)
		return;

	EERIE_3DOBJ * from = ioo->obj;

	if(!from || num == ObjSelection() || (size_t)num.handleData() >= from->selections.size())
		return;

	EERIE_3DOBJ * nouvo = new EERIE_3DOBJ;

	if(!nouvo)
		return;

	size_t nvertex = from->selections[num.handleData()].selected.size();

	long gore = -1;

	for(size_t k = 0; k < from->texturecontainer.size(); k++) {
		if(from->texturecontainer[k]
		   && boost::contains(from->texturecontainer[k]->m_texName.string(), "gore"))
		{
			gore = k;
			break;
		}
	}

	for(size_t k = 0; k < from->facelist.size(); k++) {
		if(from->facelist[k].texid == gore) {
			if(   IsNearSelection(from, from->facelist[k].vid[0], num)
			   || IsNearSelection(from, from->facelist[k].vid[1], num)
			   || IsNearSelection(from, from->facelist[k].vid[2], num)
			) {
				nvertex += 3;
			}
		}
	}

	nouvo->vertexlist.resize(nvertex);
	nouvo->vertexlist3.resize(nvertex);

	size_t inpos = 0;
	long * equival = (long *)malloc(sizeof(long) * from->vertexlist.size());
	if(!equival) {
		delete nouvo;
		return;
	}

	for(size_t k = 0; k < from->vertexlist.size(); k++) {
		equival[k] = -1;
	}
	
	const EERIE_SELECTIONS & cutSelection = from->selections[num.handleData()];

	arx_assert(0 < cutSelection.selected.size());

	for(size_t k = 0; k < cutSelection.selected.size(); k++) {
		inpos = cutSelection.selected[k];
		equival[cutSelection.selected[k]] = k;
		
		nouvo->vertexlist[k] = from->vertexlist[cutSelection.selected[k]];
		nouvo->vertexlist[k].v = from->vertexlist3[cutSelection.selected[k]].v;
		nouvo->vertexlist[k].v -= ioo->pos;
		nouvo->vertexlist[k].vert.p = nouvo->vertexlist[k].v;
		
		nouvo->vertexlist[k].vert.color = from->vertexlist[k].vert.color;
		nouvo->vertexlist[k].vert.uv = from->vertexlist[k].vert.uv;
		
		nouvo->vertexlist3[k] = nouvo->vertexlist[k];
	}

	size_t count = cutSelection.selected.size();

	for(size_t k = 0; k < from->facelist.size(); k++) {
		if(from->facelist[k].texid == gore) {
			if(   IsNearSelection(from, from->facelist[k].vid[0], num)
			   || IsNearSelection(from, from->facelist[k].vid[1], num)
			   || IsNearSelection(from, from->facelist[k].vid[2], num)
			) {
				for(long j = 0; j < 3; j++) {
					equival[from->facelist[k].vid[j]] = count;

					if(count < nouvo->vertexlist.size()) {
						nouvo->vertexlist[count] = from->vertexlist[from->facelist[k].vid[j]];
						nouvo->vertexlist[count].v = from->vertexlist3[from->facelist[k].vid[j]].v;
						nouvo->vertexlist[count].v -= ioo->pos;
						nouvo->vertexlist[count].vert.p = nouvo->vertexlist[count].v;

						nouvo->vertexlist3[count] = nouvo->vertexlist[count];
					} else {
						equival[from->facelist[k].vid[j]] = -1;
					}

					count++;
				}
			}
		}
	}

	float min = nouvo->vertexlist[0].vert.p.y;
	long nummm = 0;

	for(size_t k = 1; k < nouvo->vertexlist.size(); k++) {
		if(nouvo->vertexlist[k].vert.p.y > min) {
			min = nouvo->vertexlist[k].vert.p.y;
			nummm = k;
		}
	}
	
	nouvo->origin = nummm;
	nouvo->point0 = nouvo->vertexlist[nouvo->origin].v;
	
	for(size_t k = 0; k < nouvo->vertexlist.size(); k++) {
		nouvo->vertexlist[k].vert.p = nouvo->vertexlist[k].v -= nouvo->point0;
		nouvo->vertexlist[k].vert.color = Color(255, 255, 255, 255).toRGBA();
	}
	
	nouvo->point0 = Vec3f_ZERO;
	
	nouvo->pbox = NULL;
	nouvo->pdata = NULL;
	nouvo->cdata = NULL;
	nouvo->sdata = NULL;
	nouvo->ndata = NULL;
	
	size_t nfaces = 0;
	for(size_t k = 0; k < from->facelist.size(); k++) {
		if(   equival[from->facelist[k].vid[0]] != -1
		   && equival[from->facelist[k].vid[1]] != -1
		   && equival[from->facelist[k].vid[2]] != -1
		) {
			nfaces++;
		}
	}

	if(nfaces) {
		nouvo->facelist.reserve(nfaces);

		for(size_t k = 0; k < from->facelist.size(); k++) {
			if(   equival[from->facelist[k].vid[0]] != -1
			   && equival[from->facelist[k].vid[1]] != -1
			   && equival[from->facelist[k].vid[2]] != -1
			) {
				EERIE_FACE newface = from->facelist[k];
				newface.vid[0] = (unsigned short)equival[from->facelist[k].vid[0]];
				newface.vid[1] = (unsigned short)equival[from->facelist[k].vid[1]];
				newface.vid[2] = (unsigned short)equival[from->facelist[k].vid[2]];
				nouvo->facelist.push_back(newface);
			}
		}

		long gore = -1;

		for(size_t k = 0; k < from->texturecontainer.size(); k++) {
			if(from->texturecontainer[k]
			   && boost::contains(from->texturecontainer[k]->m_texName.string(), "gore")
			) {
				gore = k;
				break;
			}
		}

		for(size_t k = 0; k < nouvo->facelist.size(); k++) {
			nouvo->facelist[k].facetype &= ~POLY_HIDE;

			if(nouvo->facelist[k].texid == gore) {
				nouvo->facelist[k].facetype |= POLY_DOUBLESIDED;
			}
		}
	}
	
	free(equival);
	nouvo->texturecontainer = from->texturecontainer;
	
	nouvo->linked.clear();
	nouvo->originaltextures = NULL;
	
	Entity * io = new Entity("noname", EntityInstance(0));
	
	io->_itemdata = new IO_ITEMDATA();
	
	io->ioflags = IO_ITEM | IO_NOSAVE | IO_MOVABLE;
	io->script.size = 0;
	io->script.data = NULL;
	io->gameFlags |= GFLAG_NO_PHYS_IO_COL;
	
	EERIE_COLLISION_Cylinder_Create(io);
	EERIE_PHYSICS_BOX_Create(nouvo);
	if(!nouvo->pbox){
		delete nouvo;
		return;
	}
	
	io->infracolor = Color3f::blue * 0.8f;
	io->collision = COLLIDE_WITH_PLAYER;
	io->m_icon = NULL;
	io->scriptload = 1;
	io->obj = nouvo;
	io->lastpos = io->initpos = io->pos = ioo->obj->vertexlist3[inpos].v;
	io->angle = ioo->angle;
	
	io->gameFlags = ioo->gameFlags;
	io->halo = ioo->halo;
	
	io->angle.setYawPITCH(Random::getf(340.f, 380.f));
	io->angle.setPitchYAW(Random::getf(0.f, 360.f));
	io->angle.setRoll(0);
	io->obj->pbox->active = 1;
	io->obj->pbox->stopcount = 0;
	
	io->velocity = Vec3f_ZERO;
	io->stopped = 1;
	
	Vec3f vector;
	vector.x = -std::sin(glm::radians(io->angle.getPitchYAW()));
	vector.y = std::sin(glm::radians(io->angle.getYawPITCH())) * 2.f;
	vector.z = std::cos(glm::radians(io->angle.getPitchYAW()));
	vector = glm::normalize(vector);
	io->rubber = 0.6f;
	
	io->no_collide = ioo->index();
	
	io->gameFlags |= GFLAG_GOREEXPLODE;
	io->animBlend.lastanimtime = arxtime.now();
	io->soundtime = ArxInstant_ZERO;
	io->soundcount = 0;

	EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, io->angle, vector);
}



static DismembermentFlag GetCutFlag(const std::string & str) {
	
	if(str == "cut_head") {
		return FLAG_CUT_HEAD;
	} else if(str == "cut_torso") {
		return FLAG_CUT_TORSO;
	} else if(str == "cut_larm") {
		return FLAG_CUT_LARM;
	} else if(str == "cut_rarm") {
		return FLAG_CUT_HEAD;
	} else if(str == "cut_lleg") {
		return FLAG_CUT_LLEG;
	} else if(str == "cut_rleg") {
		return FLAG_CUT_RLEG;
	}
	
	return DismembermentFlag(0);
}

static ObjSelection GetCutSelection(Entity * io, DismembermentFlag flag) {
	
	if(!io || !(io->ioflags & IO_NPC) || flag == 0)
		return ObjSelection();

	std::string tx;

	if (flag == FLAG_CUT_HEAD)
		tx =  "cut_head";
	else if (flag == FLAG_CUT_TORSO)
		tx = "cut_torso";
	else if (flag == FLAG_CUT_LARM)
		tx = "cut_larm";

	if (flag == FLAG_CUT_RARM)
		tx = "cut_rarm";

	if (flag == FLAG_CUT_LLEG)
		tx = "cut_lleg";

	if (flag == FLAG_CUT_RLEG)
		tx = "cut_rleg";

	if ( !tx.empty() )
	{
		typedef std::vector<EERIE_SELECTIONS>::iterator iterator; // Convenience
		for(iterator iter = io->obj->selections.begin(); iter != io->obj->selections.end(); ++iter) {
			if(iter->selected.size() > 0 && iter->name == tx) {
				return ObjSelection(iter - io->obj->selections.begin());
			}
		}
	}

	return ObjSelection();
}

static void ReComputeCutFlags(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC))
		return;

	if(io->_npcdata->cuts & FLAG_CUT_TORSO) {
		io->_npcdata->cuts &= ~FLAG_CUT_HEAD;
		io->_npcdata->cuts &= ~FLAG_CUT_LARM;
		io->_npcdata->cuts &= ~FLAG_CUT_RARM;
	}
}

static bool IsAlreadyCut(Entity * io, DismembermentFlag fl) {
	
	if(io->_npcdata->cuts & fl)
		return true;

	if(io->_npcdata->cuts & FLAG_CUT_TORSO) {
		if(fl == FLAG_CUT_HEAD)
			return true;

		if(fl == FLAG_CUT_LARM)
			return true;

		if(fl == FLAG_CUT_RARM)
			return true;
	}

	return false;
}

static bool ARX_NPC_ApplyCuts(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC))
		return false;

	if(io->_npcdata->cuts == 0)
		return false;	// No cuts

	ReComputeCutFlags(io);
	long goretex = -1;

	for(size_t i = 0; i < io->obj->texturecontainer.size(); i++) {
		if (io->obj->texturecontainer[i]
		        &&	(boost::contains(io->obj->texturecontainer[i]->m_texName.string(), "gore")))
		{
			goretex = i;
			break;
		}
	}

	bool hid = false;

	for(size_t nn = 0; nn < io->obj->facelist.size(); nn++) {
		io->obj->facelist[nn].facetype &= ~POLY_HIDE;
	}

	for(long jj = 0; jj < 6; jj++) {
		DismembermentFlag flg = DismembermentFlag(1 << jj);
		ObjSelection numsel = GetCutSelection(io, flg);

		if((io->_npcdata->cuts & flg) && numsel != ObjSelection()) {
			for(size_t ll = 0; ll < io->obj->facelist.size(); ll++) {
				EERIE_FACE & face = io->obj->facelist[ll];

				if(   IsInSelection(io->obj, face.vid[0], numsel)
				   || IsInSelection(io->obj, face.vid[1], numsel)
				   || IsInSelection(io->obj, face.vid[2], numsel)
				) {
					if(!(face.facetype & POLY_HIDE)) {
						if(face.texid != goretex)
							hid = true;
					}

					face.facetype |= POLY_HIDE;
				}
			}

			io->_npcdata->cut = 1;
		}
	}

	return hid;
}

void ARX_NPC_TryToCutSomething(Entity * target, const Vec3f * pos)
{
	//return;
	if(!target || !(target->ioflags & IO_NPC))
		return;

	if(target->gameFlags & GFLAG_NOGORE)
		return;

	float mindistSqr = std::numeric_limits<float>::max();
	ObjSelection numsel = ObjSelection();
	long goretex = -1;

	for(size_t i = 0; i < target->obj->texturecontainer.size(); i++) {
		if(target->obj->texturecontainer[i]
		   && boost::contains(target->obj->texturecontainer[i]->m_texName.string(), "gore")
		) {
			goretex = i;
			break;
		}
	}

	for(size_t i = 0; i < target->obj->selections.size(); i++) {
		ObjSelection sel = ObjSelection(i);
		
		if(target->obj->selections[i].selected.size() > 0
		   && boost::contains(target->obj->selections[i].name, "cut_")
		) {
			DismembermentFlag fll = GetCutFlag(target->obj->selections[i].name);

			if(IsAlreadyCut(target, fll))
				continue;

			long out = 0;

			for(size_t ll = 0; ll < target->obj->facelist.size(); ll++) {
				EERIE_FACE & face = target->obj->facelist[ll];

				if(face.texid != goretex) {
					if(   IsInSelection(target->obj, face.vid[0], sel)
					   || IsInSelection(target->obj, face.vid[1], sel)
					   || IsInSelection(target->obj, face.vid[2], sel)
					) {
						if(face.facetype & POLY_HIDE) {
							out++;
						}
					}
				}
			}

			if(out < 3) {
				float dist = glm::distance2(*pos, target->obj->vertexlist3[target->obj->selections[i].selected[0]].v);

				if(dist < mindistSqr) {
					mindistSqr = dist;
					numsel = sel;
				}
			}
		}
	}

	if(numsel == ObjSelection())
		return; // Nothing to cut...

	bool hid = false;

	if(mindistSqr < square(60)) { // can only cut a close part...
		DismembermentFlag fl = GetCutFlag( target->obj->selections[numsel.handleData()].name );

		if(fl && !(target->_npcdata->cuts & fl)) {
			target->_npcdata->cuts |= fl;
			hid = ARX_NPC_ApplyCuts(target);
		}
	}

	if(hid) {
		ARX_SOUND_PlayCinematic("flesh_critical", false); // TODO why play cinmeatic sound?
		ARX_NPC_SpawnMember(target, numsel);
	}
}


void ARX_NPC_RestoreCuts() {
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && (e->ioflags & IO_NPC)
		   && e->_npcdata->cuts) {
			ARX_NPC_ApplyCuts(e);
		}
	}
}
