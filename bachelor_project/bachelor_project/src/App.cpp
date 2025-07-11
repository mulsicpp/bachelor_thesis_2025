#include "App.h"

App::App() {

}

void App::run() {
    dbg_log("run");
    while (!m_context.window_should_close()) {
        m_context.poll_events();
    }
}