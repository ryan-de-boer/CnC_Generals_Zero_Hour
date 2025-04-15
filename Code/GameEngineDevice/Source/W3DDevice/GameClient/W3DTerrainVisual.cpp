/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DTerrainVisual.cpp /////////////////////////////////////////////////////////////////////
// W3D implementation details for visual aspects of terrain
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/ThingTemplate.h"
#include "Common/WellKnownKeys.h"
#include "Common/TerrainTypes.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DDebugIcons.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DGranny.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/heightmap.h"
#include "WW3D2/Light.h"
#include "WW3D2/RendObj.h"
#include "WW3D2/ColType.h"
#include "WW3D2/ColTest.h"
#include "WW3D2/assetmgr.h"

#include "Common/UnitTimings.h" //Contains the DO_UNIT_TIMINGS define jba.		 
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTerrainVisual::W3DTerrainVisual()
{

	m_terrainRenderObject = NULL;
	m_terrainHeightMap = NULL;
	m_waterRenderObject = NULL;
	TheWaterRenderObj = NULL;

}  // end W3DTerrainVisual

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTerrainVisual::~W3DTerrainVisual()
{
	// release our render object
	if (TheTerrainRenderObject == m_terrainRenderObject) {
		TheTerrainRenderObject = NULL;
	}

	if (TheTerrainTracksRenderObjClassSystem)
	{
		delete TheTerrainTracksRenderObjClassSystem;
		TheTerrainTracksRenderObjClassSystem=NULL;
	}

#ifdef	INCLUDE_GRANNY_IN_BUILD
	if (TheGrannyRenderObjSystem)
	{
		delete TheGrannyRenderObjSystem;
		TheGrannyRenderObjSystem=NULL;
	}
#endif

	if (TheW3DShadowManager)
	{	
		delete TheW3DShadowManager;
		TheW3DShadowManager=NULL;
	}

	REF_PTR_RELEASE( m_waterRenderObject );
	TheWaterRenderObj=NULL;
	REF_PTR_RELEASE( m_terrainRenderObject );
	REF_PTR_RELEASE( m_terrainHeightMap );
}  // end ~W3DTerrainVisual

//-------------------------------------------------------------------------------------------------
/** init */
//-------------------------------------------------------------------------------------------------
void W3DTerrainVisual::init( void )
{

	// extend
	TerrainVisual::init();
	// create a new render object for W3D
	m_terrainRenderObject = NEW_REF( HeightMapRenderObjClass, () );
	m_terrainRenderObject->Set_Collision_Type( PICK_TYPE_TERRAIN );
	TheTerrainRenderObject = m_terrainRenderObject;

	// initialize track drawing system
	TheTerrainTracksRenderObjClassSystem = NEW TerrainTracksRenderObjClassSystem;
	TheTerrainTracksRenderObjClassSystem->init(W3DDisplay::m_3DScene);

#ifdef	INCLUDE_GRANNY_IN_BUILD
	// initialize Granny model drawing system
	TheGrannyRenderObjSystem = NEW GrannyRenderObjSystem;
#endif

	// initialize object shadow drawing system
	TheW3DShadowManager = NEW W3DShadowManager;
 	TheW3DShadowManager->init();
	
	// create a water plane render object
	TheWaterRenderObj=m_waterRenderObject = NEW_REF( WaterRenderObjClass, () );
	m_waterRenderObject->init(TheGlobalData->m_waterPositionZ, TheGlobalData->m_waterExtentX, TheGlobalData->m_waterExtentY, W3DDisplay::m_3DScene, (WaterRenderObjClass::WaterType)TheGlobalData->m_waterType);	//create a water plane that's 128x128 units
	m_waterRenderObject->Set_Position(Vector3(TheGlobalData->m_waterPositionX,TheGlobalData->m_waterPositionY,TheGlobalData->m_waterPositionZ));	//place water in world

#ifdef DO_UNIT_TIMINGS
#pragma MESSAGE("********************* WARNING- Doing UNIT TIMINGS. ")
#else 
		if (TheGlobalData->m_waterType == WaterRenderObjClass::WATER_TYPE_1_FB_REFLECTION)
		{	// add water render object to the pre-pass scene (to be rendered before main scene)
 			//W3DDisplay::m_prePass3DScene->Add_Render_Object( m_waterRenderObject);
		}
		else
		{	// add water render object to the post-pass scene (to be rendered after main scene)
			W3DDisplay::m_3DScene->Add_Render_Object( m_waterRenderObject);
		}
#endif
	if (TheGlobalData->m_useCloudPlane)
		m_waterRenderObject->toggleCloudLayer(true);
	else
		m_waterRenderObject->toggleCloudLayer(false);

	// set the vertex animated water properties
	Int waterSettingIndex = 0;  // use index 0 settings by default
	TheTerrainVisual->setWaterGridHeightClamps( NULL, 
																							TheGlobalData->m_vertexWaterHeightClampLow[ waterSettingIndex ], 
																							TheGlobalData->m_vertexWaterHeightClampHi[ waterSettingIndex ] );
	TheTerrainVisual->setWaterTransform( NULL, 
																			 TheGlobalData->m_vertexWaterAngle[ waterSettingIndex ], 
																			 TheGlobalData->m_vertexWaterXPosition[ waterSettingIndex ], 
																			 TheGlobalData->m_vertexWaterYPosition[ waterSettingIndex ], 
																			 TheGlobalData->m_vertexWaterZPosition[ waterSettingIndex ] );
	TheTerrainVisual->setWaterGridResolution( NULL, 
																						TheGlobalData->m_vertexWaterXGridCells[ waterSettingIndex ], 
																						TheGlobalData->m_vertexWaterYGridCells[ waterSettingIndex ], 
																						TheGlobalData->m_vertexWaterGridSize[ waterSettingIndex ] );
	TheTerrainVisual->setWaterAttenuationFactors( NULL, 
																								TheGlobalData->m_vertexWaterAttenuationA[ waterSettingIndex ], 
																								TheGlobalData->m_vertexWaterAttenuationB[ waterSettingIndex ], 
																								TheGlobalData->m_vertexWaterAttenuationC[ waterSettingIndex ], 
																								TheGlobalData->m_vertexWaterAttenuationRange[ waterSettingIndex ] );	
	m_isWaterGridRenderingEnabled = FALSE;

}  // end init

//-------------------------------------------------------------------------------------------------
/** reset */
//-------------------------------------------------------------------------------------------------
void W3DTerrainVisual::reset( void )
{

	// extend
	TerrainVisual::reset();

	m_terrainRenderObject->reset();

	if (TheW3DShadowManager)
		TheW3DShadowManager->Reset();

	if (TheTerrainTracksRenderObjClassSystem)
		TheTerrainTracksRenderObjClassSystem->Reset();

	// reset water render object if present
	if( m_waterRenderObject )
	{
		for (Int i=0; i<5; i++)
		{
			//check if this texture was ever changed from default
			if (m_currentSkyboxTexNames[i] != m_initialSkyboxTexNames[i])
			{
				m_waterRenderObject->replaceSkyboxTexture(m_currentSkyboxTexNames[i], m_initialSkyboxTexNames[i]);
				m_currentSkyboxTexNames[i]=m_initialSkyboxTexNames[i];	//update current state to new texture
			}
		}

		m_waterRenderObject->reset();
	}

}  // end reset

//-------------------------------------------------------------------------------------------------
/** update */
//-------------------------------------------------------------------------------------------------
void W3DTerrainVisual::update( void )
{

	// extend
	TerrainVisual::update();

	// if we have a water render object, it has an update method
	if( m_waterRenderObject )
		m_waterRenderObject->update();

}  // end update

typedef struct {
	char fourCC[4];   // "DDS "
	uint32_t size;     // Size of the header (124 bytes)
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchOrLinearSize;
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved1[11];
	struct {
		uint32_t size;
		uint32_t flags;
		char fourCC[4];
		uint32_t rgbBitCount;
		uint32_t rBitMask;
		uint32_t gBitMask;
		uint32_t bBitMask;
		uint32_t aBitMask;
	} pixelFormat;
	struct {
		uint32_t caps1;
		uint32_t caps2;
		uint32_t caps3;
		uint32_t caps4;
	} caps;
	uint32_t reserved2;
} DDSHeader;

bool SaveTextureToDDSFile(IDirect3DTexture8* texture, const char* filename) {
	if (!texture || !filename) return false;

	D3DSURFACE_DESC desc;
	texture->GetLevelDesc(0, &desc);

	if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8 &&
		desc.Format != D3DFMT_A1R5G5B5) {
		printf("Unsupported format. Only D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, and D3DFMT_A1R5G5B5 are supported.\n");
		return false;
	}

	FILE* file = fopen(filename, "wb");
	if (!file) return false;

	DDSHeader header = { 0 };
	memcpy(header.fourCC, "DDS ", 4);
	header.size = 124;
	header.flags = 0x00021007;
	header.height = desc.Height;
	header.width = desc.Width;
	header.pixelFormat.size = 32;
	header.caps.caps1 = 0x1000;

	if (desc.Format == D3DFMT_A1R5G5B5) {
		header.pitchOrLinearSize = desc.Width * 2;  // 16-bit per pixel
		header.pixelFormat.flags = 0x41;            // DDPF_RGB | DDPF_ALPHAPIXELS
		header.pixelFormat.rgbBitCount = 16;
		header.pixelFormat.rBitMask = 0x7C00;
		header.pixelFormat.gBitMask = 0x03E0;
		header.pixelFormat.bBitMask = 0x001F;
		header.pixelFormat.aBitMask = 0x8000;
	}
	else {
		header.pitchOrLinearSize = desc.Width * 4;
		header.pixelFormat.flags = 0x41;
		header.pixelFormat.rgbBitCount = 32;
		header.pixelFormat.rBitMask = 0x00FF0000;
		header.pixelFormat.gBitMask = 0x0000FF00;
		header.pixelFormat.bBitMask = 0x000000FF;
		header.pixelFormat.aBitMask = 0xFF000000;
	}

	fwrite(&header, sizeof(header), 1, file);

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(texture->LockRect(0, &lockedRect, NULL, D3DLOCK_READONLY))) {
		for (UINT y = 0; y < desc.Height; ++y) {
			fwrite((BYTE*)lockedRect.pBits + y * lockedRect.Pitch, 1, header.pitchOrLinearSize, file);
		}
		texture->UnlockRect(0);
	}

	fclose(file);
	return true;
}

extern std::vector<UnsignedShort> g_indices;

void WriteObjFile();
void writeOgreXml();
void writeOBJE();
void writeFBXCube();
void writeFBXTerrain();

void W3DTerrainVisual::saveHeightMap()
{
	char msg[1000];
	strcpy(msg, "");
	sprintf(msg, "%d x %d, size: %d, random %d, alpha file path: %s, indices: %d", m_terrainHeightMap->m_width, m_terrainHeightMap->m_height, m_terrainHeightMap->m_dataSize, m_terrainHeightMap->m_data[100], m_terrainHeightMap->getAlphaTerrainTexture()->Get_Texture_Name(), g_indices.size());

	WriteObjFile();
	writeOgreXml();
	writeOBJE();
	writeFBXCube();
	writeFBXTerrain();

	IDirect3DTexture8* tex = m_terrainHeightMap->getTerrainTexture()->Peek_DX8_Texture();
	SaveTextureToDDSFile(tex, "terrain_tex.dds");

	IDirect3DTexture8* alphaTex = m_terrainHeightMap->getAlphaTerrainTexture()->Peek_DX8_Texture();
	SaveTextureToDDSFile(alphaTex, "alpha_tex.dds");

	IDirect3DTexture8* edgeTex = m_terrainHeightMap->getEdgeTerrainTexture()->Peek_DX8_Texture();
	SaveTextureToDDSFile(edgeTex, "edge_tex.dds");

	MessageBox(NULL, msg, "info", NULL);

	// Define the file name
	const char* filename = "output.ppm";

	// Open file for writing
	FILE* file = fopen(filename, "w");
	if (!file) {
		perror("Failed to open file for writing");
		return;
	}

	// Image parameters
	int width = m_terrainHeightMap->m_width;
	int height = m_terrainHeightMap->m_height;
	int maxColorValue = 255;

	// Write the PPM header
	fprintf(file, "P3\n");
	fprintf(file, "%d %d\n", width, height);
	fprintf(file, "%d\n", maxColorValue);

	// Define pixel data (R, G, B for each pixel)
	// Write pixel data
	// Iterate over rows in **reverse order** to flip vertically
	for (int y = height - 1; y >= 0; y--) {
		for (int x = 0; x < width; x++) {
			// Calculate the index for the current pixel
			int index = y * width + x;

			// Make sure we don't read out of bounds
			if (index >= width * height) {
				printf("Error: Index out of bounds at pixel (%d, %d).\n", x, y);
				fclose(file);
				return;
			}

			// Extract the grayscale value
			int gray = m_terrainHeightMap->m_data[index];  // This is a single UnsignedByte value

			// Write grayscale value as RGB (R=G=B)
			fprintf(file, "%d %d %d ", gray, gray, gray);
		}
		fprintf(file, "\n"); // Newline at the end of each row
	}

	// Close the file
	fclose(file);

	printf("PPM file '%s' written successfully.\n", filename);
}

//-------------------------------------------------------------------------------------------------
/** load method for W3D visual terrain */
//-------------------------------------------------------------------------------------------------
Bool W3DTerrainVisual::load( AsciiString filename )
{
	
#if 0	
	// (gth) Testing exclusion list asset releasing
	DynamicVectorClass<StringClass> exclusion_list(8000);
	
	WW3DAssetManager::Get_Instance()->Create_Asset_List(exclusion_list);

	exclusion_list.Add(StringClass("avcomanche"));
	exclusion_list.Add(StringClass("avcomanche_d"));
	exclusion_list.Add(StringClass("ptdogwood08"));
	exclusion_list.Add(StringClass("ptdogwood01_b"));
	exclusion_list.Add(StringClass("ptpalm01"));
	exclusion_list.Add(StringClass("ptpalm01_b"));
	exclusion_list.Add(StringClass("avhummer"));
	exclusion_list.Add(StringClass("avhummer_d"));
	exclusion_list.Add(StringClass("avleopard"));
	exclusion_list.Add(StringClass("avleopard_d"));

	WW3DAssetManager::Get_Instance()->Free_Assets_With_Exclusion_List(exclusion_list);
#endif

	// enhancing functionality specific for W3D terrain
	if( TerrainVisual::load( filename ) == FALSE )
		return FALSE;  // failed

	// open the terrain file
	CachedFileInputStream fileStrm;
	if( !fileStrm.open(filename) )
	{

		REF_PTR_RELEASE( m_terrainRenderObject );
		return FALSE;

	}  // end if

	if( m_terrainRenderObject == NULL )
		return FALSE;

	REF_PTR_RELEASE( m_terrainHeightMap );
	ChunkInputStream *pStrm = &fileStrm;
	// allocate new height map data to read from file
	m_terrainHeightMap = NEW WorldHeightMap(pStrm);
	TheWritableGlobalData->addHeightMapSaver(this);

	// Add any lights loaded by map.
	MapObject *pMapObj = MapObject::getFirstMapObject();
	while (pMapObj) 
	{
		Dict *d = pMapObj->getProperties();
		if (pMapObj->isLight()) 
		{ 
			Coord3D loc = *pMapObj->getLocation();
			if (loc.z < 0) {
				Vector3 vec;
				loc.z = m_terrainRenderObject->getHeightMapHeight(loc.x, loc.y, NULL);
				loc.z += d->getReal(TheKey_lightHeightAboveTerrain);
			}
			// It is a light, and handled at the device level.  jba.
			LightClass* lightP = NEW_REF(LightClass, (LightClass::POINT));

			RGBColor c;
			c.setFromInt(d->getInt(TheKey_lightAmbientColor));
			lightP->Set_Ambient( Vector3( c.red, c.green, c.blue ) );

			c.setFromInt(d->getInt(TheKey_lightDiffuseColor));
			lightP->Set_Diffuse( Vector3(  c.red, c.green, c.blue) );

			lightP->Set_Position(Vector3(loc.x, loc.y, loc.z));

			lightP->Set_Far_Attenuation_Range(d->getReal(TheKey_lightInnerRadius), d->getReal(TheKey_lightOuterRadius));
 			W3DDisplay::m_3DScene->Add_Render_Object(lightP);
			REF_PTR_RELEASE( lightP );
		}
		pMapObj = pMapObj->getNext();
	}


	RefRenderObjListIterator *it = W3DDisplay::m_3DScene->createLightsIterator();
	// apply the heightmap to the terrain render object
	m_terrainRenderObject->initHeightData( m_terrainHeightMap->getDrawWidth(), 
																				 m_terrainHeightMap->getDrawHeight(),
																				 m_terrainHeightMap,
																				 it);
	if (it) {
	 W3DDisplay::m_3DScene->destroyLightsIterator(it);
	 it = NULL;
	}
	// add our terrain render object to the scene
	W3DDisplay::m_3DScene->Add_Render_Object( m_terrainRenderObject );

#if defined _DEBUG || defined _INTERNAL
	// Icon drawing utility object for pathfinding.
	W3DDebugIcons *icons = NEW W3DDebugIcons;
 	W3DDisplay::m_3DScene->Add_Render_Object( icons );
	icons->Release_Ref(); // belongs to scene.
#endif

#ifdef DO_UNIT_TIMINGS
#pragma MESSAGE("********************* WARNING- Doing UNIT TIMINGS. ")
#else 
	if (m_waterRenderObject)
	{
		W3DDisplay::m_3DScene->Add_Render_Object( m_waterRenderObject);
		m_waterRenderObject->enableWaterGrid(false);
	}
#endif

	pMapObj = MapObject::getFirstMapObject();
	while (pMapObj) 
	{
		Dict *d = pMapObj->getProperties();
		if (pMapObj->isScorch()) {
			const Coord3D *pos = pMapObj->getLocation();
			Vector3 loc(pos->x, pos->y, pos->z);
			Real radius = d->getReal(TheKey_objectRadius);
			Scorches type = (Scorches)d->getInt(TheKey_scorchType);
			m_terrainRenderObject->addScorch(loc, radius, type);
		}
		pMapObj = pMapObj->getNext();
	}

	// reset water render object if present
	if( m_waterRenderObject )
	{
		m_waterRenderObject->load();
	}

	return TRUE;  // success

}  // end load

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::enableWaterGrid( Bool enable )
{

	//Get default water type
	m_isWaterGridRenderingEnabled = enable;

	// make the changes in the water render object
	if( m_waterRenderObject )
		m_waterRenderObject->enableWaterGrid( enable );

}  // end enableWaterGrid

//-------------------------------------------------------------------------------------------------
/** intersect the ray with the terrain, if a hit occurs TRUE is returned
	* and the result point on the terrain is returned in "result" */
//-------------------------------------------------------------------------------------------------
Bool W3DTerrainVisual::intersectTerrain( Coord3D *rayStart, 
																				 Coord3D *rayEnd, 
																				 Coord3D *result )
{
	Bool hit = FALSE;

	// sanity
	if( rayStart == NULL || rayEnd == NULL )
		return hit;

	if( m_terrainRenderObject )
	{
		CastResultStruct res;
		LineSegClass lineSeg( Vector3( rayStart->x, rayStart->y, rayStart->z ),
													Vector3( rayEnd->x, rayEnd->y, rayEnd->z ) );
		RayCollisionTestClass rayTest( lineSeg, &res );

		hit = m_terrainRenderObject->Cast_Ray( rayTest );
		if( hit && result )
		{
			Vector3 point = rayTest.Result->ContactPoint;

			result->x = point.X;
			result->y = point.Y;
			result->z = point.Z;	

		}  // end if
		
	}  // end if

	// return hit result
	return hit;

}  // end intersectTerrain

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTerrainVisual::getTerrainColorAt( Real x, Real y, RGBColor *pColor )
{

	if( m_terrainHeightMap )
		m_terrainHeightMap->getTerrainColorAt( x, y, pColor );

}  // end getTerrainColorAt

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainType *W3DTerrainVisual::getTerrainTile( Real x, Real y )
{
	TerrainType *tile = NULL;

	if( m_terrainHeightMap )
	{
		AsciiString tileName = m_terrainHeightMap->getTerrainNameAt( x, y );

		tile = TheTerrainTypes->findTerrain( tileName );

	}  // end if

	return tile;

}  // end getTerrainTile

// ------------------------------------------------------------------------------------------------
/** set min/max height values allowed in water grid pointed to by waterTable */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setWaterGridHeightClamps( const WaterHandle *waterTable, 
																								 Real minZ, Real maxZ )
{

	if( m_waterRenderObject )
		m_waterRenderObject->setGridHeightClamps( minZ, maxZ );

}  // end setWaterGridHeightClamps

// ------------------------------------------------------------------------------------------------
/** adjust fallof parameters for grid change method */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setWaterAttenuationFactors( const WaterHandle *waterTable, 
																									 Real a, Real b, Real c, Real range )
{

	if( m_waterRenderObject )
		m_waterRenderObject->setGridChangeAttenuationFactors( a, b, c, range );

}  // end setWaterAttenuationFactors

// ------------------------------------------------------------------------------------------------
/** set the water table position and orientation in world space */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setWaterTransform( const WaterHandle *waterTable, 
																					Real angle, Real x, Real y, Real z )
{

	if( m_waterRenderObject )
		m_waterRenderObject->setGridTransform( angle, x, y, z );

}  // end setWaterTransform

// ------------------------------------------------------------------------------------------------
/** set water table transform by matrix */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setWaterTransform( const Matrix3D *transform )
{

	if( m_waterRenderObject )
		m_waterRenderObject->setGridTransform( transform );
			
}  // end setWaterTransform

// ------------------------------------------------------------------------------------------------
/** get the water transform matrix */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::getWaterTransform( const WaterHandle *waterTable, Matrix3D *transform )
{

	if( m_waterRenderObject )
		m_waterRenderObject->getGridTransform( transform );

}  // end getWaterTransform

// ------------------------------------------------------------------------------------------------
/** water grid resolution spacing */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setWaterGridResolution( const WaterHandle *waterTable,
																							 Real gridCellsX, Real gridCellsY, Real cellSize )
{

	if( m_waterRenderObject )
		m_waterRenderObject->setGridResolution( gridCellsX, gridCellsY, cellSize );

}  // end setWaterGridResolution

// ------------------------------------------------------------------------------------------------
/** get water grid resolution spacing */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::getWaterGridResolution( const WaterHandle *waterTable,	
																							 Real *gridCellsX, Real *gridCellsY, Real *cellSize )
{

	if( m_waterRenderObject )
		m_waterRenderObject->getGridResolution( gridCellsX, gridCellsY, cellSize );

}  // end getWaterGridResolution

// ------------------------------------------------------------------------------------------------
/** adjust the water grid in world coords by the delta */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::changeWaterHeight( Real x, Real y, Real delta )
{

	if( m_waterRenderObject )
		m_waterRenderObject->changeGridHeight( x, y, delta );

}  // end changeWaterHeight

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::addWaterVelocity( Real worldX, Real worldY, 
																				 Real velocity, Real preferredHeight )
{

	if( m_waterRenderObject )
		m_waterRenderObject->addVelocity( worldX, worldY, velocity, preferredHeight );

}  // end addWaterVelocity

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool W3DTerrainVisual::getWaterGridHeight( Real worldX, Real worldY, Real *height)
{
	Real gridX, gridY;

	if (m_isWaterGridRenderingEnabled && m_waterRenderObject &&
		m_waterRenderObject->worldToGridSpace(worldX, worldY, gridX, gridY))
	{	//point falls within grid, return correct height
		m_waterRenderObject->getGridVertexHeight(REAL_TO_INT(gridX),REAL_TO_INT(gridY),height);
		return TRUE;
	}
	return FALSE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setRawMapHeight(const ICoord2D *gridPos, Int height)
{
	if (m_terrainHeightMap) {
		Int x = gridPos->x+m_terrainHeightMap->getBorderSize();
		Int y = gridPos->y+m_terrainHeightMap->getBorderSize();
 		//if (m_terrainHeightMap->getHeight(x,y) != height) //ML changed to prevent scissoring with roads
 		if (m_terrainHeightMap->getHeight(x,y) > height) 
		{
			m_terrainHeightMap->setRawHeight(x, y, height);
			m_terrainRenderObject->staticLightingChanged();
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::addFactionBibDrawable(Drawable *factionBuilding, Bool highlight, Real extra)
{
	if (m_terrainHeightMap) {
		const Matrix3D * mtx = factionBuilding->getTransformMatrix();
		Vector3 corners[4];
		Coord3D pos;
		pos.set(0,0,0);
		Real exitWidth = factionBuilding->getTemplate()->getFactoryExitWidth();
		Real extraWidth = factionBuilding->getTemplate()->getFactoryExtraBibWidth() + extra;
		const GeometryInfo info = factionBuilding->getTemplate()->getTemplateGeometryInfo();
		Real sizeX = info.getMajorRadius();
		Real sizeY = info.getMinorRadius();
		if (info.getGeomType() != GEOMETRY_BOX) {
			sizeY = sizeX;
		}
		corners[0].Set(pos.x, pos.y, pos.z);
		corners[0].X -= sizeX+extraWidth;
		corners[0].Y -= sizeY+extraWidth;
		corners[1].Set(pos.x, pos.y, pos.z);
		corners[1].X += sizeX+exitWidth+extraWidth;
		corners[1].Y -= sizeY+extraWidth;
		corners[2].Set(pos.x, pos.y, pos.z);
		corners[2].X += sizeX+exitWidth+extraWidth;
		corners[2].Y += sizeY+extraWidth;
		corners[3].Set(pos.x, pos.y, pos.z);
		corners[3].X -= sizeX+extraWidth;
		corners[3].Y += sizeY+extraWidth;
		mtx->Transform_Vector(*mtx, corners[0], &corners[0]);
		mtx->Transform_Vector(*mtx, corners[1], &corners[1]);
		mtx->Transform_Vector(*mtx, corners[2], &corners[2]);
		mtx->Transform_Vector(*mtx, corners[3], &corners[3]);
		m_terrainRenderObject->addTerrainBibDrawable(corners, factionBuilding->getID(), highlight);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::addFactionBib(Object *factionBuilding, Bool highlight, Real extra)
{
	if (m_terrainHeightMap) {
		const Matrix3D * mtx = factionBuilding->getTransformMatrix();
		Vector3 corners[4];
		Coord3D pos;
		pos.set(0,0,0);
		Real exitWidth = factionBuilding->getTemplate()->getFactoryExitWidth();
		Real extraWidth = factionBuilding->getTemplate()->getFactoryExtraBibWidth() + extra;
		const GeometryInfo info = factionBuilding->getGeometryInfo();
		Real sizeX = info.getMajorRadius();
		Real sizeY = info.getMinorRadius();
		if (info.getGeomType() != GEOMETRY_BOX) {
			sizeY = sizeX;
		}
		corners[0].Set(pos.x, pos.y, pos.z);
		corners[0].X -= sizeX+extraWidth;
		corners[0].Y -= sizeY+extraWidth;
		corners[1].Set(pos.x, pos.y, pos.z);
		corners[1].X += sizeX+exitWidth+extraWidth;
		corners[1].Y -= sizeY+extraWidth;
		corners[2].Set(pos.x, pos.y, pos.z);
		corners[2].X += sizeX+exitWidth+extraWidth;
		corners[2].Y += sizeY+extraWidth;
		corners[3].Set(pos.x, pos.y, pos.z);
		corners[3].X -= sizeX+extraWidth;
		corners[3].Y += sizeY+extraWidth;
		mtx->Transform_Vector(*mtx, corners[0], &corners[0]);
		mtx->Transform_Vector(*mtx, corners[1], &corners[1]);
		mtx->Transform_Vector(*mtx, corners[2], &corners[2]);
		mtx->Transform_Vector(*mtx, corners[3], &corners[3]);
		m_terrainRenderObject->addTerrainBib(corners, factionBuilding->getID(), highlight);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::removeFactionBibDrawable(Drawable *factionBuilding)
{
	if (m_terrainHeightMap) {
		m_terrainRenderObject->removeTerrainBibDrawable(factionBuilding->getID());
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::removeFactionBib(Object *factionBuilding)
{
	if (m_terrainHeightMap) {
		m_terrainRenderObject->removeTerrainBib(factionBuilding->getID());
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::removeAllBibs(void)
{
	if (m_terrainHeightMap) {
		m_terrainRenderObject->removeAllTerrainBibs();
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::removeBibHighlighting(void)
{
	if (m_terrainHeightMap) {
		m_terrainRenderObject->removeTerrainBibHighlighting();
	}
}
 
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setTerrainTracksDetail(void)
{
	if (TheTerrainTracksRenderObjClassSystem)
		TheTerrainTracksRenderObjClassSystem->setDetail();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::setShoreLineDetail(void)
{
	if (m_terrainRenderObject) 
		m_terrainRenderObject->setShoreLineDetail();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/// Replace the skybox texture
void W3DTerrainVisual::replaceSkyboxTextures(const AsciiString *oldTexName[5], const AsciiString *newTexName[5])
{
	if (m_waterRenderObject)
	{
		for (Int i=0; i<5; i++)
		{
			//check if this texture was never changed before and is still using the default art.
			if (m_initialSkyboxTexNames[i].isEmpty())
			{	m_initialSkyboxTexNames[i]=*oldTexName[i];
				m_currentSkyboxTexNames[i]=*oldTexName[i];
			}

			if (m_currentSkyboxTexNames[i] != *newTexName[i])
			{	m_waterRenderObject->replaceSkyboxTexture(m_currentSkyboxTexNames[i], *newTexName[i]);
				m_currentSkyboxTexNames[i]=*newTexName[i];	//update current state to new texture
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::crc( Xfer *xfer )
{

	// extend base class
	TerrainVisual::crc( xfer );

}  // end CRC

// ------------------------------------------------------------------------------------------------
/** Xfer
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	TerrainVisual::xfer( xfer );

	// flag for whether or not the water grid is enabled
	Bool gridEnabled = m_isWaterGridRenderingEnabled;
	xfer->xferBool( &gridEnabled );
	if( gridEnabled != m_isWaterGridRenderingEnabled )
	{

		DEBUG_CRASH(( "W3DTerrainVisual::xfer - m_isWaterGridRenderingEnabled mismatch\n" ));
		throw SC_INVALID_DATA;

	}  // end if

	// xfer grid data if enabled
	if( gridEnabled )
		xfer->xferSnapshot( m_waterRenderObject );

/*
	{

		// grid width and height
		Int width = getGridWidth();
		Int height = getGridheight();
		xfer->xferInt( &width );
		xfer->xferInt( &height );
		if( width != getGridWidth() )
		{

			DEBUG_CRASH(( "W3DTerainVisual::xfer - grid width mismatch '%d' should be '%d'\n",
										width, getGridWidth() ));
			throw SC_INVALID_DATA;

		}  // end if
		if( height != getGridHeight() )
		{

			DEBUG_CRASH(( "W3DTerainVisual::xfer - grid height mismatch '%d' should be '%d'\n",
										height, getGridHeight() ));
			throw SC_INVALID_DATA;

		}  // end if

		// write data for each grid

	}  // end if
*/

	// Write out the terrain height data.
	if (version >= 2) {
		UnsignedByte *data = m_terrainHeightMap->getDataPtr();
		Int len = m_terrainHeightMap->getXExtent()*m_terrainHeightMap->getYExtent();
		Int xferLen = len;
		xfer->xferInt(&xferLen);
		if (len!=xferLen) {
			DEBUG_CRASH(("Bad height map length."));
			if (len>xferLen) {
				len = xferLen;
			}
		}
		xfer->xferUser(data, len);	
		if (xfer->getXferMode() == XFER_LOAD)	{	
			// Update the display height map.
			m_terrainRenderObject->staticLightingChanged();
		}
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DTerrainVisual::loadPostProcess( void )
{

	// extend base class
	TerrainVisual::loadPostProcess();

}  // end loadPostProcess

