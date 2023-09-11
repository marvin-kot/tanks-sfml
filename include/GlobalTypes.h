#pragma once

namespace globalTypes
{
    enum Direction
    {
        Unknown,
        Up,
        Left,
        Down,
        Right,
        Max4Direction
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

    enum GameState
    {
        TitleScreen,
        LoadNextLevel,
        StartLevelScreen,
        StartLevel,
        PlayingLevel,
        GameOver = 5,
        GameOverScreen,
        BonusShop,
        SelectLevel,
        ExitGame,
        // Network Game states
        WaitingForStartRequest,
        WaitingForLevelStartRequest,
        ConnectToHost,
        WaitForLevelData
    };


}
