// HL1 BSP Parser (first try)
// By PROPHESSOR (2018)

#include <stdio.h>
#include <stdint.h>

#define SELECTOR_DEFAULT_VALUE  -100
#define SELECTOR_EXIT_VALUE     -1

#define LUMP_ENTITIES      0
#define LUMP_PLANES        1
#define LUMP_TEXTURES      2
#define LUMP_VERTEXES      3
#define LUMP_VISIBILITY    4
#define LUMP_NODES         5
#define LUMP_TEXINFO       6
#define LUMP_FACES         7
#define LUMP_LIGHTING      8
#define LUMP_CLIPNODES     9
#define LUMP_LEAVES       10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES        12
#define LUMP_SURFEDGES    13
#define LUMP_MODELS       14
#define HEADER_LUMPS      15

#define MAX_MAP_HULLS        4
#define MAX_MAP_MODELS       400
#define MAX_MAP_BRUSHES      4096
#define MAX_MAP_ENTITIES     1024
#define MAX_MAP_ENTSTRING    (128*1024)
#define MAX_MAP_PLANES       32767
#define MAX_MAP_NODES        32767
#define MAX_MAP_CLIPNODES    32767
#define MAX_MAP_LEAFS        8192
#define MAX_MAP_VERTS        65535
#define MAX_MAP_FACES        65535
#define MAX_MAP_MARKSURFACES 65535
#define MAX_MAP_TEXINFO      8192
#define MAX_MAP_EDGES        256000
#define MAX_MAP_SURFEDGES    512000
#define MAX_MAP_TEXTURES     512
#define MAX_MAP_MIPTEX       0x200000
#define MAX_MAP_LIGHTING     0x200000
#define MAX_MAP_VISIBILITY   0x200000
#define MAX_MAP_PORTALS     65536

const char lumpNames[15][18] = {
                        "Entities", "Planes", "Textures", "Verticles",
                        "Visibility", "Nodes", "Tex. info", "Faces",
                        "Lighting", "Clip nodes", "Leaves",
                        "Mark surfaces", "Edges", "Surf. edges", "Models"
};

typedef struct Vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef struct BSPLump {
    int32_t offset;
    int32_t length;
} BSPLump_t;

struct BSPHeader {
    int32_t version;
    BSPLump_t lumps[HEADER_LUMPS];
};

// Annoncers of the map parsers
void parsePlanes(FILE *, int32_t, int32_t);
void parseTextures(FILE *, int32_t, int32_t);
void parseFaces(FILE *, int32_t, int32_t);
void parseLightmaps(FILE *, int32_t, int32_t);
void parseVertexes(FILE *, int32_t, int32_t);
void parseModels(FILE *, int32_t, int32_t);
void parseTexinfo(FILE *, int32_t, int32_t);

int main() {
    printf("\n\nBSP Parser\nBy PROPHESSOR (2018)\n\n");
    FILE *file = fopen("bounce.bsp", "r");

    struct BSPHeader bspHeader;
    fread(&bspHeader, sizeof(bspHeader), 1, file);
    printf(" === BSP Header === \n");
    
    printf("Version: %d ", bspHeader.version);
    if(bspHeader.version == 30) printf("OK!");
    else printf("ERROR!");
    printf("\n");
    printf("Header lumps: %d\n", HEADER_LUMPS);
    printf("\n");

    short selectedLump;
    for(;;) {
        printf(" === Header Lumps === \n");

        for(short i = 0; i < HEADER_LUMPS; i++) {
            printf("  == Lump %d (%s) ==  \n", i, lumpNames[i]);
            printf("\tLump offset: %d\n", bspHeader.lumps[i].offset);
            printf("\tLump length: %d\n", bspHeader.lumps[i].length);
        }

        printf("\n\n");

        selectedLump = SELECTOR_DEFAULT_VALUE;
        printf("Which lump are you selecting? (Enter %d to exit)\n>>> ", SELECTOR_EXIT_VALUE);
        scanf("%hi", &selectedLump);
        /*fflush(stdin);*/
        if(selectedLump == SELECTOR_EXIT_VALUE) break;
        if(selectedLump == SELECTOR_DEFAULT_VALUE) break;

        switch(selectedLump) {
            case LUMP_PLANES:
                parsePlanes(file, bspHeader.lumps[LUMP_PLANES].offset, bspHeader.lumps[LUMP_PLANES].length);
                break;
            case LUMP_TEXTURES:
                parseTextures(file, bspHeader.lumps[LUMP_TEXTURES].offset, bspHeader.lumps[LUMP_TEXTURES].length);
                break;
            case LUMP_VERTEXES:
                parseVertexes(file, bspHeader.lumps[LUMP_VERTEXES].offset, bspHeader.lumps[LUMP_VERTEXES].length);
                break;
            case LUMP_TEXINFO:
                parseTexinfo(file, bspHeader.lumps[LUMP_TEXINFO].offset, bspHeader.lumps[LUMP_TEXINFO].length);
                break;
            case LUMP_FACES:
                parseFaces(file, bspHeader.lumps[LUMP_FACES].offset, bspHeader.lumps[LUMP_FACES].length);
                break;
            case LUMP_LIGHTING:
                parseLightmaps(file, bspHeader.lumps[LUMP_LIGHTING].offset, bspHeader.lumps[LUMP_LIGHTING].length);
                break;
            case LUMP_MODELS:
                parseModels(file, bspHeader.lumps[LUMP_MODELS].offset, bspHeader.lumps[LUMP_MODELS].length);
                break;
            default:
                printf("Doesn't implemented!\n");
                /*printf("Invalid lump index!\n");*/
                break;
        }
    }

    printf("\n\nBye!\n\n");
    return 0;
}

// ==============
// Parse planes
// ==============

#define PLANE_X 0     // Plane is perpendicular to given axis
#define PLANE_Y 1
#define PLANE_Z 2
#define PLANE_ANYX 3  // Non-axial plane is snapped to the nearest
#define PLANE_ANYY 4
#define PLANE_ANYZ 5

typedef struct Plane {
    vec3 normal;
    float dist;
    int32_t type;
} plane_t;

void parsePlanes(FILE *file, int32_t offset, int32_t length) {
    printf(" ### Planes parser ### \n");

    plane_t plane;

    short i = 0;
    for(int off = offset; off < offset + length; off += sizeof(plane)) {
        fseek(file, off, SEEK_SET);
        fread(&plane, sizeof(plane), 1, file);
        printf(" == Plane #%hi == \n", i++);
        printf("Plane.offset: %i\n", off);
        printf("Plane.normal: (%f %f %f)\n", plane.normal.x, plane.normal.y, plane.normal.z);
        printf("Plane.dist: %f\n", plane.dist);
        printf("Plane.type: %i\n", plane.type);
    }
}

// ==============
// Parse textures
// ==============

#define MAXTEXTURENAME 16
#define MIPLEVELS 4

typedef struct  {
    uint32_t    numOfTextures;
    int32_t     offsetToTextures;
} textureheader_t;

typedef struct Texture {
    char        name[MAXTEXTURENAME];
    uint32_t    width;
    uint32_t    height;
    uint32_t    offsets[MIPLEVELS];
} texture_t;

void parseTextures(FILE *file, int32_t offset, int32_t length) {
    printf(" ### Textures parser ### \n");

    textureheader_t textureHeader;
    texture_t texture;

    fseek(file, offset, SEEK_SET);
    fread(&textureHeader, sizeof(textureHeader), 1, file);
    printf("%i textures found!\n", textureHeader.numOfTextures);
    printf("Texture offset is %i\n", textureHeader.offsetToTextures);

    short i = 0;
    for(int off = offset + textureHeader.offsetToTextures; off < offset + length; off += sizeof(texture)) {
        fseek(file, off, SEEK_SET);
        printf("offset: %i\n", off);
        fread(&texture, sizeof(texture), 1, file);
        printf(" == Texture #%hi == \n", i++);
        printf("Texture.name: %s\n", texture.name);
        printf("Texture.width: %i\n", texture.width);
        printf("Texture.height: %i\n", texture.height);
        printf("Texture.offsets: [%i, %i, %i, %i]\n", texture.offsets[0], texture.offsets[1], texture.offsets[2], texture.offsets[3]);
    }
}

// ==============
// Parse vertexes
// ==============

typedef struct Vertex {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} vertex_t;

void parseVertexes(FILE *file, int32_t offset, int32_t length) {
    printf(" ### Vertices parser ### \n");

    vertex_t vertex;

    short i = 0;
    for(int off = offset; off < offset + length; off += sizeof(vertex)) {
        fseek(file, off, SEEK_SET);
        fread(&vertex, sizeof(vertex), 1, file);
        printf(" == Vertex #%hi == \n", i++);
        printf("xyz(%hi %hi %hi)\n", vertex.x, vertex.y, vertex.z);
    }
}

// ==============
// Parse texinfos
// ==============

typedef struct texInfo {
    vec3        s;
    float       sShift;
    vec3        t;
    float       tShift;
    uint32_t    texture;
    uint32_t    flags;
} texinfo_t;

void parseTexinfo(FILE *file, int32_t offset, int32_t length) {
    printf(" ### Texinfo parser ### \n");

    texinfo_t texinfo;

    short i = 0;
    for(int off = offset; off < offset + length; off += sizeof(texinfo)) {
        fseek(file, off, SEEK_SET);
        fread(&texinfo, sizeof(texinfo), 1, file);
        printf(" == Texinfo #%i == \n", i++);
        printf("Texinfo.s: (%f %f %f)\n",           texinfo.s.x, texinfo.s.y, texinfo.s.z);
        printf("Texinfo.sShift: %f\n",              texinfo.sShift);
        printf("Texinfo.t: (%f %f %f)\n",           texinfo.t.x, texinfo.t.y, texinfo.t.z);
        printf("Texinfo.tShift: %f\n",              texinfo.tShift);
        printf("Texinfo.texture: %i\n",             texinfo.texture);
        printf("Texinfo.flags: %i\n",               texinfo.flags);
    }
}


// ==============
// Parse faces
// ==============

typedef struct Face {
    uint16_t    plane;
    uint16_t    planeSide;
    uint32_t    firstEdge;
    uint16_t    edges;
    uint16_t    texture;
    uint8_t     lightStyles[4];
    uint32_t    lightmapOffset;
} face_t;

void parseFaces(FILE *file, int32_t offset, int32_t length) {
    printf(" ### Faces parser ### \n");

    face_t face;

    short i = 0;
    for(int off = offset; off < offset + length; off += sizeof(face)) {
        fseek(file, off, SEEK_SET);
        fread(&face, sizeof(face), 1, file);
        printf(" == Face #%i == \n", i++);
        printf("Face.plane: %i\n", face.plane);
        printf("Face.planeSide: %i\n", face.planeSide);
        printf("Face.firstEdge: %i\n", face.firstEdge);
        printf("Face.texture: %i\n", face.texture);
        printf("Face.lightStyles: [%i %i %i, %i]\n", face.lightStyles[0], face.lightStyles[1], face.lightStyles[2], face.lightStyles[3]);
        printf("Face.lightmapOffset: %i\n", face.lightmapOffset);
    }
}

// ==============
// Parse lightmaps
// ==============

typedef struct Lightmap {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} lightmap_t;

void parseLightmaps(FILE *file, int32_t offset, int32_t length) {
    printf(" ### Lightmaps parser ### \n");

    lightmap_t lightmap;

    short i = 0;
    for(int off = offset; off < offset + length; off += sizeof(lightmap)) {
        fseek(file, off, SEEK_SET);
        fread(&lightmap, sizeof(lightmap), 1, file);
        printf(" == Lightmap #%hi == \n", i++);
        printf("rgb(%hi %hi %hi)\n", lightmap.r, lightmap.g, lightmap.b);
    }
}

// ==============
// Parse models
// ==============

typedef struct Model {
    vec3    boxMin;
    vec3    boxMax;
    vec3    origin;
    int32_t headNodes[MAX_MAP_HULLS];
    int32_t visLeafs;
    int32_t firstFace;
    int32_t faces;
} model_t;

void parseModels(FILE *file, int32_t offset, int32_t length) {
    printf(" ### Models parser ### \n");

    model_t model;

    short i = 0;
    for(int off = offset; off < offset + length; off += sizeof(model)) {
        fseek(file, off, SEEK_SET);
        fread(&model, sizeof(model), 1, file);
        printf(" == Model #%hi == \n", i++);
        printf("Model.boxMin: (%f %f %f)\n", model.boxMin.x, model.boxMin.y, model.boxMin.z);
        printf("Model.boxMax: (%f %f %f)\n", model.boxMax.x, model.boxMax.y, model.boxMax.z);
        printf("Model.origin: (%f %f %f)\n", model.origin.x, model.origin.y, model.origin.z);
        printf("Model.headNodes: [%i %i %i %i]\n", model.headNodes[0], model.headNodes[1], model.headNodes[2], model.headNodes[3]);
        printf("Model.visLeafs: %i\n", model.visLeafs);
        printf("Model.firstFace: %i\n", model.firstFace);
        printf("Model.faces: %i\n", model.faces);
    }
}
