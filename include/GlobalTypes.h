#pragma once

namespace globalTypes
{
    enum Direction
    {
        Unknown,
        Up,
        Left,
        Down,
        Right
    };

    enum EagleWallDirection
    {
        NotAnEagleWall,
        DownLeft,
        LeftLeftDown,
        LeftLeftUp,
        UpLeft,
        UpUpLeft,
        UpUpRight,
        UpRight,
        RightRightUp,
        RightRightDown,
        DownRight,
        DownDownRight,
        DownDownLeft,
        MaxDirection
    };

}
