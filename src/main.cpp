#include "app/engine_app.hpp"


#include <iostream>

int main() {
    auto *app = new app::EngineApp();
    app->run();

    delete app;

    return 0;
}
