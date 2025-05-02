#include "app/engine_app.hpp"

#include <iostream>


int main() {
    try {
        auto *app = new app::EngineApp();
        app->run();

        delete app;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
