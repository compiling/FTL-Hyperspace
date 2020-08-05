#include "CustomCrewManifest.h"
#include "CustomShips.h"
#include "freetype.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>

CustomCrewManifest CustomCrewManifest::instance = CustomCrewManifest();

void CustomCrewManifest::OnInit(CrewManifest *manifest, ShipManager *ship)
{
    currentPage = 0;
    crewManifest = manifest;

    std::string buttonImg("upgradeUI/Equipment/button_crew_arrow");
    leftButton = new Button();
    leftButton->OnInit(buttonImg, 850, 139);

    rightButton = new Button();
    rightButton->OnInit(buttonImg, 885, 139);

    rightButton->bMirror = true;



    auto custom = CustomShipSelect::GetInstance();
    crewLimit = custom->GetDefaultDefinition().crewLimit;

    if (custom->HasCustomDef(ship->myBlueprint.blueprintName))
    {
        crewLimit = custom->GetDefinition(ship->myBlueprint.blueprintName).crewLimit;
    }

    crewEquipBoxes.clear();

    int boxX = crewManifest->position.x + 75;
    int boxY = crewManifest->position.y - 45;

    for (int i = 0; i < crewLimit; i++)
    {
        if (i % 8 == 0)
        {
            boxX = crewManifest->position.x + 75;
            boxY = crewManifest->position.y - 45;
            crewEquipBoxes.push_back(std::vector<CrewEquipBox*>());
        }

        if ((i % 8) % 3 == 0)
        {
            boxX = crewManifest->position.x + 75;
            boxY += 133;
        }

        if (i == crewLimit - ((crewLimit % 8) % 3))
        {
            if (crewLimit == i + 1)
            {
                boxX += 170;
            }
            else if (crewLimit == i + 2)
            {
                boxX += 86;
            }
        }
        else
        {
            if (i % 8 == 6)
            {
                boxX += 86;
            }
        }



        CrewEquipBox* box = new CrewEquipBox(Point(boxX, boxY), ship, i);

        crewEquipBoxes[std::floor(i / 8)].push_back(box);

        boxX += 170;
    }

    overCrewBox = new CrewEquipBox(Point(146, crewManifest->position.y + crewManifest->overBox.GetHeight() + crewManifest->overBox.position.y), ship, crewLimit);
}

void CustomCrewManifest::OnRender()
{
    CSurface::GL_PushMatrix();
    if (crewManifest->confirmingDelete >= 0)
    {
        CSurface::GL_SetColorTint(0.25f, 0.25f, 0.25f, 1.0f);
    }


    CSurface::GL_Translate(crewManifest->position.x, crewManifest->position.y);

    CSurface::GL_RenderPrimitive(crewManifest->box);


    TextLibrary* lib = G_->GetTextLibrary();
    freetype::easy_print(10, 25.f, 55.f, lib->GetText("rename_note"));




    if (!overCrewBox->IsEmpty())
    {
        crewManifest->overBox.OnRender();
    }

    CSurface::GL_PopMatrix();
    CSurface::GL_PushMatrix();

    int slot = 0;

    for (CrewEquipBox* i : GetPage(currentPage))
    {
        if (!i->IsEmpty())
        {
            if (crewManifest->confirmingDelete == slot)
            {
                CSurface::GL_RemoveColorTint();
            }
        }

        i->OnRender(false);
        i->RenderLabels(false, false);

        if (i->GetConfirmDelete())
        {
            crewManifest->confirmingDelete = slot;

            auto text = TextString();
            text.data = "confirm_dismiss";
            text.isLiteral = false;

            auto noText = TextString();
            noText.data = "";
            noText.isLiteral = true;

            crewManifest->deleteDialog.SetText(text, 300, false, noText, noText);
            crewManifest->deleteDialog.SetPosition(Point(i->location.x - 53, i->location.y + 86));
            crewManifest->deleteDialog.Open();
        }

        if (crewManifest->confirmingDelete == slot)
        {
            CSurface::GL_SetColorTint(0.25f, 0.25f, 0.25f, 1.f);
        }
        slot++;
    }

    if (!overCrewBox->IsEmpty())
    {
        if (crewManifest->confirmingDelete == crewLimit)
        {
            CSurface::GL_RemoveColorTint();
        }

        overCrewBox->OnRender(false);
        overCrewBox->RenderLabels(false, false);

        if (overCrewBox->GetConfirmDelete())
        {
            crewManifest->confirmingDelete = crewLimit;

            auto text = TextString();
            text.data = "confirm_dismiss";
            text.isLiteral = false;

            auto noText = TextString();
            noText.data = "";
            noText.isLiteral = true;

            crewManifest->deleteDialog.SetText(text, 300, false, noText, noText);
            crewManifest->deleteDialog.SetPosition(Point(overCrewBox->location.x - 53, overCrewBox->location.y + 86));
            crewManifest->deleteDialog.Open();
        }

        if (crewManifest->confirmingDelete == crewLimit)
        {
            CSurface::GL_SetColorTint(0.25f, 0.25f, 0.25f, 1.f);
        }
    }

    CSurface::GL_PopMatrix();


    if (crewLimit > 8)
    {
        leftButton->OnRender();
        rightButton->OnRender();
    }

    crewManifest->infoBox.OnRender();

    if (crewManifest->confirmingDelete >= 0)
    {
        CSurface::GL_RemoveColorTint();
        crewManifest->deleteDialog.OnRender();
    }
}

void CustomCrewManifest::Update()
{
    std::vector<CrewMember*> crewList = std::vector<CrewMember*>();

    G_->GetCrewFactory()->GetCrewList(&crewList, 0, false);

    int slot = 0;

    for (auto i : crewEquipBoxes)
    {
        for (auto box : i)
        {
            box->Restart();

            if (slot < crewList.size())
            {
                EquipmentBoxItem item = EquipmentBoxItem();
                item.pCrew = crewList[slot];

                box->AddItem(item);
                box->bDead = false;
            }

            slot++;
        }
    }

    overCrewBox->Restart();
    if (crewList.size() > crewLimit)
    {
        EquipmentBoxItem item = EquipmentBoxItem();
        item.pCrew = crewList[crewLimit];

        overCrewBox->AddItem(item);
        overCrewBox->bDead = false;
    }
}

void CustomCrewManifest::OnTextInput(SDLKey key)
{
    for (auto i : GetPage(currentPage))
    {
        if (!i->IsEmpty())
        {
            i->OnTextInput(key);
        }
    }

    if (!overCrewBox->IsEmpty())
    {
        overCrewBox->OnTextInput(key);
    }
}

void CustomCrewManifest::OnTextEvent(CEvent::TextEvent event)
{
    for (auto i : GetPage(currentPage))
    {
        if (!i->IsEmpty())
        {
            i->OnTextEvent(event);
        }
    }

    if (!overCrewBox->IsEmpty())
    {
        overCrewBox->OnTextEvent(event);
    }
}

void CustomCrewManifest::Close()
{
    for (auto i : GetPage(currentPage))
    {
        if (!i->IsEmpty())
        {
            i->CloseRename();
        }
    }

    if (!overCrewBox->IsEmpty())
    {
        overCrewBox->CloseRename();
    }
}

void CustomCrewManifest::MouseClick(int mX, int mY)
{
    if (crewManifest->confirmingDelete < 0)
    {
        for (auto i : GetPage(currentPage))
        {
            i->MouseClick(mX, mY);
        }
        if (!overCrewBox->IsEmpty())
        {
            overCrewBox->MouseClick(mX, mY);
        }
    }
    else
    {
        crewManifest->deleteDialog.MouseClick(mX, mY);

        if (!crewManifest->deleteDialog.bOpen)
        {
            if (crewManifest->deleteDialog.result)
            {
                if (crewManifest->confirmingDelete == crewLimit)
                {
                    overCrewBox->RemoveItem();
                }
                else
                {
                    GetPage(currentPage)[crewManifest->confirmingDelete]->RemoveItem();
                }
            }
            crewManifest->confirmingDelete = -1;
        }
    }

    if (crewLimit > 8)
    {
        if (leftButton->bActive && leftButton->bHover)
        {
            if (currentPage == 0)
            {
                currentPage = crewEquipBoxes.size() - 1;
            }
            else
            {
                currentPage--;
            }
        }

        if (rightButton->bActive && rightButton->bHover)
        {
            if (currentPage == crewEquipBoxes.size() - 1)
            {
                currentPage = 0;
            }
            else
            {
                currentPage++;
            }
        }
    }

    if (currentPage < 0)
    {
        currentPage = 0;
    }
    if (currentPage > crewEquipBoxes.size() - 1)
    {
        currentPage = crewEquipBoxes.size() - 1;
    }




    Update();
}

void CustomCrewManifest::MouseMove(int mX, int mY)
{
    crewManifest->infoBox.Clear();

    if (crewManifest->confirmingDelete >= 0)
    {
        crewManifest->deleteDialog.MouseMove(mX, mY);
    }
    else
    {
        for (auto i : GetPage(currentPage))
        {
            i->MouseMove(mX, mY);

            if (i->bMouseHovering && !i->IsEmpty())
            {
                i->bGlow = true;
                i->SetBlueprint(&crewManifest->infoBox, true);
            }
            else
            {
                i->bGlow = false;
            }
        }

        if (!overCrewBox->IsEmpty())
        {
            overCrewBox->MouseMove(mX, mY);

            if (overCrewBox->bMouseHovering)
            {
                overCrewBox->bGlow = true;
                overCrewBox->SetBlueprint(&crewManifest->infoBox, true);
            }
            else
            {
                overCrewBox->bGlow = false;
            }
        }
    }

    if (crewLimit > 8)
    {
        leftButton->MouseMove(mX, mY, false);
        rightButton->MouseMove(mX, mY, false);
    }
}

HOOK_METHOD(CrewManifest, OnInit, (ShipManager *ship) -> void)
{
    super(ship);
    CustomCrewManifest::GetInstance()->OnInit(this, ship);
}

HOOK_METHOD(CrewManifest, OnRender, () -> void)
{
    //super();

    CustomCrewManifest::GetInstance()->OnRender();
}

HOOK_METHOD(CrewManifest, Update, () -> void)
{
    CustomCrewManifest::GetInstance()->Update();
}

HOOK_METHOD(CrewManifest, OnTextInput, (SDLKey key) -> void)
{
    CustomCrewManifest::GetInstance()->OnTextInput(key);
}


HOOK_METHOD(CrewManifest, OnTextEvent, (CEvent::TextEvent event) -> void)
{
    CustomCrewManifest::GetInstance()->OnTextEvent(event);
}

HOOK_METHOD(CrewManifest, Close, () -> void)
{
    super();
    CustomCrewManifest::GetInstance()->Close();
}


HOOK_METHOD(CrewManifest, MouseClick, (int mX, int mY) -> void)
{
    CustomCrewManifest::GetInstance()->MouseClick(mX, mY);
}

HOOK_METHOD(CrewManifest, MouseMove, (int mX, int mY) -> void)
{
    CustomCrewManifest::GetInstance()->MouseMove(mX, mY);
}

static bool forceNextSlot = false;

HOOK_METHOD(CrewStoreBox, Purchase, () -> void)
{
    forceNextSlot = true;
    super();
    forceNextSlot = false;
}

HOOK_METHOD(ShipManager, AddCrewMemberFromBlueprint, (CrewBlueprint* bp, int slot, bool init, int roomId, bool intruder) -> CrewMember*)
{
    if (forceNextSlot)
    {
        slot = -1;
    }

    auto ret = super(bp, slot, init, roomId, intruder);

    return ret;
}