#include <stdio.h>
#include <stdlib.h>

#include "photon.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#define LCD_RESX 2560
#define LCD_RESY 1600

uint8_t padding0[]= {0x19,0x00,0xFD,0x12,0x01,0x00,0x00,0x00};
uint8_t padding1[]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t padding2[]= {0x6C,0x00,0x00,0x00,0xAC,0xF0,0x00,0x00};
uint8_t padding3[]= {0xD0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t padding4[]= {0x01,0x00,0x00,0x00};
uint8_t padding5[]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t padding6[]= {0x33,0x04,0x00,0x00,0xE6,0x00,0x00,0x00};
uint8_t padding7[]= {0x8C,0x00,0x00,0x00,0x44,0xE0,0x00,0x00};

char inputFile[256],outputFile[256],temp[256];
char verbose;
int prx1,pry1,prx2,pry2,pr1size,pr2size;
unsigned char *data,*preview1,*preview2;

int exposure=120,border=20;

struct HEADER header;
struct previewImage preview1Img,preview2Img;
struct layerdata layer;

uint16_t *compressColorImage(uint8_t *data,int w,int h,int *size)
{
    int x,y;
    uint8_t *point;
    uint8_t lastPixel;
    int16_t count;

    *size=0;
    point=data;

    lastPixel=*point++;
    count=0;
    for (y=0; y<h; y++)
        for(x=0; x<w; x++)
        {
            count++;
            if (count==4094 || *point!=lastPixel)
            {
                *size+=2;
                if (count>1)
                    *size+=2;
                count=0;
            }
            lastPixel=*point++;
        }

    uint16_t *compressed = (uint16_t *)malloc(*size);
    uint16_t *comPoint=compressed,temppix;

    point=data;
    lastPixel=*point++;
    count=0;
    for (y=0; y<h; y++)
        for(x=0; x<w; x++)
        {
            count++;
            if (count==4094 || *point!=lastPixel)
            {
                temppix=lastPixel>>3;
                *comPoint=(temppix<<11) | (temppix<<6) | (31-temppix);
                if (count>1)
                {
                    *comPoint |=0x0020;
                    comPoint++;
                    *comPoint=(count-1) | 0x3000;
                }
                count=0;
                comPoint++;
            }
            lastPixel=*point++;
        }
    return compressed;
}

uint8_t *compressBWImage(uint8_t *data,int w,int h,int *size)
{
    int x,y;
    uint8_t *point;
    uint8_t lastPixel;
    uint8_t count;

    *size=0;
    point=data;

    lastPixel=(*point++) & 0x80;
    count=0;
    for (y=0; y<h; y++)
        for(x=0; x<w; x++)
        {
            count++;
            if (count==125 || (*point & 0x80) !=lastPixel)
            {
                *size+=1;
                count=0;
            }
            lastPixel=(*point++)&0x80;
        }

    uint8_t *compressed = (uint8_t *)malloc(*size);
    uint8_t *comPoint=compressed;

    point=data;
    lastPixel=(*point++) & 0x80;
    count=0;
    for (y=0; y<h; y++)
        for(x=0; x<w; x++)
        {
            count++;
            if (count==125 || (*point & 0x80) !=lastPixel)
            {
                *comPoint=lastPixel + count;
                count=0;
                comPoint++;
            }
            lastPixel=(*point++ & 0x80);
        }
    return compressed;
}

void imgcenter(uint8_t *dest,uint8_t *source,int w,int h)
{
    uint8_t *pointdest,*pointsource;
    int x,y;

    pointsource=source;
    for (y=0; y<border; y++)
    {
        pointdest=dest+((((LCD_RESY-h-border-border) / 2)+y)*LCD_RESX)+((LCD_RESX-w-border-border) / 2);
        for(x=0; x<(w+border+border); x++)
        {
            *pointdest++=255;
        }
    }
    for (y=0; y<h; y++)
    {
        pointdest=dest+((((LCD_RESY-h-border-border) / 2)+y+border)*LCD_RESX)+((LCD_RESX-w-border-border) / 2);
        for(x=0; x<border; x++)
        {
            *pointdest++=255;
        }
        for(x=0; x<w; x++)
        {
            *pointdest=*pointsource;
            pointsource+=1;
            pointdest+=1;
        }
        for(x=0; x<border; x++)
        {
            *pointdest++=255;
        }
    }
    for (y=0; y<border; y++)
    {
        pointdest=dest+((((LCD_RESY-h-border-border) / 2)+y+border+h)*LCD_RESX)+((LCD_RESX-w-border-border) / 2);
        for(x=0; x<(w+border+border); x++)
        {
            *pointdest++=255;
        }
    }
}

void imghome(uint8_t *dest,uint8_t *source,int w,int h)
{
    uint8_t *pointdest,*pointsource;
    int x,y;

    pointsource=source;
    for (y=0; y<border; y++)
    {
        pointdest=dest+y*LCD_RESX;
        for(x=0; x<(w+border+border); x++)
        {
            *pointdest++=255;
        }
    }
    for (y=0; y<h; y++)
    {
        pointdest=dest+((y+border)*LCD_RESX);
        for(x=0; x<border; x++)
        {
            *pointdest++=255;
        }
        for(x=0; x<w; x++)
        {
            *pointdest=*pointsource;
            pointsource+=1;
            pointdest+=1;
        }
        for(x=0; x<border; x++)
        {
            *pointdest++=255;
        }
    }
    for (y=0; y<border; y++)
    {
        pointdest=dest+(y+border+h)*LCD_RESX;
        for(x=0; x<(w+border+border); x++)
        {
            *pointdest++=255;
        }
    }
}

int main(int argc, char **argv)
{
    uint32_t nn;
    uint8_t center = 0;

    verbose=0;
    printf ("PCB 2 PHOTON - Essenza 20200310\n\r");

    for (nn=1; nn<argc; nn++)
    {
        if (!strcmp(argv[nn],"-i"))
            strcpy(inputFile,argv[++nn]);
        else if (!strcmp(argv[nn],"-o"))
            strcpy(outputFile,argv[++nn]);
        else if (!strcmp(argv[nn],"-v"))
            verbose=1;
        else if (!strcmp(argv[nn],"-t"))
            exposure=atoi(argv[++nn]);
        else if (!strcmp(argv[nn],"-b"))
            border=atoi(argv[++nn]);
        else if (!strcmp(argv[nn],"-c"))
            center =1;
        else if (!strcmp(argv[nn],"-h"))
        {
            printf ("Usage :\n\rpcb2photon [options] -i inputfile [-o outputfile] [-t exposuretime(s)] [-b border(pixel)]\n\r");
            printf ("Input file must be 338.66 DPI\n\n\n\n\r");
            printf ("Options:\n\r -v : verbose\n\r");
            printf ("-h : this help\n\r");
            printf ("-c : center image on lcd\n\r");
        }
        else
            strcpy(inputFile,argv[nn]);
    }

    if (strlen(inputFile)==0)
    {
        printf ("Please specify input file\n\r");
        return -1;
    }

    if (strlen(outputFile)==0)
    {
        int n;
        strcpy(outputFile,inputFile);
        n=strlen(outputFile);
        while (outputFile[n]!='.' && n )
            n--;
        if (n)
            strcpy(outputFile+n,".cbddlp");
    }

    if (verbose)
    {
        printf ("Input  file = %s\n\r",inputFile);
        printf ("Output file = %s\n\r",outputFile);
        printf ("Exposure = %d\n\r",exposure);
        printf ("Border = %d\n\r",border);
    }

    int x,y,nchannels;
    data = stbi_load(inputFile, &x, &y, &nchannels, 0);       // load png

    if (x> LCD_RESX || y>LCD_RESY)
    {
        printf ("Input image max %d x %d\n\r",LCD_RESX,LCD_RESY);
        return -1;
    }

    if (nchannels==3)                                                        //convert to BW
    {
        int xx,yy,orig,dest,pixel;
        unsigned char *tempimage=(unsigned char *)malloc(x*y);
        for (dest=0,orig=0,yy=0; yy<y; yy++)
            for(xx=0; xx<x; xx++,nn++,dest++)
            {
                pixel=data[orig++]+data[orig++]+data[orig++];
                tempimage[dest]= (pixel>384) ? 255 : 0;
            }
        free (data);
        data=tempimage;
        nchannels=1;
    }

    prx1=400;
    pry1=y*prx1/x;
    preview1=(unsigned char *)malloc(prx1*pry1);
    stbir_resize_uint8(data, x, y, 0, preview1, prx1, pry1, 0, nchannels);
    uint16_t *prev1=compressColorImage(preview1,prx1,pry1,&pr1size);

    prx2=200;
    pry2=y*prx2/x;
    preview2=(unsigned char *)malloc(prx2*pry2);
    stbir_resize_uint8(data, x, y, 0, preview2, prx2, pry2, 0, nchannels);
    uint16_t *prev2=compressColorImage(preview2,prx2,pry2,&pr2size);

    unsigned char *tempimage=(unsigned char *)malloc(LCD_RESX*LCD_RESY);       // crea una nuova immagine;
    memset(tempimage,0,LCD_RESX*LCD_RESY);                                   // entirely black
    if (center ==1)
        imgcenter(tempimage,data,x,y);
    else
        imghome(tempimage,data,x,y);
    free(data);
    data=tempimage;
    x=LCD_RESX;
    y=LCD_RESY;


    int layersize;
    uint8_t *layerimage=compressBWImage(data,x,y,&layersize);

    memcpy(header.unknown0,padding0,8);
    header.sizeX=192;
    header.sizeY=120;
    header.sizeZ=200;
    memcpy(header.padding0,padding0,12);
    header.layerThickness=1.6;
    header.normalExposure=exposure;
    header.bottomExposure=0;
    header.offTime=0;
    header.nBottomLayers=0;
    header.resolutionX=LCD_RESX;
    header.resolutionY=LCD_RESY;
    header.preview1Offset=sizeof(header);
    header.layerDefsStart=sizeof(header)+sizeof(preview1Img)+sizeof(preview2Img)+pr1size+pr2size;
    header.nLayers=1;
    header.preview2Offset=sizeof(header)+sizeof(preview1Img);

    preview1Img.width=prx1;
    preview1Img.height=pry1;
    preview1Img.dataStartPos=sizeof(header)+sizeof(preview1Img)+sizeof(preview2Img);
    preview1Img.dataSize=pr1size;

    preview2Img.width=prx2;
    preview2Img.height=pry2;
    preview2Img.dataStartPos=sizeof(header)+sizeof(preview1Img)+sizeof(preview2Img)+pr1size;
    preview2Img.dataSize=pr2size;

    layer.layerHeight=0;
    layer.exposureTime=100;
    layer.offTime=0;
    layer.dataStartPos=sizeof(header)+sizeof(preview1Img)+sizeof(preview2Img)+pr1size+pr2size+sizeof(layer);
    layer.rawDataSize=layersize;

    FILE *out=fopen(outputFile,"wb");
    if (out)
    {
        fwrite(&header,sizeof(header),1,out);
        fwrite(&preview1Img,sizeof(preview1Img),1,out);
        fwrite(&preview2Img,sizeof(preview2Img),1,out);
        fwrite(prev1,pr1size,1,out);
        fwrite(prev2,pr2size,1,out);
        fwrite(&layer,sizeof(layer),1,out);
        fwrite(layerimage,layersize,1,out);
        fclose(out);
    }
    printf("Done.\n\r");
    return 0;
}
