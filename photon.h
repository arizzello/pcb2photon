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
