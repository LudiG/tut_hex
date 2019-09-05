/**
 * @file tut_hex.cpp
 * @date 17-Jul-2019
 */

#include "orx.h"


/* This is a basic C tutorial creating a viewport and an object.
 *
 * As orx is data driven, here we just write 2 lines of code to create a viewport
 * and an object. All their properties are defined in the config file (01_Object.ini).
 * As a matter of fact, the viewport is associated with a camera implicitly created from the
 * info given in the config file. You can also set their sizes, positions, the object colors,
 * scales, rotations, animations, physical properties, and so on. You can even request
 * random values for these without having to add a single line of code.
 * In a later tutorial we'll see how to generate your whole scene (all background
 * and landscape objects for example) with a simple for loop written in 3 lines of code.
 *
 * For now, you can try to uncomment some of the lines of 01_Object.ini, play with them,
 * then relaunch this tutorial. For an exhaustive list of options, please look at CreationTemplate.ini.
 */

orxFLOAT _screenHeight; // The screen height.
orxFLOAT _screenWidth; // The screen width.

orxFLOAT _tileRadius; // The tile radius in screen coordinates (pixels).

orxVECTOR _screenCoord; // The current screen coordinates of the mouse.
orxVECTOR _tilePos; // The current tile position.

#pragma mark - hexagon math

orxVECTOR cubeToAxial(const orxVECTOR& cube)
{
    orxVECTOR result;
    orxVector_Set(&result, cube.fX, cube.fY, 0.0);

    return result;
}

orxVECTOR axialToCube(const orxVECTOR& axial)
{
    orxFLOAT x = axial.fX;
    orxFLOAT y = axial.fY;

    orxFLOAT z = -x - y;

    orxVECTOR result;
    orxVector_Set(&result, x, y, z);

    return result;
}

orxVECTOR cubeRound(const orxVECTOR& cube)
{
    orxFLOAT rx = orxMath_Round(cube.fX);
    orxFLOAT ry = orxMath_Round(cube.fY);
    orxFLOAT rz = orxMath_Round(cube.fZ);

    orxFLOAT xDiff = orxMath_Abs(rx - cube.fX);
    orxFLOAT yDiff = orxMath_Abs(ry - cube.fY);
    orxFLOAT zDiff = orxMath_Abs(rz - cube.fZ);

    if ((xDiff > yDiff) && (xDiff > zDiff))
        rx = -ry - rz;

    else if (yDiff > zDiff)
        ry = -rx - rz;

    else
        rz = -rx - ry;

    orxVECTOR result;
    orxVector_Set(&result, rx, ry, rz);

    return result;
}

orxVECTOR axialRound(const orxVECTOR& axial)
{
    return cubeToAxial(cubeRound(axialToCube(axial)));
}

orxVECTOR pixelToHex_PointyTop(const orxVECTOR& point)
{
    orxFLOAT size = _tileRadius;

    orxVECTOR result;
    orxVector_Set(&result, ((orxMath_Pow(3.0, 0.5)/3.0 * point.fX) + (-1.0/3.0 * point.fY)) / size, (2.0/3.0 * point.fY) / size, 0.0);

    return result;
}

orxVECTOR pixelToHex_FlatTop(const orxVECTOR& point)
{
    orxFLOAT size = _tileRadius;

    orxVECTOR result;
    orxVector_Set(&result, (2.0/3.0 * point.fX) / size, ((-1.0/3.0 * point.fX) + (orxMath_Pow(3.0, 0.5)/3.0 * point.fY)) / size, 0.0);

    return result;
}

#pragma mark - orx

static orxSTATUS orxFASTCALL handleShaderEvent(const orxEVENT* currentEvent)
{
    switch(currentEvent->eID)
    {
        case orxSHADER_EVENT_SET_PARAM:
        {
            // Get the event payload.
            orxSHADER_EVENT_PAYLOAD *pstPayload = (orxSHADER_EVENT_PAYLOAD*)currentEvent->pstPayload;

            // look for the parameter of interest.
            if (!orxString_Compare(pstPayload->zParamName, "highlight"))
            {
                orxVector_Copy(&pstPayload->vValue, &_tilePos);
            }
        }
    }

    return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL Init()
{
    orxDisplay_GetScreenSize(&_screenWidth, &_screenHeight);

    orxConfig_PushSection("Shader");
    _tileRadius = orxConfig_GetFloat("radius");
    orxConfig_PopSection();

    orxViewport_CreateFromConfig("Viewport");
    orxObject_CreateFromConfig("Object");

    orxEvent_AddHandler(orxEVENT_TYPE_SHADER, handleShaderEvent);

    return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL Run()
{
    orxSTATUS result = orxSTATUS_SUCCESS;

    // INPUT: Quit
    if(orxInput_IsActive("Quit"))
    {
        result = orxSTATUS_FAILURE;
    }

    // INPUT: Mouse
    orxVECTOR mouse;
    orxMouse_GetPosition(&mouse);

    _screenCoord.fX = mouse.fX;
    _screenCoord.fY = _screenHeight - mouse.fY;
    _screenCoord.fZ = 0.0;

    // Calculate the tile position from the mouse position.
    orxVECTOR tilePosOld;
    orxVector_Copy(&tilePosOld, &_tilePos);
    _tilePos = axialRound(pixelToHex_FlatTop(_screenCoord));

    // DEBUG_PRINT
    if ((tilePosOld.fX != _tilePos.fX) || (tilePosOld.fY != _tilePos.fY))
    {
        orxLOG("TILE: %f, %f FOR SHADER: %f, %f.", _tilePos.fX, _tilePos.fY, _screenCoord.fX, _screenCoord.fY);
    }

    return result;
}

void orxFASTCALL Exit()
{
}

orxSTATUS orxFASTCALL Bootstrap()
{
    orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

    return orxSTATUS_SUCCESS;
}

int main(int argc, char** argv)
{
    orxConfig_SetBootstrap(Bootstrap);

    orx_Execute(argc, argv, Init, Run, Exit);

    return EXIT_SUCCESS;
}
