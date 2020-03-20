/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
	
	Alessandro Santo Rizzello - arizzello@essenza.ws    20200320
	
*/

#ifndef __PHOTON__
#define __PHOTON__

#include <stdint.h>

struct HEADER {
        uint8_t unknown0[8]; //File type ID???
        //Build volume definition
        float sizeX;
        float sizeY;
        float sizeZ;
        int padding0[3];
        float layerThickness;
        float normalExposure;
        float bottomExposure;
        float offTime;
        int nBottomLayers; //Haven't found where these layers show up
        int resolutionX;
        int resolutionY;
        int preview1Offset;
        int layerDefsStart;
        int nLayers;
        int preview2Offset;
        int unknown6; //padding???
        int unknown7; //padding???
        int endofpreview2;
        int padding1[6];//00s padding?
    };

struct previewImage{
    int width;
    int height;

    int dataStartPos;
    int dataSize;

    int padding[4];
};

struct layerdata{
    float layerHeight; // thickness
    float exposureTime; // secs
    int offTime;  // secs

    int dataStartPos;
    int rawDataSize;

    int padding[4];
};

#endif // __PHOTON__
