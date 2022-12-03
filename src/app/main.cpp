#include <cstdlib>

#include "platform.h"

int main() {
    PlatformState state;
    state.styleDark = true;

    if (!InitPlatform(&state, "SourceModel")) {
        std::exit(EXIT_FAILURE);
    }

    RunPlatformMainLoop(&state);

    CleanupPlatform(&state);

    std::exit(EXIT_SUCCESS);
}