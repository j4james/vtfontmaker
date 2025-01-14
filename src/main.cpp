// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "application.h"
#include "capabilities.h"
#include "coloring.h"
#include "dialog.h"
#include "font.h"
#include "os.h"
#include "vt.h"

bool check_compatibility(const capabilities& caps)
{
    const auto compatible =
        caps.has_soft_fonts &&
        caps.has_horizontal_scrolling &&
        caps.has_color &&
        caps.has_rectangle_ops &&
        caps.has_macros &&
        caps.has_pages;
    if (!compatible) {
        vtout.write(application::name);
        vtout.write(" requires a VT525-compatible terminal.\n");
        return false;
    }
    if (caps.height < 24) {
        vtout.write(application::name);
        vtout.write(" requires a minimum screen height of 24.\n");
        return false;
    }
    if (caps.width < 54) {
        vtout.write(application::name);
        vtout.write(" requires a minimum screen width of 54.\n");
        return false;
    }
    return true;
}

int main(int argc, const char* argv[])
{
    os os;
    capabilities caps;
    if (!check_compatibility(caps))
        return 1;

    // Set the window title.
    vtout.decswt(application::name);
    // Set default attributes.
    vtout.sgr();
    // Clear the screen.
    vtout.ed(2);
    // Hide the cursor and disable auto wrap.
    vtout.rm('?', {25, 7});
    // Enable horizontal margins and origin mode.
    vtout.sm('?', {69, 6});
    // Enable rectangular change extent.
    vtout.decsace(2);

    vtout.cup(caps.height / 2, (caps.width - 10) / 2 + 1);
    vtout.write("Loading...");
    vtout.flush();

    // Load the soft font.
    const auto font = soft_font{};
    // Setup the color palette.
    const auto colors = coloring{caps};

    auto start_path = std::filesystem::path{};
    for (int i = 1; i < argc; i++) {
        auto arg = std::string{argv[i]};
        if (!arg.starts_with("-")) {
            start_path = std::filesystem::current_path().append(arg);
            break;
        }
    }

    dialog::initialize(caps);
    keyboard::initialize(caps);
    application app{caps, start_path};
    app.run();

    // Disable horizontal margins and origin mode.
    vtout.rm('?', {69, 6});
    // Clear the window title.
    vtout.decswt();
    // Clean out our macros on exit.
    vtout.decdmac({}, 1, {});
    // Set default attributes.
    vtout.sgr({});
    // Clear all pages.
    vtout.cup();
    vtout.ppa(3);
    vtout.ed();
    vtout.ppa(2);
    vtout.ed();
    vtout.ppa(1);
    vtout.ed();
    // Show the cursor and reenable autowrap.
    vtout.sm('?', {25, 7});
    // Restore default character set.
    vtout.ls0();

    return 0;
}
