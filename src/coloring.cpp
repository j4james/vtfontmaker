// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "coloring.h"

#include "capabilities.h"
#include "vt.h"

coloring::coloring(const capabilities& caps)
{
    // Save the current color table.
    _color_table = caps.query_color_table();
    // Set the desired color table entries.
    vtout.dcs("2$p"
              "0;2;0;0;0/"           // Black
              "1;2;8;8;8/"           // DarkGray
              "2;2;19;30;50/"        // DarkerBlue
              "3;2;23;34;54/"        // DarkBlue
              "4;2;56;67;87/"        // LightBlue
              "5;2;58;70;90/"        // LighterBlue
              "6;2;75;75;75/"        // LightGray
              "7;2;80;80;80/"        // White
              "8;2;15;24;40/"        // DarkestBlue
              "14;2;95;95;95/"       // BrightWhite
              "15;2;100;100;100/");  // BrightestWhite
}

coloring::~coloring()
{
    // Restore the original colors, or at least a reasonable palette.
    if (!_color_table.empty())
        vtout.dcs("2$p" + _color_table);
    else {
        vtout.dcs("2$p"
                  "0;2;0;0;0/"
                  "1;2;80;14;14/"
                  "2;2;20;80;20/"
                  "3;2;80;80;20/"
                  "4;2;20;20;80/"
                  "5;2;80;20;80/"
                  "6;2;20;80;80/"
                  "7;2;47;47;47/"
                  "8;2;27;27;27/"
                  "14;2;0;100;100/"
                  "15;2;100;100;100");
    }
}
