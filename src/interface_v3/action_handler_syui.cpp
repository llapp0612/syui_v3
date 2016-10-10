
//Setup
#include "bar_manager.h"
#include "action_handler_misc.h"
#include "nel/gui/interface_expr.h"
#include "../entities.h"
#include "nel/gui/action_handler.h"
#include "action_handler_game.h"
#include "nel/gui/group_container.h"

#include "nel/misc/vectord.h"

// the enemy target slot is always 0, when the range is to far. this will fix it. maybe there is another better way, but not found yet
static int checkSlot(int &nSlot)
{
	CLFECOMMON::TCLEntityId target = UserEntity->selection();
	if (target != CLFECOMMON::INVALID_SLOT)
	{
		NLMISC::CCDBNodeLeaf *tSlot = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:TARGET:SLOT");
		CEntityCL *Ent = EntitiesMngr.entity(nSlot);
        CVectorD vec = UserEntity->pos() - EntitiesMngr.entity(target)->pos();

		double nDistance = fabs(vec.x) + fabs(vec.y);
		if (nDistance > CLFECOMMON::THRESHOLD_TARGET_ID_CLIENT_M)
		{
			if (Ent)
			{
				if (nSlot == 0 && EntitiesMngr.entity(tSlot->getValue32())->isEnemy())
					nSlot = 255; // have to set Slot to 255, because if the target is to far away, then the server send always Slot 0 for targets target
			}
		}
	}
	return nSlot;
}

// *** called when tartar change or tartar name change
// *** need for syui
class CActionHandlerSetTartarName : public IActionHandler
{
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		string sSlot = getParam(Params, "slot");
		string sNameTartar = getParam(Params, "target");
		string sTitleTartar = getParam(Params, "title");

		if (sSlot.empty()) return;

		if (!findInterfacePath(sNameTartar, pCaller)) return;

		CInterfaceExprValue evValue;
		if (CInterfaceExpr::eval(sSlot, evValue, NULL))
		{
			sint32 nSlot = (sint32)evValue.getInteger();

			ucstring TartarName;
			ucstring TartarTitle;
			ucstring preTartarName;
			ucstring preTartarTitle;

			// Get from nSlot
			if (nSlot > -1)
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				//uint32 nDBid = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString(nSlot)+":P6")->getValue32();
				uint32 nDBid = 0;

				if (nSlot < sint32(EntitiesMngr.entities().size()) && EntitiesMngr.entities()[nSlot] != NULL)
				{
					nDBid = EntitiesMngr.entities()[nSlot]->getNameId();
				}

				CGroupContainer *pTarget = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:target"));
				CGroupContainer *pTartar = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:tartar"));
				
				nSlot = checkSlot(nSlot);
				//need to remove uctitle from tartar, if target is empty
				if (pTarget != NULL)
				{
				    if (pTarget->getActive()) 
					{
						if (pTarget->getUCTitle() == ucstring(""))
						{
							if (pTartar != NULL)
							{
								if (pTartar->getActive())
								{
									nSlot = 255;
									pTartar->setUCTitle(ucstring("")); //will remove the title, otherwise get the title of the target, when target is removed
								}
							}
						}
					}
				}

				//need to remove uctitle from tartar, if targets target is 255
				if (nSlot == 255)
				{
					if (pTartar != NULL)
					{
						if (pTartar->getActive())
						{
							nSlot = 255;
							pTartar->setUCTitle(ucstring("")); //will remove the title, otherwise get the title of the target, when target is removed
						}
					}
				}
				
				CEntityCL *pE = EntitiesMngr.entity(nSlot);
				if (!pE->isUser() && nSlot == 0)
				{
					nSlot = 255;
				}

				if (nDBid != 0 && nSlot != 255)
				{
					CSPPRemoveTitleAndCheckSelection *pSPPRT = new CSPPRemoveTitleAndCheckSelection;
					pSPPRT->Slot = nSlot;
					pIM->addServerID(sNameTartar, nDBid, pSPPRT);
				}

				if (pE != NULL)
				{
					preTartarName = pE->getDisplayName();
					preTartarTitle = pE->getTitle();
					if (!preTartarName.empty() && UserEntity->canEngageCombat())
					{
						TartarName = preTartarName;
					}
					if (preTartarName.empty() && !preTartarTitle.empty())
					{
						TartarName = preTartarTitle;
					}
				}
				if (nDBid == 0 && pE == NULL)
				{
					TartarName = ucstring("");
					return;
				}
			}
			// Set to target
			CInterfaceExprValue evUCStr;
			evUCStr.setUCString(TartarName);
			CInterfaceLink::setTargetProperty(sNameTartar, evUCStr);
		}
	}
};
REGISTER_ACTION_HANDLER(CActionHandlerSetTartarName, "set_tartar_name");

// ***************************************************************************

class CActionHandlerSetTarHp : public IActionHandler
{
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		string sSlot = getParam(Params, "slot");

		if (sSlot.empty()) return;
		CInterfaceExprValue evValue;
		if (CInterfaceExpr::eval(sSlot, evValue, NULL))
		{
			sint32 nSlot = (sint32)evValue.getInteger();

			if (nSlot > -1)
			{
				if (nSlot < sint32(EntitiesMngr.entities().size()) && EntitiesMngr.entities()[nSlot] != NULL)
				{
					nSlot = checkSlot(nSlot);

				}

				CBarManager::CBarInfo barInfo = CBarManager::getInstance()->getBarsByEntityId(nSlot);
				NLMISC::CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:BARS:TARTAR:HP", true);
				int value = barInfo.Score[SCORES::hit_points];

				if (value < 0) {
					value = 0;
				}

				node->setValue32(value);
				//ucstring msg = "Slot: " + toString(nSlot) + " Hp: " + toString(value);
				//CInterfaceManager::getInstance()->displaySystemInfo(msg, "SYS");
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CActionHandlerSetTarHp, "set_tar_hp");

// ***************************************************************************


