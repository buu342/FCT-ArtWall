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
        std::vector<string> tags;
        int      tagcount;
        float    averageluminance;
        m_col    averagecolor;
        int      facecount;
        double   edges[4];
        std::vector<double> cuts;
    };

#endif