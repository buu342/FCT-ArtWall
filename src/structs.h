#ifndef ATRWALL_STRUCTS
#define ATRWALL_STRUCTS

    struct m_col {
        float red;
        float green;
        float blue;
    };
    
    enum ThumbType {
        None,
        Image,
        GIF,
        Video
    };

    enum FilterType {
        NoFilter = -1,
        Tag = 0,
        Luminance = 1,
        Color = 2,
        Faces = 3,
        Edge = 4,
        Texture = 5,
        Scenes = 6,
        Match = 7
    };

    struct Vector2D{
        float x;
        float y;
        inline Vector2D operator+(Vector2D a) {return {x+a.x, y+a.y};};
        inline Vector2D operator-(Vector2D a) {return {x-a.x, y-a.y};};
        inline Vector2D operator*(Vector2D a) {return {x*a.x, y*a.y};};
        inline Vector2D operator/(Vector2D a) {return {x/a.x, y/a.y};};
        inline Vector2D operator+(float a) {return {x+a, y+a};};
        inline Vector2D operator-(float a) {return {x-a, y-a};};
        inline Vector2D operator*(float a) {return {x*a, y*a};};
        inline Vector2D operator/(float a) {return {x/a, y/a};};
        inline Vector2D operator-(void) {return {-x, -y};};
        inline bool operator==(Vector2D a) {return {x == a.x && y == a.y};};
        inline bool operator<(Vector2D a) {return {x < a.x && y < a.y};};
        inline bool operator>(Vector2D a) {return {x > a.x && y > a.y};};
        inline bool operator<=(Vector2D a) {return {x <= a.x && y <= a.y};};
        inline bool operator>=(Vector2D a) {return {x >= a.x && y >= a.y};};
    };

    struct Meta {
        string path;
        std::vector<string> tags;
        double   averageluminance;
        m_col    averagecolor;
        int      facecount;
        int      edgeangle;
        std::vector<double> cuts;
        double   textures[8];
    };

#endif