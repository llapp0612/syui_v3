
#include "action_handler_misc.h"
//#include "nel/gui/interface_expr.h"
#include "../entities.h"
#include "nel/gui/action_handler.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace BRICK_TYPE;

//extern NL3D::UDriver	*Driver;
//extern NLMISC::CLog		g_log;
//extern bool				ShowInterface;
//extern bool				ShowHelp;
//extern uint8			PlayerSelectedSlot;
//extern bool				IsInRingSession;

// *** check for user selection and remove title
class CSPPRemoveTitleAndCheckSelection : public CStringPostProcessRemoveTitle
{
public:

	sint32 Slot;

	bool cbIDStringReceived(ucstring &inout)
	{
		if (UserEntity != NULL)
		{
			if (UserEntity->selection() == Slot)
			{
				ucstring copyInout = inout;
				CStringPostProcessRemoveTitle::cbIDStringReceived(inout);
				if (inout.empty())
				{
					CEntityCL *entity = EntitiesMngr.entity(Slot);
					CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(entity);
					bool womanTitle = false;
					if (pChar != NULL)
						womanTitle = pChar->getGender() == GSGENDER::female;

					STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(copyInout), womanTitle);

					// Sometimes translation contains another title
					ucstring::size_type pos = copyInout.find('$');
					if (pos != ucstring::npos)
					{
						copyInout = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(copyInout), womanTitle);
					}

					CStringPostProcessRemoveTitle::cbIDStringReceived(copyInout);
					inout = copyInout;
				}

				return true;
			}
		}

		return false;
	}
};

// *** check for user selection and remove name (keep title)
class CSPPRemoveNameAndCheckSelection : public CStringPostProcessRemoveName
{
public:

	sint32 Slot;

	bool cbIDStringReceived(ucstring &inout)
	{
		if (UserEntity != NULL)
		{
			if (UserEntity->selection() == Slot)
			{
				CEntityCL *entity = EntitiesMngr.entity(Slot);
				CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(entity);
				if (pChar != NULL)
				{
					const CCharacterSheet *pSheet = pChar->getSheet();
					if (pSheet != NULL)
					{
						string sFame = pSheet->getFame();
						if (strnicmp(sFame.c_str(), "tribe_", 6) == 0)
						{
							inout = STRING_MANAGER::CStringManagerClient::getFactionLocalizedName(sFame);
							return true; // return tribe name
						}
					}
					Woman = pChar->getGender() == GSGENDER::female;
				}

				CStringPostProcessRemoveName::cbIDStringReceived(inout);

				return true; // return title
			}
		}
		return false;
	}
};

// ***************************************************************************
static bool findInterfacePath(string &sPath, CCtrlBase *pCaller)
{
	if (sPath.rfind(':') == string::npos)
	{
		if (pCaller == NULL) return false;
		sPath = pCaller->getId() + ":" + sPath;
	}
	else
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string elt = sPath.substr(0, sPath.rfind(':'));
		CInterfaceElement *pIE;
		if (pCaller != NULL)
			pIE = CWidgetManager::getInstance()->getElementFromId(pCaller->getId(), elt);
		else
			pIE = CWidgetManager::getInstance()->getElementFromId(elt);
		if (pIE == NULL) return false;
		sPath = pIE->getId() + ":" + sPath.substr(sPath.rfind(':') + 1, sPath.size());
	}
	return true;
}
